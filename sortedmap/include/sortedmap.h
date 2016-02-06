#pragma once
#include <exception>
#include <map>

#include <Python.h>

#define likely(condition) __builtin_expect(!!(condition), 1)
#define unlikely(condition) __builtin_expect(!!(condition), 0)

class PythonError : std::exception {};

template<typename T>
class OwnedRef final{
private:
    T *ob;  // The underlying object we own a reference to.

public:
    OwnedRef<T>() {
        ob = NULL;
    }

    OwnedRef<T>(T *ob) {
        if (likely(ob)) {
            Py_INCREF(ob);
        }
        this->ob = ob;
    }

    OwnedRef<T>(const OwnedRef<T> &ref) {
        ob = ref.ob;
        if (likely(ob)) {
            Py_INCREF(ob);
        }
    }

    ~OwnedRef<T>() {
        if (likely(ob)) {
            Py_DECREF(ob);
        }
    }

    T *incref() const {
        Py_INCREF(ob);
        return ob;
    }

    bool operator<(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_LT);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    bool operator<=(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_LE);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    bool operator>(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_GT);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    bool operator>=(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_GE);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    bool operator==(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_EQ);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    bool operator!=(const OwnedRef<T> &b) const {
        int result = PyObject_RichCompareBool(ob, b, Py_NE);
        if (unlikely(result < 0)) {
            throw PythonError();
        }
        return result;
    }

    constexpr operator T*() const {
        return ob;
    }
};

PyObject *py_identity(PyObject*);

namespace sortedmap {
    using maptype = std::map<OwnedRef<PyObject>, OwnedRef<PyObject>>;

    struct object {
        PyObject_HEAD
        maptype map;
    };

    bool check(PyObject*);
    bool check_exact(PyObject*);

    typedef PyObject *iterfunc(object*);
    typedef PyObject *viewfunc(object*);
    object *newobject(PyTypeObject*, PyObject*, PyObject*);
    void dealloc(object*);
    int traverse(object*, visitproc, void*);
    void clear(object*);
    PyObject *pyclear(object*);
    PyObject *richcompare(object*, PyObject*, int);
    Py_ssize_t len(object*);
    PyObject *getitem(object*, PyObject*);
    int setitem(object*, PyObject*, PyObject*);
    int contains(object*, PyObject*);
    PyObject *repr(object*);
    object *copy(object*);
    bool update(object*, PyObject*, PyObject*);
    PyObject *pyupdate(object*, PyObject*, PyObject*);

    namespace abstractiter {
        using itertype = decltype(std::declval<maptype>().cbegin());
        typedef PyObject *extract_element(itertype);

        struct object {
            PyObject_HEAD
            itertype iter;
            decltype(std::declval<maptype>().cend()) end;
            OwnedRef<sortedmap::object> map;
        };

        void dealloc(object*);

        template<sortedmap::abstractiter::extract_element f>
        PyObject*
        next(object *self)
        {
            PyObject *ret;

            if (unlikely(self->iter == self->end)) {
                return NULL;
            }

            ret = f(self->iter);
            self->iter = std::move(std::next(self->iter, 1));
            return ret;
        }

        template<typename iterobject, PyTypeObject &cls>
        PyObject*
        iter(sortedmap::object *self)
        {
            iterobject *ret = PyObject_New(iterobject, &cls);
            if (!ret) {
                return NULL;
            }

            ret->iter = std::move(self->map.cbegin());
            ret->end = std::move(self->map.cend());
            new(&ret->map) OwnedRef<sortedmap::object>(self);
            return (PyObject*) ret;
        }
    }

    namespace keyiter {
        using object = abstractiter::object;

        abstractiter::extract_element elem;
        iterfunc iter;

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.keyiter",                        // tp_name
            sizeof(object),                             // tp_basicsize
            0,                                          // tp_itemsize
            (destructor) abstractiter::dealloc,         // tp_dealloc
            0,                                          // tp_print
            0,                                          // tp_getattr
            0,                                          // tp_setattr
            0,                                          // tp_reserved
            0,                                          // tp_repr
            0,                                          // tp_as_number
            0,                                          // tp_as_sequence
            0,                                          // tp_as_mapping
            0,                                          // tp_hash
            0,                                          // tp_call
            0,                                          // tp_str
            0,                                          // tp_getattro
            0,                                          // tp_setattro
            0,                                          // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                         // tp_flags
            0,                                          // tp_doc
            0,                                          // tp_traverse
            0,                                          // tp_clear
            0,                                          // tp_richcompare
            0,                                          // tp_weaklistoffset
            (getiterfunc) py_identity,                  // tp_iter
            (iternextfunc) abstractiter::next<elem>,    // tp_iternext
        };
    }

    namespace valiter {
        using object = abstractiter::object;

        abstractiter::extract_element elem;
        iterfunc iter;

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.valiter",                        // tp_name
            sizeof(object),                             // tp_basicsize
            0,                                          // tp_itemsize
            (destructor) abstractiter::dealloc,         // tp_dealloc
            0,                                          // tp_print
            0,                                          // tp_getattr
            0,                                          // tp_setattr
            0,                                          // tp_reserved
            0,                                          // tp_repr
            0,                                          // tp_as_number
            0,                                          // tp_as_sequence
            0,                                          // tp_as_mapping
            0,                                          // tp_hash
            0,                                          // tp_call
            0,                                          // tp_str
            0,                                          // tp_getattro
            0,                                          // tp_setattro
            0,                                          // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                         // tp_flags
            0,                                          // tp_doc
            0,                                          // tp_traverse
            0,                                          // tp_clear
            0,                                          // tp_richcompare
            0,                                          // tp_weaklistoffset
            (getiterfunc) py_identity,                  // tp_iter
            (iternextfunc) abstractiter::next<elem>,    // tp_iternext
        };
    }

    namespace itemiter {
        using object = abstractiter::object;

        abstractiter::extract_element elem;
        iterfunc iter;

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.itemiter",                       // tp_name
            sizeof(object),                             // tp_basicsize
            0,                                          // tp_itemsize
            (destructor) abstractiter::dealloc,         // tp_dealloc
            0,                                          // tp_print
            0,                                          // tp_getattr
            0,                                          // tp_setattr
            0,                                          // tp_reserved
            0,                                          // tp_repr
            0,                                          // tp_as_number
            0,                                          // tp_as_sequence
            0,                                          // tp_as_mapping
            0,                                          // tp_hash
            0,                                          // tp_call
            0,                                          // tp_str
            0,                                          // tp_getattro
            0,                                          // tp_setattro
            0,                                          // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                         // tp_flags
            0,                                          // tp_doc
            0,                                          // tp_traverse
            0,                                          // tp_clear
            0,                                          // tp_richcompare
            0,                                          // tp_weaklistoffset
            (getiterfunc) py_identity,                  // tp_iter
            (iternextfunc) abstractiter::next<elem>,    // tp_iternext
        };
    }

    namespace abstractview {
        struct object {
            PyObject_HEAD
            OwnedRef<sortedmap::object> map;
        };

        void dealloc(object*);
        PyObject *repr(object*);

        template<typename viewobject, PyTypeObject &cls>
        PyObject*
        view(sortedmap::object *self)
        {
            viewobject *ret = PyObject_New(viewobject, &cls);
            if (!ret) {
                return NULL;
            }

            new(&ret->map) OwnedRef<sortedmap::object>(self);
            return (PyObject*) ret;
        }

        template<sortedmap::iterfunc iter, binaryfunc f>
        PyObject*
        binop(object *self, PyObject* other)
        {
            PyObject *it;
            PyObject *lhs;
            PyObject *rhs;
            PyObject *res;

            if (!(it = iter(self->map))) {
                return NULL;
            }

            lhs = PySet_New(other);
            Py_DECREF(it);
            if (!lhs) {
                return NULL;
            }

            if (!(rhs = PySet_New(other))) {
                Py_DECREF(lhs);
                return NULL;
            }
            res = f(lhs, rhs);
            Py_DECREF(lhs);
            Py_DECREF(rhs);
            return res;
        }

        template<sortedmap::iterfunc iter>
        PyObject*
        richcompare(object *self, PyObject *other, int opid)
        {
            PyObject *it;
            PyObject *lhs;
            PyObject *rhs;
            PyObject *res;

            if (!(it = iter(self->map))) {
                return NULL;
            }

            lhs = PySet_New(it);
            Py_DECREF(it);
            if (!lhs) {
                Py_DECREF(lhs);
                return NULL;
            }

            if (!(rhs = PySet_New(other))) {
                Py_DECREF(lhs);
                return NULL;
            }
            res = PyObject_RichCompare(lhs, rhs, opid);
            Py_DECREF(lhs);
            Py_DECREF(rhs);
            return res;
        }

        template<sortedmap::iterfunc iterf>
        PyObject*
        iter(object *self)
        {
            return iterf(self->map);
        }
    }

    namespace keyview {
        using object = abstractview::object;

        viewfunc view;

        PyNumberMethods as_number = {
            0,                                              // nb_add
            (binaryfunc) abstractview::binop<keyiter::iter,
                                             PyNumber_Add>, // nb_subtract
            0,                                              // nb_multiply
            0,                                              // nb_remainder
            0,                                              // nb_divmod
            0,                                              // nb_power
            0,                                              // nb_negative
            0,                                              // nb_positive
            0,                                              // nb_absolute
            0,                                              // nb_bool
            0,                                              // nb_invert
            0,                                              // nb_lshift
            0,                                              // nb_rshift
            (binaryfunc) abstractview::binop<keyiter::iter,
                                             PyNumber_And>, // nb_and
            (binaryfunc) abstractview::binop<keyiter::iter,
                                             PyNumber_Xor>, // nb_xor
            (binaryfunc) abstractview::binop<keyiter::iter,
                                             PyNumber_Or>,  // nb_or
        };

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.keyview",                            // tp_name
            sizeof(object),                                 // tp_basicsize
            0,                                              // tp_itemsize
            (destructor) abstractview::dealloc,             // tp_dealloc
            0,                                              // tp_print
            0,                                              // tp_getattr
            0,                                              // tp_setattr
            0,                                              // tp_reserved
            (reprfunc) abstractview::repr,                  // tp_repr
            &as_number,                                     // tp_as_number
            0,                                              // tp_as_sequence
            0,                                              // tp_as_mapping
            0,                                              // tp_hash
            0,                                              // tp_call
            (reprfunc) abstractview::repr,                  // tp_str
            0,                                              // tp_getattro
            0,                                              // tp_setattro
            0,                                              // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                             // tp_flags
            0,                                              // tp_doc
            0,                                              // tp_traverse
            0,                                              // tp_clear
            (richcmpfunc) abstractview::richcompare<keyiter::iter>,
            0,                                              // tp_weaklistoffset
            (getiterfunc) abstractview::iter<keyiter::iter> // tp_iter
        };
    }

    namespace valview {
        using object = abstractview::object;

        viewfunc view;

        PyNumberMethods as_number = {
            0,                                              // nb_add
            (binaryfunc) abstractview::binop<valiter::iter,
                                             PyNumber_Add>, // nb_subtract
            0,                                              // nb_multiply
            0,                                              // nb_remainder
            0,                                              // nb_divmod
            0,                                              // nb_power
            0,                                              // nb_negative
            0,                                              // nb_positive
            0,                                              // nb_absolute
            0,                                              // nb_bool
            0,                                              // nb_invert
            0,                                              // nb_lshift
            0,                                              // nb_rshift
            (binaryfunc) abstractview::binop<valiter::iter,
                                             PyNumber_And>, // nb_and
            (binaryfunc) abstractview::binop<valiter::iter,
                                             PyNumber_Xor>, // nb_xor
            (binaryfunc) abstractview::binop<valiter::iter,
                                             PyNumber_Or>,  // nb_or
        };

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.valview",                            // tp_name
            sizeof(object),                                 // tp_basicsize
            0,                                              // tp_itemsize
            (destructor) abstractview::dealloc,             // tp_dealloc
            0,                                              // tp_print
            0,                                              // tp_getattr
            0,                                              // tp_setattr
            0,                                              // tp_reserved
            (reprfunc) abstractview::repr,                  // tp_repr
            &as_number,                                     // tp_as_number
            0,                                              // tp_as_sequence
            0,                                              // tp_as_mapping
            0,                                              // tp_hash
            0,                                              // tp_call
            (reprfunc) abstractview::repr,                  // tp_str
            0,                                              // tp_getattro
            0,                                              // tp_setattro
            0,                                              // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                             // tp_flags
            0,                                              // tp_doc
            0,                                              // tp_traverse
            0,                                              // tp_clear
            (richcmpfunc) abstractview::richcompare<valiter::iter>,
            0,                                              // tp_weaklistoffset
            (getiterfunc) abstractview::iter<valiter::iter> // tp_iter
        };
    }

    namespace itemview {
        using object = abstractview::object;

        viewfunc view;

        PyNumberMethods as_number = {
            0,                                              // nb_add
            (binaryfunc) abstractview::binop<itemiter::iter,
                                             PyNumber_Add>, // nb_subtract
            0,                                              // nb_multiply
            0,                                              // nb_remainder
            0,                                              // nb_divmod
            0,                                              // nb_power
            0,                                              // nb_negative
            0,                                              // nb_positive
            0,                                              // nb_absolute
            0,                                              // nb_bool
            0,                                              // nb_invert
            0,                                              // nb_lshift
            0,                                              // nb_rshift
            (binaryfunc) abstractview::binop<itemiter::iter,
                                             PyNumber_And>, // nb_and
            (binaryfunc) abstractview::binop<itemiter::iter,
                                             PyNumber_Xor>, // nb_xor
            (binaryfunc) abstractview::binop<itemiter::iter,
                                             PyNumber_Or>,  // nb_or
        };

        PyTypeObject type = {
            PyVarObject_HEAD_INIT(&PyType_Type, 0)
            "sortedmap.itemview",                           // tp_name
            sizeof(object),                                 // tp_basicsize
            0,                                              // tp_itemsize
            (destructor) abstractview::dealloc,             // tp_dealloc
            0,                                              // tp_print
            0,                                              // tp_getattr
            0,                                              // tp_setattr
            0,                                              // tp_reserved
            (reprfunc) abstractview::repr,                  // tp_repr
            &as_number,                                     // tp_as_number
            0,                                              // tp_as_sequence
            0,                                              // tp_as_mapping
            0,                                              // tp_hash
            0,                                              // tp_call
            (reprfunc) abstractview::repr,                  // tp_str
            0,                                              // tp_getattro
            0,                                              // tp_setattro
            0,                                              // tp_as_buffer
            Py_TPFLAGS_DEFAULT,                             // tp_flags
            0,                                              // tp_doc
            0,                                              // tp_traverse
            0,                                              // tp_clear
            (richcmpfunc) abstractview::richcompare<itemiter::iter>,
            0,                                              // tp_weaklistoffset
            (getiterfunc) abstractview::iter<itemiter::iter>// tp_iter
        };
    }

    PySequenceMethods as_sequence = {
        0,                                          // sq_length
        0,                                          // sq_concat
        0,                                          // sq_repeat
        0,                                          // sq_item
        0,                                          // placeholder
        0,                                          // sq_ass_item
        0,                                          // placeholder
        (objobjproc) contains,                      // sq_contains
    };

    PyMappingMethods as_mapping = {
        (lenfunc) len,                              // mp_length
        (binaryfunc) getitem,                       // mp_subscript
        (objobjargproc) setitem,                    // mp_ass_subscript
    };

    PyDoc_STRVAR(keys_doc,
                 "Returns\n"
                 "-------\n"
                 "v : key_view\n"
                 "    A set-like object providing a view on map's keys.\n");
    PyDoc_STRVAR(values_doc,
                 "Returns\n"
                 "-------\n"
                 "v : value_view\n"
                 "    A set-like object providing a view on map's values.\n");
    PyDoc_STRVAR(items_doc,
                 "Returns\n"
                 "-------\n"
                 "v : value_view\n"
                 "    A set-like object providing a view on map's items.\n");
    PyDoc_STRVAR(clear_doc,
                 "Remove all items from the map.");
    PyDoc_STRVAR(copy_doc,
                 "Returns\n"
                 "-------\n"
                 "copy : sortedmap\n"
                 "    A shallow copy of this sortedmap.\n");
    PyDoc_STRVAR(update_doc,
                 "Update the sortedmap from a mapping or iterable.\n"
                 "\n"
                 "Parameters\n"
                 "----------\n"
                 "it : iterable[key, value]\n"
                 "**kwargs\n"
                 "    The mappings to update this sortedmap with.\n");

    PyMethodDef methods[] = {
        {"keys", (PyCFunction) keyview::view, METH_NOARGS, keys_doc},
        {"values", (PyCFunction) valview::view, METH_NOARGS, values_doc},
        {"items", (PyCFunction) itemview::view, METH_NOARGS, items_doc},
        {"clear", (PyCFunction) pyclear, METH_NOARGS, clear_doc},
        {"copy", (PyCFunction) copy, METH_NOARGS, copy_doc},
        {"update", (PyCFunction) pyupdate, METH_KEYWORDS, update_doc},
        {NULL},
    };

    PyDoc_STRVAR(sortedmap_doc,
                 "A sorted mapping that does not use hashing.\n"
                 "\n"
                 "Parameters\n"
                 "----------\n"
                 "mapping : mapping\n"
                 "**kwargs\n"
                 "    The initial mapping.\n");

    PyTypeObject type = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "sortedmap.sortedmap",                      // tp_name
        sizeof(object),                             // tp_basicsize
        0,                                          // tp_itemsize
        (destructor) dealloc,                       // tp_dealloc
        0,                                          // tp_print
        0,                                          // tp_getattr
        0,                                          // tp_setattr
        0,                                          // tp_reserved
        (reprfunc) repr,                            // tp_repr
        0,                                          // tp_as_number
        &as_sequence,                               // tp_as_sequence
        &as_mapping,                                // tp_as_mapping
        0,                                          // tp_hash
        0,                                          // tp_call
        (reprfunc) repr,                            // tp_str
        0,                                          // tp_getattro
        0,                                          // tp_setattro
        0,                                          // tp_as_buffer
        Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,                         // tp_flags
        sortedmap_doc,                              // tp_doc
        (traverseproc) traverse,                    // tp_traverse
        (inquiry) clear,                            // tp_clear
        (richcmpfunc) richcompare,                  // tp_richcompare
        0,                                          // tp_weaklistoffset
        (getiterfunc) keyiter::iter,                // tp_iter
        0,                                          // tp_iternext
        methods,                                    // tp_methods
        0,                                          // tp_members
        0,                                          // tp_getset
        0,                                          // tp_base
        0,                                          // tp_dict
        0,                                          // tp_descr_get
        0,                                          // tp_descr_set
        0,                                          // tp_dictoffset
        0,                                          // tp_init
        0,                                          // tp_alloc
        (newfunc) newobject,                        // tp_new
    };
};
