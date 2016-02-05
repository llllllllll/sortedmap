#include <exception>
#include <map>

#include "sortedmap.h"

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
    PyObject *asset;
    PyObject *ret;

    if (!(asset = PySet_New((PyObject*) self))) {
        return NULL;
    }
    ret = PyUnicode_FromFormat("%s(%R)", Py_TYPE(self)->tp_name, asset);
    Py_DECREF(self);
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

    if (!self) {
        return NULL;
    }

    return new(self) sortedmap::object;
}


sortedmap::object*
sortedmap::newobject(PyTypeObject *cls, PyObject *args, PyObject *kwargs)
{
    sortedmap::object *self;

    if (!(self = innernew(cls))) {
        return NULL;
    }

    if (!sortedmap::update(self, args, kwargs)) {
        Py_DECREF(self);
        return NULL;
    }
    return self;
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

Py_ssize_t
sortedmap::len(sortedmap::object *self)
{
    return self->map.size();
}

PyObject*
sortedmap::getitem(sortedmap::object *self, PyObject *key)
{
    try{
        return self->map.at(key).incref();
    }
    catch (std::out_of_range &e) {
        PyErr_SetObject(PyExc_KeyError, key);
        return NULL;
    }
}

int
sortedmap::setitem(sortedmap::object *self, PyObject *key, PyObject *value)
{
    if (!value) {
        self->map.erase(key);
    }
    else {
        try {
            self->map[key] = value;
        }
        catch (PythonError &e) {
            return -1;
        }
    }
    return 0;
}

int
sortedmap::contains(sortedmap::object *self, PyObject *key)
{
    return self->map.find(key) != self->map.end();
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

    if (!ret) {
        return NULL;
    }

    ret->map = self->map;
    return ret;
}

_Py_IDENTIFIER(keys);

static bool
merge(sortedmap::object *self, PyObject *other)
{
    if (sortedmap::check_exact(other)) {
        try {
            for (const auto &pair : ((sortedmap::object*) other)->map) {
                self->map[std::get<0>(pair)] = std::get<1>(pair);
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
                self->map[key] = value;
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

        if (!(keys = PyMapping_Keys(other))) {
            return false;
        }

        it = PyObject_GetIter(keys);
        Py_DECREF(keys);
        if (!it) {
            return false;
        }
        for (key = PyIter_Next(it); key; key = PyIter_Next(it)) {
            if (!(tmp = PyObject_GetItem(other, key))) {
                Py_DECREF(key);
                Py_DECREF(it);
                return false;
            }
            try {
                self->map[key] = tmp;
            }
            catch (PythonError &e) {
                Py_DECREF(key);
                Py_DECREF(it);
                return false;
            }
        }
        Py_DECREF(key);
        Py_DECREF(it);
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

    if (!(it = PyObject_GetIter(seq2))) {
        return false;
    }

    for (n = 0; ; ++n) {
        PyObject *key, *value;
        Py_ssize_t len;

        fast = NULL;
        if (!(item = PyIter_Next(it))) {
            if (PyErr_Occurred()) {
                goto fail;
            }
            break;
        }

        /* Convert item to sequence, and verify length 2. */
        if (!(fast = PySequence_Fast(item, ""))) {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
                PyErr_Format(PyExc_TypeError,
                             "cannot convert sortedmap update "
                             "sequence element #%zd to a sequence",
                             n);
            goto fail;
        }
        len = PySequence_Fast_GET_SIZE(fast);
        if (len != 2) {
            PyErr_Format(PyExc_ValueError,
                         "sortedmap update sequence element #%zd "
                         "has length %zd; 2 is required",
                         n, len);
            goto fail;
        }

        /* Update/merge with this (key, value) pair. */
        key = PySequence_Fast_GET_ITEM(fast, 0);
        value = PySequence_Fast_GET_ITEM(fast, 1);
        try{
            self->map[key] = value;
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

    if (!PyArg_UnpackTuple(args, "update", 0, 1, &arg)) {
        return false;
    }

    else if (arg) {
        if (_PyObject_HasAttrId(arg, &PyId_keys)) {
            if (!merge(self, arg)) {
                return false;
            }
        }
        else {
            if (!merge_from_seq2(self, arg)) {
                return false;
            }
        }
    }
    if (kwargs) {
        if (PyArg_ValidateKeywordArguments(kwargs)) {
            if (!merge(self, kwargs)) {
                return false;
            }
        }
    }
    return true;
}

PyObject*
sortedmap::pyupdate(sortedmap::object *self, PyObject *args, PyObject *kwargs)
{
    if (!sortedmap::update(self, args, kwargs)) {
        return NULL;
    }
    Py_RETURN_NONE;
}


static struct PyModuleDef _sortedmap_module = {
    PyModuleDef_HEAD_INIT,
    "sortedmap._sortedmap",
    "",
    -1,
};

extern "C" {
PyMODINIT_FUNC
PyInit__sortedmap(void)
{
    PyObject *m;

    if (PyType_Ready(&sortedmap::type)) {
        return NULL;
    }

    if (!(m = PyModule_Create(&_sortedmap_module))) {
        return NULL;
    }

    if (PyModule_AddObject(m, "sortedmap", (PyObject*) &sortedmap::type)) {
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
}
