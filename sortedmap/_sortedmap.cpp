#include <vector>
#include <exception>
#include <map>

#include "sortedmap.h"

const char *sortedmap::keyiter::name = "sortedmap.keyiter";
const char *sortedmap::valiter::name = "sortedmap.valiter";
const char *sortedmap::itemiter::name = "sortedmap.itemiter";
const char *sortedmap::keyview::name = "sortedmap.keyview";
const char *sortedmap::valview::name = "sortedmap.valview";
const char *sortedmap::itemview::name = "sortedmap.itemview";

PyObject*
py_identity(PyObject *ob)
{
    Py_INCREF(ob);
    return ob;
}

bool
sortedmap::check(PyObject *ob)
{
    return PyObject_IsInstance(ob, (PyObject*) &sortedmap::type);
}

bool
sortedmap::check_exact(PyObject *ob)
{
    return Py_TYPE(ob) == &sortedmap::type;
}

void
sortedmap::abstractiter::dealloc(sortedmap::abstractiter::object *self)
{
    using sortedmap::abstractiter::itertype;
    using ownedtype = OwnedRef<sortedmap::object>;

    self->iter.~itertype();
    self->map.~ownedtype();
    PyObject_Del(self);
}

PyObject*
sortedmap::keyiter::elem(sortedmap::abstractiter::itertype it)
{
    return std::get<0>(*it).incref();
}

PyObject*
sortedmap::valiter::elem(sortedmap::abstractiter::itertype it)
{
    return std::get<1>(*it).incref();
}

PyObject*
sortedmap::itemiter::elem(sortedmap::abstractiter::itertype it)
{
    return PyTuple_Pack(2,
                        sortedmap::keyiter::elem(it),
                        sortedmap::valiter::elem(it));
}

PyObject*
sortedmap::keyiter::iter(sortedmap::object *self)
{
    return sortedmap::abstractiter::iter<sortedmap::keyiter::object,
                                         sortedmap::keyiter::type>(self);
}

PyObject*
sortedmap::valiter::iter(sortedmap::object *self)
{
    return sortedmap::abstractiter::iter<sortedmap::valiter::object,
                                         sortedmap::valiter::type>(self);
}

PyObject*
sortedmap::itemiter::iter(sortedmap::object *self)
{
    return sortedmap::abstractiter::iter<sortedmap::itemiter::object,
                                         sortedmap::itemiter::type>(self);
}

PyObject*
sortedmap::abstractview::repr(sortedmap::abstractview::object *self)
{
    PyObject *aslist;
    PyObject *ret;

    if (!(aslist = PySequence_List((PyObject*) self))) {
        return NULL;
    }
    ret = PyUnicode_FromFormat("%s(%R)", Py_TYPE(self)->tp_name, aslist);
    Py_DECREF(aslist);
    return ret;
}

PyObject*
sortedmap::keyview::view(sortedmap::object *self)
{
    return sortedmap::abstractview::view<sortedmap::keyview::object,
                                         sortedmap::keyview::type>(self);
}

PyObject*
sortedmap::valview::view(sortedmap::object *self)
{
    return sortedmap::abstractview::view<sortedmap::valview::object,
                                         sortedmap::valview::type>(self);
}

PyObject*
sortedmap::itemview::view(sortedmap::object *self)
{
    return sortedmap::abstractview::view<sortedmap::itemview::object,
                                         sortedmap::itemview::type>(self);
}

void
sortedmap::abstractview::dealloc(sortedmap::abstractview::object *self)
{
    using ownedtype = OwnedRef<sortedmap::object>;

    self->map.~ownedtype();
    PyObject_Del(self);
}

static sortedmap::object*
innernew(PyTypeObject *cls)
{
    sortedmap::object *self = PyObject_GC_New(sortedmap::object, cls);

    if (unlikely(!self)) {
        return NULL;
    }

    return new(self) sortedmap::object;
}

sortedmap::object*
sortedmap::newobject(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    return innernew(cls);
}

int
sortedmap::init(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    return (sortedmap::update(self, args, kwargs)) ? 0 : -1;
}

void
sortedmap::dealloc(sortedmap::object *self)
{
    using sortedmap::maptype;

    sortedmap::clear(self);
    self->map.~maptype();
    PyObject_GC_Del(self);
}

int
sortedmap::traverse(sortedmap::object *self, visitproc visit, void *arg)
{
    for (const auto &pair : self->map) {
        Py_VISIT(pair.first);
        Py_VISIT(pair.second);
    }
    return 0;
}

void
sortedmap::clear(sortedmap::object *self)
{
    self->map.clear();
}

PyObject*
sortedmap::pyclear(sortedmap::object *self)
{
    sortedmap::clear(self);
    Py_RETURN_NONE;
}

PyObject *
sortedmap::richcompare(sortedmap::object *self, PyObject *other, int opid)
{
    if (!(opid == Py_EQ || opid == Py_NE) || !sortedmap::check(other)) {
        Py_RETURN_NOTIMPLEMENTED;
    }

    sortedmap::object *asmap = (sortedmap::object*) other;

    if (self->map.size() != asmap->map.size()) {
        if (opid == Py_EQ) {
            Py_RETURN_FALSE;
        }
        else {
            Py_RETURN_TRUE;
        }
    }

    int status;
    OwnedRef<PyObject> other_val;

    for (const auto &pair : self->map) {
        try{
            other_val = asmap->map.at(std::get<0>(pair));
        }
        catch (std::out_of_range &e) {
            if (opid == Py_EQ) {
                Py_RETURN_FALSE;
            }
            else {
                Py_RETURN_TRUE;
            }
        }
        catch (PythonError &e) {
            return NULL;
        }
        status = PyObject_RichCompareBool(std::get<1>(pair), other_val, opid);
        if (unlikely(status < 0)) {
            return NULL;
        }
        if (!status) {
            Py_RETURN_FALSE;
        }
    }
    Py_RETURN_TRUE;
}

Py_ssize_t
sortedmap::len(sortedmap::object *self)
{
    return self->map.size();
}

PyObject*
sortedmap::getitem(sortedmap::object *self, PyObject *key)
{
    try {
        const auto &it = self->map.find(key);
        if (it == self->map.end()) {
            PyErr_SetObject(PyExc_KeyError, key);
            return NULL;
        }
        return std::get<1>(*it).incref();
    }
    catch (PythonError &e) {
        return NULL;
    }
}

PyObject*
sortedmap::get(sortedmap::object *self, PyObject *key, PyObject *def)
{
    try {
        const auto &it = self->map.find(key);
        if (it == self->map.end()) {
            Py_INCREF(def);
            return def;
        }
        return std::get<1>(*it).incref();
    }
    catch (PythonError &e) {
        return NULL;
    }
}

PyObject*
sortedmap::pyget(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    const char *keywords[] = {"key", "default", NULL};
    PyObject *key;
    PyObject *def = NULL;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "O|O:get",
                                     (char**) keywords,
                                     &key,
                                     &def)) {
        return NULL;
    }

    if (!def) {
        def = Py_None;
    }

    return sortedmap::get(self, key, def);
}

PyObject*
sortedmap::pop(sortedmap::object *self, PyObject *key, PyObject *def)
{
    try {
        PyObject *ret;

        const auto &it = self->map.find(key);
        if (it == self->map.end()) {
            if (!def) {
                PyErr_SetObject(PyExc_KeyError, key);
            }
            else {
                Py_INCREF(def);
            }
            return def;
        }
        ret = std::get<1>(*it).incref();
        // use the same iterator to the item for a faster erase
        self->map.erase(it);
        ++self->iter_revision;
        return ret;
    }
    catch (PythonError &e) {
        return NULL;
    }
}

PyObject*
sortedmap::pypop(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    const char *keywords[] = {"key", "default", NULL};
    PyObject *key;
    PyObject *def = NULL;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "O|O:pop",
                                     (char**) keywords,
                                     &key,
                                     &def)) {
        return NULL;
    }

    return sortedmap::pop(self, key, def);
}

static void
setitem_throws(sortedmap::object *self, PyObject *key, PyObject *value)
{
    const auto &pair = self->map.emplace(key, value);
    if (std::get<1>(pair)) {
        ++self->iter_revision;
    }
    else {
        std::get<1>(*std::get<0>(pair)) = std::move(OwnedRef<PyObject>(value));
    }
}

int
sortedmap::setitem(sortedmap::object *self, PyObject *key, PyObject *value)
{
    try {
        if (!value) {
            self->map.erase(key);
            ++self->iter_revision;
        }
        else {
            setitem_throws(self, key, value);
        }
    }
    catch (PythonError &e) {
        return -1;
    }
    return 0;
}

int
sortedmap::contains(sortedmap::object *self, PyObject *key)
{
    try {
        return self->map.find(key) != self->map.end();
    }
    catch (PythonError &e) {
        return -1;
    }
}

PyObject*
sortedmap::repr(sortedmap::object *self)
{
    PyObject *it;
    PyObject *aslist;
    PyObject *ret;

    if (!(it = itemiter::iter(self))) {
        return NULL;
    }
    if (!(aslist = PySequence_List(it))) {
        Py_DECREF(it);
        return NULL;
    }
    ret = PyUnicode_FromFormat("%s(%R)", Py_TYPE(self)->tp_name, aslist);
    Py_DECREF(it);
    Py_DECREF(aslist);
    return ret;
}

sortedmap::object*
sortedmap::copy(sortedmap::object *self)
{
    sortedmap::object *ret = innernew(Py_TYPE(self));

    if (unlikely(!ret)) {
        return NULL;
    }

    ret->map = self->map;
    return ret;
}

static bool
merge(sortedmap::object *self, PyObject *other)
{
    if (sortedmap::check_exact(other)) {
        try {
            for (const auto &pair : ((sortedmap::object*) other)->map) {
                setitem_throws(self, std::get<0>(pair), std::get<1>(pair));
            }
        }
        catch (PythonError &e) {
            return false;
        }

        return true;
    }

    PyObject *key;

    if (PyDict_Check(other)) {
        PyObject *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(other, &pos, &key, &value)) {
            try {
                setitem_throws(self, key, value);
            }
            catch (PythonError &e) {
                return false;
            }
        }
    }
    else {
        PyObject *keys;
        PyObject *it;
        PyObject *tmp;

        if (unlikely(!(keys = PyMapping_Keys(other)))) {
            return false;
        }

        it = PyObject_GetIter(keys);
        Py_DECREF(keys);
        if (unlikely(!it)) {
            return false;
        }
        while ((key = PyIter_Next(it))) {
            if (unlikely(!(tmp = PyObject_GetItem(other, key)))) {
                Py_DECREF(key);
                Py_DECREF(it);
                return false;
            }
            try {
                setitem_throws(self, key, tmp);
            }
            catch (PythonError &e) {
                Py_DECREF(key);
                Py_DECREF(it);
                return false;
            }
            Py_DECREF(key);
        }
        Py_DECREF(it);
        if (unlikely(PyErr_Occurred())) {
            return false;
        }
    }
    return true;
}

static bool
merge_from_seq2(sortedmap::object *self, PyObject *seq2)
{
    PyObject *it;
    Py_ssize_t n;
    PyObject *item;
    PyObject *fast;

    if (unlikely(!(it = PyObject_GetIter(seq2)))) {
        return false;
    }

    for (n = 0;;++n) {
        PyObject *key, *value;
        Py_ssize_t len;

        fast = NULL;
        if (unlikely(!(item = PyIter_Next(it)))) {
            if (unlikely(PyErr_Occurred())) {
                goto fail;
            }
            break;
        }

        // convert item to sequence, and verify length 2
        if (unlikely(!(fast = PySequence_Fast(item, "")))) {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
                PyErr_Format(PyExc_TypeError,
                             "cannot convert sortedmap update "
                             "sequence element %zd to a sequence",
                             n);
            goto fail;
        }
        len = PySequence_Fast_GET_SIZE(fast);
        if (unlikely(len != 2)) {
            PyErr_Format(PyExc_ValueError,
                         "sortedmap update sequence element %zd "
                         "has length %zd; 2 is required",
                         n, len);
            goto fail;
        }

        // update with this (key, value) pair
        key = PySequence_Fast_GET_ITEM(fast, 0);
        value = PySequence_Fast_GET_ITEM(fast, 1);
        try{
            setitem_throws(self, key, value);
        }
        catch (PythonError &e) {
            goto fail;
        }
        Py_DECREF(fast);
        Py_DECREF(item);
    }

    n = 0;
    goto return_;
fail:
    Py_XDECREF(item);
    Py_XDECREF(fast);
    n = -1;
return_:
    Py_DECREF(it);
    return !Py_SAFE_DOWNCAST(n, Py_ssize_t, int);
}

bool
sortedmap::update(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    PyObject *arg = NULL;

    if (unlikely(!PyArg_UnpackTuple(args, "update", 0, 1, &arg))) {
        return false;
    }

    if (arg) {
#if !COMPILING_IN_PY2
        _Py_IDENTIFIER(keys);

        if (_PyObject_HasAttrId(arg, &PyId_keys)) {
#else
        if (PyObject_HasAttrString(arg, "keys")) {
#endif  // !COMPILING_IN_PY2

            if (unlikely(!merge(self, arg))) {
                return false;
            }
        }
        else {
            if (unlikely(!merge_from_seq2(self, arg))) {
                return false;
            }
        }
    }
    if (kwargs && PyDict_Size(kwargs)) {
        if (unlikely(!merge(self, kwargs))) {
            return false;
        }
    }
    return true;
}

PyObject*
sortedmap::pyupdate(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    if (unlikely(!sortedmap::update(self, args, kwargs))) {
        return NULL;
    }
    Py_RETURN_NONE;
}

sortedmap::object*
sortedmap::fromkeys(PyTypeObject *cls, PyObject *seq, PyObject *value) {
    sortedmap::object *self;
    PyObject *it;
    PyObject *key;

    if (unlikely(!(it = PyObject_GetIter(seq)))) {
        return NULL;
    }

    if (unlikely(!(self = innernew(cls)))) {
        Py_DECREF(it);
        return NULL;
    }

    while ((key = PyIter_Next(it))) {
        self->map[key] = value;
        Py_DECREF(key);
    }
    Py_DECREF(it);
    if (unlikely(PyErr_Occurred())) {
        return NULL;
    }

    return self;
}

sortedmap::object*
sortedmap::pyfromkeys(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    const char *keywords[] = {"seq", "value", NULL};
    PyObject *seq;
    PyObject *value = NULL;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "O|O:fromkeys",
                                     (char**) keywords,
                                     &seq,
                                     &value)) {
        return NULL;
    }

    if (!value) {
        value = Py_None;
    }

    return sortedmap::fromkeys((PyTypeObject*) cls, seq, value);
}


#define MODULE_NAME "sortedmap._sortedmap"
PyDoc_STRVAR(module_doc,
             "A sorted map that does not use hashing.");

#if !COMPILING_IN_PY2
static struct PyModuleDef _sortedmap_module = {
    PyModuleDef_HEAD_INIT,
    MODULE_NAME,
    module_doc,
    -1,
};
#endif  // !COMPILING_IN_PY2

extern "C" {
PyMODINIT_FUNC
#if !COMPILING_IN_PY2
#define ERROR_RETURN NULL
PyInit__sortedmap(void)
#else
#define ERROR_RETURN
init_sortedmap(void)
#endif  // !COMPILING_IN_PY2
{
    std::vector<PyTypeObject*> ts = {&sortedmap::type,
                                     &sortedmap::keyiter::type,
                                     &sortedmap::valiter::type,
                                     &sortedmap::itemiter::type,
                                     &sortedmap::keyview::type,
                                     &sortedmap::valview::type,
                                     &sortedmap::itemiter::type};
    PyObject *m;

    for (const auto &t : ts) {
        if (PyType_Ready(t)) {
            return ERROR_RETURN;
        }
    }

#if !COMPILING_IN_PY2
    if (!(m = PyModule_Create(&_sortedmap_module))) {
#else
    if (!(m = Py_InitModule3(MODULE_NAME, NULL, module_doc))) {
#endif  // !COMPILING_IN_PY2
        return ERROR_RETURN;
    }

    if (PyModule_AddObject(m, "sortedmap", (PyObject*) &sortedmap::type)) {
        Py_DECREF(m);
        return ERROR_RETURN;
    }

#if !COMPILING_IN_PY2
    return m;
#endif  // !COMPILING_IN_PY2
}
}
