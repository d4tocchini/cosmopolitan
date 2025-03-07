/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=4 sts=4 sw=4 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Python 3                                                                     │
│ https://docs.python.org/3/license.html                                       │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "third_party/python/Include/abstract.h"
#include "third_party/python/Include/longobject.h"
#include "third_party/python/Include/modsupport.h"
#include "third_party/python/Include/moduleobject.h"
#include "third_party/python/Include/object.h"
#include "third_party/python/Include/objimpl.h"
#include "third_party/python/Include/pgenheaders.h"
#include "third_party/python/Include/pyerrors.h"
#include "third_party/python/Include/pylifecycle.h"
#include "third_party/python/Include/pymacro.h"
#include "third_party/python/Include/pymem.h"
#include "third_party/python/Include/pystate.h"
#include "third_party/python/Include/pythonrun.h"
/* clang-format off */

/*
 *  atexit - allow programmer to define multiple exit functions to be executed
 *  upon normal program termination.
 *
 *   Translated from atexit.py by Collin Winter.
 +   Copyright 2007 Python Software Foundation.
 */

/* Forward declaration (for atexit_cleanup) */
static PyObject *atexit_clear(PyObject*, PyObject*);
/* Forward declaration of module object */
static struct PyModuleDef atexitmodule;

/* ===================================================================== */
/* Callback machinery. */

typedef struct {
    PyObject *func;
    PyObject *args;
    PyObject *kwargs;
} atexit_callback;

typedef struct {
    atexit_callback **atexit_callbacks;
    int ncallbacks;
    int callback_len;
} atexitmodule_state;

#define GET_ATEXIT_STATE(mod) ((atexitmodule_state*)PyModule_GetState(mod))


static void
atexit_delete_cb(atexitmodule_state *modstate, int i)
{
    atexit_callback *cb;

    cb = modstate->atexit_callbacks[i];
    modstate->atexit_callbacks[i] = NULL;
    Py_DECREF(cb->func);
    Py_DECREF(cb->args);
    Py_XDECREF(cb->kwargs);
    PyMem_Free(cb);
}

/* Clear all callbacks without calling them */
static void
atexit_cleanup(atexitmodule_state *modstate)
{
    atexit_callback *cb;
    int i;
    for (i = 0; i < modstate->ncallbacks; i++) {
        cb = modstate->atexit_callbacks[i];
        if (cb == NULL)
            continue;

        atexit_delete_cb(modstate, i);
    }
    modstate->ncallbacks = 0;
}

/* Installed into pylifecycle.c's atexit mechanism */

static void
atexit_callfuncs(void)
{
    PyObject *exc_type = NULL, *exc_value, *exc_tb, *r;
    atexit_callback *cb;
    PyObject *module;
    atexitmodule_state *modstate;
    int i;

    module = PyState_FindModule(&atexitmodule);
    if (module == NULL)
        return;
    modstate = GET_ATEXIT_STATE(module);

    if (modstate->ncallbacks == 0)
        return;


    for (i = modstate->ncallbacks - 1; i >= 0; i--)
    {
        cb = modstate->atexit_callbacks[i];
        if (cb == NULL)
            continue;

        r = PyObject_Call(cb->func, cb->args, cb->kwargs);
        Py_XDECREF(r);
        if (r == NULL) {
            /* Maintain the last exception, but don't leak if there are
               multiple exceptions. */
            if (exc_type) {
                Py_DECREF(exc_type);
                Py_XDECREF(exc_value);
                Py_XDECREF(exc_tb);
            }
            PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
            if (!PyErr_GivenExceptionMatches(exc_type, PyExc_SystemExit)) {
                PySys_WriteStderr("Error in atexit._run_exitfuncs:\n");
                PyErr_NormalizeException(&exc_type, &exc_value, &exc_tb);
                PyErr_Display(exc_type, exc_value, exc_tb);
            }
        }
    }

    atexit_cleanup(modstate);

    if (exc_type)
        PyErr_Restore(exc_type, exc_value, exc_tb);
}

/* ===================================================================== */
/* Module methods. */

PyDoc_STRVAR(atexit_register__doc__,
"register(func, *args, **kwargs) -> func\n\
\n\
Register a function to be executed upon normal program termination\n\
\n\
    func - function to be called at exit\n\
    args - optional arguments to pass to func\n\
    kwargs - optional keyword arguments to pass to func\n\
\n\
    func is returned to facilitate usage as a decorator.");

static PyObject *
atexit_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    atexitmodule_state *modstate;
    atexit_callback *new_callback;
    PyObject *func = NULL;

    modstate = GET_ATEXIT_STATE(self);

    if (modstate->ncallbacks >= modstate->callback_len) {
        atexit_callback **r;
        modstate->callback_len += 16;
        r = (atexit_callback**)PyMem_Realloc(modstate->atexit_callbacks,
                                      sizeof(atexit_callback*) * modstate->callback_len);
        if (r == NULL)
            return PyErr_NoMemory();
        modstate->atexit_callbacks = r;
    }

    if (PyTuple_GET_SIZE(args) == 0) {
        PyErr_SetString(PyExc_TypeError,
                "register() takes at least 1 argument (0 given)");
        return NULL;
    }

    func = PyTuple_GET_ITEM(args, 0);
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError,
                "the first argument must be callable");
        return NULL;
    }

    new_callback = PyMem_Malloc(sizeof(atexit_callback));
    if (new_callback == NULL)
        return PyErr_NoMemory();

    new_callback->args = PyTuple_GetSlice(args, 1, PyTuple_GET_SIZE(args));
    if (new_callback->args == NULL) {
        PyMem_Free(new_callback);
        return NULL;
    }
    new_callback->func = func;
    new_callback->kwargs = kwargs;
    Py_INCREF(func);
    Py_XINCREF(kwargs);

    modstate->atexit_callbacks[modstate->ncallbacks++] = new_callback;

    Py_INCREF(func);
    return func;
}

PyDoc_STRVAR(atexit_run_exitfuncs__doc__,
"_run_exitfuncs() -> None\n\
\n\
Run all registered exit functions.");

static PyObject *
atexit_run_exitfuncs(PyObject *self, PyObject *unused)
{
    atexit_callfuncs();
    if (PyErr_Occurred())
        return NULL;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(atexit_clear__doc__,
"_clear() -> None\n\
\n\
Clear the list of previously registered exit functions.");

static PyObject *
atexit_clear(PyObject *self, PyObject *unused)
{
    atexit_cleanup(GET_ATEXIT_STATE(self));
    Py_RETURN_NONE;
}

PyDoc_STRVAR(atexit_ncallbacks__doc__,
"_ncallbacks() -> int\n\
\n\
Return the number of registered exit functions.");

static PyObject *
atexit_ncallbacks(PyObject *self, PyObject *unused)
{
    atexitmodule_state *modstate;

    modstate = GET_ATEXIT_STATE(self);

    return PyLong_FromSsize_t(modstate->ncallbacks);
}

static int
atexit_m_traverse(PyObject *self, visitproc visit, void *arg)
{
    int i;
    atexitmodule_state *modstate;

    modstate = GET_ATEXIT_STATE(self);
    for (i = 0; i < modstate->ncallbacks; i++) {
        atexit_callback *cb = modstate->atexit_callbacks[i];
        if (cb == NULL)
            continue;
        Py_VISIT(cb->func);
        Py_VISIT(cb->args);
        Py_VISIT(cb->kwargs);
    }
    return 0;
}

static int
atexit_m_clear(PyObject *self)
{
    atexitmodule_state *modstate;
    modstate = GET_ATEXIT_STATE(self);
    atexit_cleanup(modstate);
    return 0;
}

static void
atexit_free(PyObject *m)
{
    atexitmodule_state *modstate;
    modstate = GET_ATEXIT_STATE(m);
    atexit_cleanup(modstate);
    PyMem_Free(modstate->atexit_callbacks);
}

PyDoc_STRVAR(atexit_unregister__doc__,
"unregister(func) -> None\n\
\n\
Unregister an exit function which was previously registered using\n\
atexit.register\n\
\n\
    func - function to be unregistered");

static PyObject *
atexit_unregister(PyObject *self, PyObject *func)
{
    atexitmodule_state *modstate;
    atexit_callback *cb;
    int i, eq;

    modstate = GET_ATEXIT_STATE(self);

    for (i = 0; i < modstate->ncallbacks; i++)
    {
        cb = modstate->atexit_callbacks[i];
        if (cb == NULL)
            continue;

        eq = PyObject_RichCompareBool(cb->func, func, Py_EQ);
        if (eq < 0)
            return NULL;
        if (eq)
            atexit_delete_cb(modstate, i);
    }
    Py_RETURN_NONE;
}

static PyMethodDef atexit_methods[] = {
    {"register", (PyCFunction) atexit_register, METH_VARARGS|METH_KEYWORDS,
        atexit_register__doc__},
    {"_clear", (PyCFunction) atexit_clear, METH_NOARGS,
        atexit_clear__doc__},
    {"unregister", (PyCFunction) atexit_unregister, METH_O,
        atexit_unregister__doc__},
    {"_run_exitfuncs", (PyCFunction) atexit_run_exitfuncs, METH_NOARGS,
        atexit_run_exitfuncs__doc__},
    {"_ncallbacks", (PyCFunction) atexit_ncallbacks, METH_NOARGS,
        atexit_ncallbacks__doc__},
    {NULL, NULL}        /* sentinel */
};

/* ===================================================================== */
/* Initialization function. */

PyDoc_STRVAR(atexit__doc__,
"allow programmer to define multiple exit functions to be executed\
upon normal program termination.\n\
\n\
Two public functions, register and unregister, are defined.\n\
");


static struct PyModuleDef atexitmodule = {
    PyModuleDef_HEAD_INIT,
    "atexit",
    atexit__doc__,
    sizeof(atexitmodule_state),
    atexit_methods,
    NULL,
    atexit_m_traverse,
    atexit_m_clear,
    (freefunc)atexit_free
};

PyMODINIT_FUNC
PyInit_atexit(void)
{
    PyObject *m;
    atexitmodule_state *modstate;

    m = PyModule_Create(&atexitmodule);
    if (m == NULL)
        return NULL;

    modstate = GET_ATEXIT_STATE(m);
    modstate->callback_len = 32;
    modstate->ncallbacks = 0;
    modstate->atexit_callbacks = PyMem_New(atexit_callback*,
                                           modstate->callback_len);
    if (modstate->atexit_callbacks == NULL)
        return NULL;

    _Py_PyAtExit(atexit_callfuncs);
    return m;
}
