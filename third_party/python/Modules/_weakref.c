/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=4 sts=4 sw=4 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Python 3                                                                     │
│ https://docs.python.org/3/license.html                                       │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "third_party/python/Include/dictobject.h"
#include "third_party/python/Include/listobject.h"
#include "third_party/python/Include/longobject.h"
#include "third_party/python/Include/modsupport.h"
#include "third_party/python/Include/object.h"
#include "third_party/python/Include/objimpl.h"
#include "third_party/python/Include/weakrefobject.h"
#include "third_party/python/Modules/clinic/_weakref.inc"
/* clang-format off */

#define GET_WEAKREFS_LISTPTR(o) \
        ((PyWeakReference **) PyObject_GET_WEAKREFS_LISTPTR(o))

/*[clinic input]
module _weakref
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=ffec73b85846596d]*/

/*[clinic input]

_weakref.getweakrefcount -> Py_ssize_t

  object: object
  /

Return the number of weak references to 'object'.
[clinic start generated code]*/

static Py_ssize_t
_weakref_getweakrefcount_impl(PyObject *module, PyObject *object)
/*[clinic end generated code: output=301806d59558ff3e input=cedb69711b6a2507]*/
{
    PyWeakReference **list;

    if (!PyType_SUPPORTS_WEAKREFS(Py_TYPE(object)))
        return 0;

    list = GET_WEAKREFS_LISTPTR(object);
    return _PyWeakref_GetWeakrefCount(*list);
}


static int
is_dead_weakref(PyObject *value)
{
    if (!PyWeakref_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "not a weakref");
        return -1;
    }
    return PyWeakref_GET_OBJECT(value) == Py_None;
}

/*[clinic input]

_weakref._remove_dead_weakref -> object

  dct: object(subclass_of='&PyDict_Type')
  key: object
  /

Atomically remove key from dict if it points to a dead weakref.
[clinic start generated code]*/

static PyObject *
_weakref__remove_dead_weakref_impl(PyObject *module, PyObject *dct,
                                   PyObject *key)
/*[clinic end generated code: output=d9ff53061fcb875c input=19fc91f257f96a1d]*/
{
    if (_PyDict_DelItemIf(dct, key, is_dead_weakref) < 0) {
        if (PyErr_ExceptionMatches(PyExc_KeyError))
            /* This function is meant to allow safe weak-value dicts
               with GC in another thread (see issue #28427), so it's
               ok if the key doesn't exist anymore.
               */
            PyErr_Clear();
        else
            return NULL;
    }
    Py_RETURN_NONE;
}


PyDoc_STRVAR(weakref_getweakrefs__doc__,
"getweakrefs(object) -- return a list of all weak reference objects\n"
"that point to 'object'.");

static PyObject *
weakref_getweakrefs(PyObject *self, PyObject *object)
{
    PyObject *result = NULL;

    if (PyType_SUPPORTS_WEAKREFS(Py_TYPE(object))) {
        PyWeakReference **list = GET_WEAKREFS_LISTPTR(object);
        Py_ssize_t count = _PyWeakref_GetWeakrefCount(*list);

        result = PyList_New(count);
        if (result != NULL) {
            PyWeakReference *current = *list;
            Py_ssize_t i;
            for (i = 0; i < count; ++i) {
                PyList_SET_ITEM(result, i, (PyObject *) current);
                Py_INCREF(current);
                current = current->wr_next;
            }
        }
    }
    else {
        result = PyList_New(0);
    }
    return result;
}


PyDoc_STRVAR(weakref_proxy__doc__,
"proxy(object[, callback]) -- create a proxy object that weakly\n"
"references 'object'.  'callback', if given, is called with a\n"
"reference to the proxy when 'object' is about to be finalized.");

static PyObject *
weakref_proxy(PyObject *self, PyObject *args)
{
    PyObject *object;
    PyObject *callback = NULL;
    PyObject *result = NULL;

    if (PyArg_UnpackTuple(args, "proxy", 1, 2, &object, &callback)) {
        result = PyWeakref_NewProxy(object, callback);
    }
    return result;
}


static PyMethodDef
weakref_functions[] =  {
    _WEAKREF_GETWEAKREFCOUNT_METHODDEF
    _WEAKREF__REMOVE_DEAD_WEAKREF_METHODDEF
    {"getweakrefs",     weakref_getweakrefs,            METH_O,
     weakref_getweakrefs__doc__},
    {"proxy",           weakref_proxy,                  METH_VARARGS,
     weakref_proxy__doc__},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef weakrefmodule = {
	PyModuleDef_HEAD_INIT,
	"_weakref",
	"Weak-reference support module.",
	-1,
	weakref_functions,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC
PyInit__weakref(void)
{
    PyObject *m;

    m = PyModule_Create(&weakrefmodule);

    if (m != NULL) {
        Py_INCREF(&_PyWeakref_RefType);
        PyModule_AddObject(m, "ref",
                           (PyObject *) &_PyWeakref_RefType);
        Py_INCREF(&_PyWeakref_RefType);
        PyModule_AddObject(m, "ReferenceType",
                           (PyObject *) &_PyWeakref_RefType);
        Py_INCREF(&_PyWeakref_ProxyType);
        PyModule_AddObject(m, "ProxyType",
                           (PyObject *) &_PyWeakref_ProxyType);
        Py_INCREF(&_PyWeakref_CallableProxyType);
        PyModule_AddObject(m, "CallableProxyType",
                           (PyObject *) &_PyWeakref_CallableProxyType);
    }
    return m;
}
