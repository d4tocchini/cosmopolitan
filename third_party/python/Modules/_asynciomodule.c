/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=4 sts=4 sw=4 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Python 3                                                                     │
│ https://docs.python.org/3/license.html                                       │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "third_party/python/Include/abstract.h"
#include "third_party/python/Include/boolobject.h"
#include "third_party/python/Include/descrobject.h"
#include "third_party/python/Include/dictobject.h"
#include "third_party/python/Include/genobject.h"
#include "third_party/python/Include/import.h"
#include "third_party/python/Include/listobject.h"
#include "third_party/python/Include/longobject.h"
#include "third_party/python/Include/modsupport.h"
#include "third_party/python/Include/object.h"
#include "third_party/python/Include/objimpl.h"
#include "third_party/python/Include/pyerrors.h"
#include "third_party/python/Include/pymacro.h"
#include "third_party/python/Include/setobject.h"
#include "third_party/python/Include/structmember.h"
#include "third_party/python/Include/traceback.h"
/* clang-format off */

/*[clinic input]
module _asyncio
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=8fd17862aa989c69]*/


/* identifiers used from some functions */
_Py_IDENTIFIER(add_done_callback);
_Py_IDENTIFIER(call_soon);
_Py_IDENTIFIER(cancel);
_Py_IDENTIFIER(send);
_Py_IDENTIFIER(throw);
_Py_IDENTIFIER(_step);
_Py_IDENTIFIER(_schedule_callbacks);
_Py_IDENTIFIER(_wakeup);


/* State of the _asyncio module */
static PyObject *all_tasks;
static PyObject *current_tasks;
static PyObject *traceback_extract_stack;
static PyObject *asyncio_get_event_loop;
static PyObject *asyncio_future_repr_info_func;
static PyObject *asyncio_task_repr_info_func;
static PyObject *asyncio_task_get_stack_func;
static PyObject *asyncio_task_print_stack_func;
static PyObject *asyncio_InvalidStateError;
static PyObject *asyncio_CancelledError;
static PyObject *inspect_isgenerator;


typedef enum {
    STATE_PENDING,
    STATE_CANCELLED,
    STATE_FINISHED
} fut_state;

#define FutureObj_HEAD(prefix)                                              \
    PyObject_HEAD                                                           \
    PyObject *prefix##_loop;                                                \
    PyObject *prefix##_callbacks;                                           \
    PyObject *prefix##_exception;                                           \
    PyObject *prefix##_result;                                              \
    PyObject *prefix##_source_tb;                                           \
    fut_state prefix##_state;                                               \
    int prefix##_log_tb;                                                    \
    int prefix##_blocking;                                                  \
    PyObject *dict;                                                         \
    PyObject *prefix##_weakreflist;

typedef struct {
    FutureObj_HEAD(fut)
} FutureObj;

typedef struct {
    FutureObj_HEAD(task)
    PyObject *task_fut_waiter;
    PyObject *task_coro;
    int task_must_cancel;
    int task_log_destroy_pending;
} TaskObj;

typedef struct {
    PyObject_HEAD
    TaskObj *sw_task;
    PyObject *sw_arg;
} TaskStepMethWrapper;

typedef struct {
    PyObject_HEAD
    TaskObj *ww_task;
} TaskWakeupMethWrapper;


#include "third_party/python/Modules/clinic/_asynciomodule.inc"


/*[clinic input]
class _asyncio.Future "FutureObj *" "&Future_Type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=00d3e4abca711e0f]*/

/* Get FutureIter from Future */
static PyObject* future_new_iter(PyObject *);
static inline int future_call_schedule_callbacks(FutureObj *);

static int
future_schedule_callbacks(FutureObj *fut)
{
    Py_ssize_t len;
    PyObject *callbacks;
    int i;

    if (fut->fut_callbacks == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "uninitialized Future object");
        return -1;
    }

    len = PyList_GET_SIZE(fut->fut_callbacks);
    if (len == 0) {
        return 0;
    }

    callbacks = PyList_GetSlice(fut->fut_callbacks, 0, len);
    if (callbacks == NULL) {
        return -1;
    }
    if (PyList_SetSlice(fut->fut_callbacks, 0, len, NULL) < 0) {
        Py_DECREF(callbacks);
        return -1;
    }

    for (i = 0; i < len; i++) {
        PyObject *handle;
        PyObject *cb = PyList_GET_ITEM(callbacks, i);

        handle = _PyObject_CallMethodIdObjArgs(fut->fut_loop, &PyId_call_soon,
                                               cb, fut, NULL);

        if (handle == NULL) {
            Py_DECREF(callbacks);
            return -1;
        }
        Py_DECREF(handle);
    }

    Py_DECREF(callbacks);
    return 0;
}


static int
future_init(FutureObj *fut, PyObject *loop)
{
    PyObject *res;
    int is_true;
    _Py_IDENTIFIER(get_debug);

    // Same to FutureObj_clear() but not clearing fut->dict
    Py_CLEAR(fut->fut_loop);
    Py_CLEAR(fut->fut_callbacks);
    Py_CLEAR(fut->fut_result);
    Py_CLEAR(fut->fut_exception);
    Py_CLEAR(fut->fut_source_tb);

    fut->fut_state = STATE_PENDING;
    fut->fut_log_tb = 0;
    fut->fut_blocking = 0;

    if (loop == Py_None) {
        loop = _PyObject_CallNoArg(asyncio_get_event_loop);
        if (loop == NULL) {
            return -1;
        }
    }
    else {
        Py_INCREF(loop);
    }
    fut->fut_loop = loop;

    res = _PyObject_CallMethodId(fut->fut_loop, &PyId_get_debug, NULL);
    if (res == NULL) {
        return -1;
    }
    is_true = PyObject_IsTrue(res);
    Py_DECREF(res);
    if (is_true < 0) {
        return -1;
    }
    if (is_true) {
        fut->fut_source_tb = _PyObject_CallNoArg(traceback_extract_stack);
        if (fut->fut_source_tb == NULL) {
            return -1;
        }
    }

    fut->fut_callbacks = PyList_New(0);
    if (fut->fut_callbacks == NULL) {
        return -1;
    }

    return 0;
}

static PyObject *
future_set_result(FutureObj *fut, PyObject *res)
{
    if (fut->fut_state != STATE_PENDING) {
        PyErr_SetString(asyncio_InvalidStateError, "invalid state");
        return NULL;
    }

    assert(!fut->fut_result);
    Py_INCREF(res);
    fut->fut_result = res;
    fut->fut_state = STATE_FINISHED;

    if (future_call_schedule_callbacks(fut) == -1) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *
future_set_exception(FutureObj *fut, PyObject *exc)
{
    PyObject *exc_val = NULL;

    if (fut->fut_state != STATE_PENDING) {
        PyErr_SetString(asyncio_InvalidStateError, "invalid state");
        return NULL;
    }

    if (PyExceptionClass_Check(exc)) {
        exc_val = PyObject_CallObject(exc, NULL);
        if (exc_val == NULL) {
            return NULL;
        }
        if (fut->fut_state != STATE_PENDING) {
            Py_DECREF(exc_val);
            PyErr_SetString(asyncio_InvalidStateError, "invalid state");
            return NULL;
        }
    }
    else {
        exc_val = exc;
        Py_INCREF(exc_val);
    }
    if (!PyExceptionInstance_Check(exc_val)) {
        Py_DECREF(exc_val);
        PyErr_SetString(PyExc_TypeError, "invalid exception object");
        return NULL;
    }
    if ((PyObject*)Py_TYPE(exc_val) == PyExc_StopIteration) {
        Py_DECREF(exc_val);
        PyErr_SetString(PyExc_TypeError,
                        "StopIteration interacts badly with generators "
                        "and cannot be raised into a Future");
        return NULL;
    }

    assert(!fut->fut_exception);
    fut->fut_exception = exc_val;
    fut->fut_state = STATE_FINISHED;

    if (future_call_schedule_callbacks(fut) == -1) {
        return NULL;
    }

    fut->fut_log_tb = 1;
    Py_RETURN_NONE;
}

static int
future_get_result(FutureObj *fut, PyObject **result)
{
    if (fut->fut_state == STATE_CANCELLED) {
        PyErr_SetNone(asyncio_CancelledError);
        return -1;
    }

    if (fut->fut_state != STATE_FINISHED) {
        PyErr_SetString(asyncio_InvalidStateError, "Result is not set.");
        return -1;
    }

    fut->fut_log_tb = 0;
    if (fut->fut_exception != NULL) {
        Py_INCREF(fut->fut_exception);
        *result = fut->fut_exception;
        return 1;
    }

    Py_INCREF(fut->fut_result);
    *result = fut->fut_result;
    return 0;
}

static PyObject *
future_add_done_callback(FutureObj *fut, PyObject *arg)
{
    if (fut->fut_state != STATE_PENDING) {
        PyObject *handle = _PyObject_CallMethodIdObjArgs(fut->fut_loop,
                                                         &PyId_call_soon,
                                                         arg, fut, NULL);
        if (handle == NULL) {
            return NULL;
        }
        Py_DECREF(handle);
    }
    else {
        if (fut->fut_callbacks == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "uninitialized Future object");
            return NULL;
        }
        int err = PyList_Append(fut->fut_callbacks, arg);
        if (err != 0) {
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

static PyObject *
future_cancel(FutureObj *fut)
{
    fut->fut_log_tb = 0;

    if (fut->fut_state != STATE_PENDING) {
        Py_RETURN_FALSE;
    }
    fut->fut_state = STATE_CANCELLED;

    if (future_call_schedule_callbacks(fut) == -1) {
        return NULL;
    }

    Py_RETURN_TRUE;
}

/*[clinic input]
_asyncio.Future.__init__

    *
    loop: object = None

This class is *almost* compatible with concurrent.futures.Future.

    Differences:

    - result() and exception() do not take a timeout argument and
      raise an exception when the future isn't done yet.

    - Callbacks registered with add_done_callback() are always called
      via the event loop's call_soon_threadsafe().

    - This class is not compatible with the wait() and as_completed()
      methods in the concurrent.futures package.
[clinic start generated code]*/

static int
_asyncio_Future___init___impl(FutureObj *self, PyObject *loop)
/*[clinic end generated code: output=9ed75799eaccb5d6 input=89af317082bc0bf8]*/

{
    return future_init(self, loop);
}

static int
FutureObj_clear(FutureObj *fut)
{
    Py_CLEAR(fut->fut_loop);
    Py_CLEAR(fut->fut_callbacks);
    Py_CLEAR(fut->fut_result);
    Py_CLEAR(fut->fut_exception);
    Py_CLEAR(fut->fut_source_tb);
    Py_CLEAR(fut->dict);
    return 0;
}

static int
FutureObj_traverse(FutureObj *fut, visitproc visit, void *arg)
{
    Py_VISIT(fut->fut_loop);
    Py_VISIT(fut->fut_callbacks);
    Py_VISIT(fut->fut_result);
    Py_VISIT(fut->fut_exception);
    Py_VISIT(fut->fut_source_tb);
    Py_VISIT(fut->dict);
    return 0;
}

/*[clinic input]
_asyncio.Future.result

Return the result this future represents.

If the future has been cancelled, raises CancelledError.  If the
future's result isn't yet available, raises InvalidStateError.  If
the future is done and has an exception set, this exception is raised.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_result_impl(FutureObj *self)
/*[clinic end generated code: output=f35f940936a4b1e5 input=49ecf9cf5ec50dc5]*/
{
    PyObject *result;
    int res = future_get_result(self, &result);

    if (res == -1) {
        return NULL;
    }

    if (res == 0) {
        return result;
    }

    assert(res == 1);

    PyErr_SetObject(PyExceptionInstance_Class(result), result);
    Py_DECREF(result);
    return NULL;
}

/*[clinic input]
_asyncio.Future.exception

Return the exception that was set on this future.

The exception (or None if no exception was set) is returned only if
the future is done.  If the future has been cancelled, raises
CancelledError.  If the future isn't done yet, raises
InvalidStateError.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_exception_impl(FutureObj *self)
/*[clinic end generated code: output=88b20d4f855e0710 input=733547a70c841c68]*/
{
    if (self->fut_state == STATE_CANCELLED) {
        PyErr_SetNone(asyncio_CancelledError);
        return NULL;
    }

    if (self->fut_state != STATE_FINISHED) {
        PyErr_SetString(asyncio_InvalidStateError, "Exception is not set.");
        return NULL;
    }

    if (self->fut_exception != NULL) {
        self->fut_log_tb = 0;
        Py_INCREF(self->fut_exception);
        return self->fut_exception;
    }

    Py_RETURN_NONE;
}

/*[clinic input]
_asyncio.Future.set_result

    res: object
    /

Mark the future done and set its result.

If the future is already done when this method is called, raises
InvalidStateError.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_set_result(FutureObj *self, PyObject *res)
/*[clinic end generated code: output=a620abfc2796bfb6 input=5b9dc180f1baa56d]*/
{
    return future_set_result(self, res);
}

/*[clinic input]
_asyncio.Future.set_exception

    exception: object
    /

Mark the future done and set an exception.

If the future is already done when this method is called, raises
InvalidStateError.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_set_exception(FutureObj *self, PyObject *exception)
/*[clinic end generated code: output=f1c1b0cd321be360 input=e45b7d7aa71cc66d]*/
{
    return future_set_exception(self, exception);
}

/*[clinic input]
_asyncio.Future.add_done_callback

    fn: object
    /

Add a callback to be run when the future becomes done.

The callback is called with a single argument - the future object. If
the future is already done when this is called, the callback is
scheduled with call_soon.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_add_done_callback(FutureObj *self, PyObject *fn)
/*[clinic end generated code: output=819e09629b2ec2b5 input=8f818b39990b027d]*/
{
    return future_add_done_callback(self, fn);
}

/*[clinic input]
_asyncio.Future.remove_done_callback

    fn: object
    /

Remove all instances of a callback from the "call when done" list.

Returns the number of callbacks removed.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_remove_done_callback(FutureObj *self, PyObject *fn)
/*[clinic end generated code: output=5ab1fb52b24ef31f input=0a43280a149d505b]*/
{
    PyObject *newlist;
    Py_ssize_t len, i, j=0;

    if (self->fut_callbacks == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "uninitialized Future object");
        return NULL;
    }

    len = PyList_GET_SIZE(self->fut_callbacks);
    if (len == 0) {
        return PyLong_FromSsize_t(0);
    }

    newlist = PyList_New(len);
    if (newlist == NULL) {
        return NULL;
    }

    for (i = 0; i < PyList_GET_SIZE(self->fut_callbacks); i++) {
        int ret;
        PyObject *item = PyList_GET_ITEM(self->fut_callbacks, i);
        Py_INCREF(item);
        ret = PyObject_RichCompareBool(fn, item, Py_EQ);
        if (ret == 0) {
            if (j < len) {
                PyList_SET_ITEM(newlist, j, item);
                j++;
                continue;
            }
            ret = PyList_Append(newlist, item);
        }
        Py_DECREF(item);
        if (ret < 0) {
            goto fail;
        }
    }

    if (j < len) {
        Py_SIZE(newlist) = j;
    }
    j = PyList_GET_SIZE(newlist);
    len = PyList_GET_SIZE(self->fut_callbacks);
    if (j != len) {
        if (PyList_SetSlice(self->fut_callbacks, 0, len, newlist) < 0) {
            goto fail;
        }
    }
    Py_DECREF(newlist);
    return PyLong_FromSsize_t(len - j);

fail:
    Py_DECREF(newlist);
    return NULL;
}

/*[clinic input]
_asyncio.Future.cancel

Cancel the future and schedule callbacks.

If the future is already done or cancelled, return False.  Otherwise,
change the future's state to cancelled, schedule the callbacks and
return True.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_cancel_impl(FutureObj *self)
/*[clinic end generated code: output=e45b932ba8bd68a1 input=515709a127995109]*/
{
    return future_cancel(self);
}

/*[clinic input]
_asyncio.Future.cancelled

Return True if the future was cancelled.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_cancelled_impl(FutureObj *self)
/*[clinic end generated code: output=145197ced586357d input=943ab8b7b7b17e45]*/
{
    if (self->fut_state == STATE_CANCELLED) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

/*[clinic input]
_asyncio.Future.done

Return True if the future is done.

Done means either that a result / exception are available, or that the
future was cancelled.
[clinic start generated code]*/

static PyObject *
_asyncio_Future_done_impl(FutureObj *self)
/*[clinic end generated code: output=244c5ac351145096 input=28d7b23fdb65d2ac]*/
{
    if (self->fut_state == STATE_PENDING) {
        Py_RETURN_FALSE;
    }
    else {
        Py_RETURN_TRUE;
    }
}

static PyObject *
FutureObj_get_blocking(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_blocking) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static int
FutureObj_set_blocking(FutureObj *fut, PyObject *val, void *Py_UNUSED(ignored))
{
    int is_true = PyObject_IsTrue(val);
    if (is_true < 0) {
        return -1;
    }
    fut->fut_blocking = is_true;
    return 0;
}

static PyObject *
FutureObj_get_log_traceback(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_log_tb) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static int
FutureObj_set_log_traceback(FutureObj *fut, PyObject *val, void *Py_UNUSED(ignored))
{
    int is_true = PyObject_IsTrue(val);
    if (is_true < 0) {
        return -1;
    }
    fut->fut_log_tb = is_true;
    return 0;
}

static PyObject *
FutureObj_get_loop(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_loop == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(fut->fut_loop);
    return fut->fut_loop;
}

static PyObject *
FutureObj_get_callbacks(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_callbacks == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(fut->fut_callbacks);
    return fut->fut_callbacks;
}

static PyObject *
FutureObj_get_result(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_result == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(fut->fut_result);
    return fut->fut_result;
}

static PyObject *
FutureObj_get_exception(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_exception == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(fut->fut_exception);
    return fut->fut_exception;
}

static PyObject *
FutureObj_get_source_traceback(FutureObj *fut, void *Py_UNUSED(ignored))
{
    if (fut->fut_source_tb == NULL) {
        Py_RETURN_NONE;
    }
    Py_INCREF(fut->fut_source_tb);
    return fut->fut_source_tb;
}

static PyObject *
FutureObj_get_state(FutureObj *fut, void *Py_UNUSED(ignored))
{
    _Py_IDENTIFIER(PENDING);
    _Py_IDENTIFIER(CANCELLED);
    _Py_IDENTIFIER(FINISHED);
    PyObject *ret = NULL;

    switch (fut->fut_state) {
    case STATE_PENDING:
        ret = _PyUnicode_FromId(&PyId_PENDING);
        break;
    case STATE_CANCELLED:
        ret = _PyUnicode_FromId(&PyId_CANCELLED);
        break;
    case STATE_FINISHED:
        ret = _PyUnicode_FromId(&PyId_FINISHED);
        break;
    default:
        assert (0);
    }
    Py_XINCREF(ret);
    return ret;
}

/*[clinic input]
_asyncio.Future._repr_info
[clinic start generated code]*/

static PyObject *
_asyncio_Future__repr_info_impl(FutureObj *self)
/*[clinic end generated code: output=fa69e901bd176cfb input=f21504d8e2ae1ca2]*/
{
    return PyObject_CallFunctionObjArgs(
        asyncio_future_repr_info_func, self, NULL);
}

/*[clinic input]
_asyncio.Future._schedule_callbacks
[clinic start generated code]*/

static PyObject *
_asyncio_Future__schedule_callbacks_impl(FutureObj *self)
/*[clinic end generated code: output=5e8958d89ea1c5dc input=4f5f295f263f4a88]*/
{
    int ret = future_schedule_callbacks(self);
    if (ret == -1) {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *
FutureObj_repr(FutureObj *fut)
{
    _Py_IDENTIFIER(_repr_info);

    PyObject *rinfo = _PyObject_CallMethodIdObjArgs((PyObject*)fut,
                                                    &PyId__repr_info,
                                                    NULL);
    if (rinfo == NULL) {
        return NULL;
    }

    PyObject *rinfo_s = PyUnicode_Join(NULL, rinfo);
    Py_DECREF(rinfo);
    if (rinfo_s == NULL) {
        return NULL;
    }

    PyObject *rstr = NULL;
    PyObject *type_name = PyObject_GetAttrString((PyObject*)Py_TYPE(fut),
                                                 "__name__");
    if (type_name != NULL) {
        rstr = PyUnicode_FromFormat("<%S %U>", type_name, rinfo_s);
        Py_DECREF(type_name);
    }
    Py_DECREF(rinfo_s);
    return rstr;
}

static void
FutureObj_finalize(FutureObj *fut)
{
    _Py_IDENTIFIER(call_exception_handler);
    _Py_IDENTIFIER(message);
    _Py_IDENTIFIER(exception);
    _Py_IDENTIFIER(future);
    _Py_IDENTIFIER(source_traceback);

    PyObject *error_type, *error_value, *error_traceback;
    PyObject *context;
    PyObject *type_name;
    PyObject *message = NULL;
    PyObject *func;

    if (!fut->fut_log_tb) {
        return;
    }
    assert(fut->fut_exception != NULL);
    fut->fut_log_tb = 0;

    /* Save the current exception, if any. */
    PyErr_Fetch(&error_type, &error_value, &error_traceback);

    context = PyDict_New();
    if (context == NULL) {
        goto finally;
    }

    type_name = PyObject_GetAttrString((PyObject*)Py_TYPE(fut), "__name__");
    if (type_name == NULL) {
        goto finally;
    }

    message = PyUnicode_FromFormat(
        "%S exception was never retrieved", type_name);
    Py_DECREF(type_name);
    if (message == NULL) {
        goto finally;
    }

    if (_PyDict_SetItemId(context, &PyId_message, message) < 0 ||
        _PyDict_SetItemId(context, &PyId_exception, fut->fut_exception) < 0 ||
        _PyDict_SetItemId(context, &PyId_future, (PyObject*)fut) < 0) {
        goto finally;
    }
    if (fut->fut_source_tb != NULL) {
        if (_PyDict_SetItemId(context, &PyId_source_traceback,
                              fut->fut_source_tb) < 0) {
            goto finally;
        }
    }

    func = _PyObject_GetAttrId(fut->fut_loop, &PyId_call_exception_handler);
    if (func != NULL) {
        PyObject *res = _PyObject_CallArg1(func, context);
        if (res == NULL) {
            PyErr_WriteUnraisable(func);
        }
        else {
            Py_DECREF(res);
        }
        Py_DECREF(func);
    }

finally:
    Py_XDECREF(context);
    Py_XDECREF(message);

    /* Restore the saved exception. */
    PyErr_Restore(error_type, error_value, error_traceback);
}


static PyAsyncMethods FutureType_as_async = {
    (unaryfunc)future_new_iter,         /* am_await */
    0,                                  /* am_aiter */
    0                                   /* am_anext */
};

static PyMethodDef FutureType_methods[] = {
    _ASYNCIO_FUTURE_RESULT_METHODDEF
    _ASYNCIO_FUTURE_EXCEPTION_METHODDEF
    _ASYNCIO_FUTURE_SET_RESULT_METHODDEF
    _ASYNCIO_FUTURE_SET_EXCEPTION_METHODDEF
    _ASYNCIO_FUTURE_ADD_DONE_CALLBACK_METHODDEF
    _ASYNCIO_FUTURE_REMOVE_DONE_CALLBACK_METHODDEF
    _ASYNCIO_FUTURE_CANCEL_METHODDEF
    _ASYNCIO_FUTURE_CANCELLED_METHODDEF
    _ASYNCIO_FUTURE_DONE_METHODDEF
    _ASYNCIO_FUTURE__REPR_INFO_METHODDEF
    _ASYNCIO_FUTURE__SCHEDULE_CALLBACKS_METHODDEF
    {NULL, NULL}        /* Sentinel */
};

#define FUTURE_COMMON_GETSETLIST                                              \
    {"_state", (getter)FutureObj_get_state, NULL, NULL},                      \
    {"_asyncio_future_blocking", (getter)FutureObj_get_blocking,              \
                                 (setter)FutureObj_set_blocking, NULL},       \
    {"_loop", (getter)FutureObj_get_loop, NULL, NULL},                        \
    {"_callbacks", (getter)FutureObj_get_callbacks, NULL, NULL},              \
    {"_result", (getter)FutureObj_get_result, NULL, NULL},                    \
    {"_exception", (getter)FutureObj_get_exception, NULL, NULL},              \
    {"_log_traceback", (getter)FutureObj_get_log_traceback,                   \
                       (setter)FutureObj_set_log_traceback, NULL},            \
    {"_source_traceback", (getter)FutureObj_get_source_traceback, NULL, NULL},

static PyGetSetDef FutureType_getsetlist[] = {
    FUTURE_COMMON_GETSETLIST
    {NULL} /* Sentinel */
};

static void FutureObj_dealloc(PyObject *self);

static PyTypeObject FutureType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_asyncio.Future",
    sizeof(FutureObj),                       /* tp_basicsize */
    .tp_dealloc = FutureObj_dealloc,
    .tp_as_async = &FutureType_as_async,
    .tp_repr = (reprfunc)FutureObj_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE
        | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = _asyncio_Future___init____doc__,
    .tp_traverse = (traverseproc)FutureObj_traverse,
    .tp_clear = (inquiry)FutureObj_clear,
    .tp_weaklistoffset = offsetof(FutureObj, fut_weakreflist),
    .tp_iter = (getiterfunc)future_new_iter,
    .tp_methods = FutureType_methods,
    .tp_getset = FutureType_getsetlist,
    .tp_dictoffset = offsetof(FutureObj, dict),
    .tp_init = (initproc)_asyncio_Future___init__,
    .tp_new = PyType_GenericNew,
    .tp_finalize = (destructor)FutureObj_finalize,
};

#define Future_CheckExact(obj) (Py_TYPE(obj) == &FutureType)

static inline int
future_call_schedule_callbacks(FutureObj *fut)
{
    if (Future_CheckExact(fut)) {
        return future_schedule_callbacks(fut);
    }
    else {
        /* `fut` is a subclass of Future */
        PyObject *ret = _PyObject_CallMethodId(
            (PyObject*)fut, &PyId__schedule_callbacks, NULL);
        if (ret == NULL) {
            return -1;
        }

        Py_DECREF(ret);
        return 0;
    }
}

static void
FutureObj_dealloc(PyObject *self)
{
    FutureObj *fut = (FutureObj *)self;

    if (Future_CheckExact(fut)) {
        /* When fut is subclass of Future, finalizer is called from
         * subtype_dealloc.
         */
        if (PyObject_CallFinalizerFromDealloc(self) < 0) {
            // resurrected.
            return;
        }
    }

    PyObject_GC_UnTrack(self);

    if (fut->fut_weakreflist != NULL) {
        PyObject_ClearWeakRefs(self);
    }

    (void)FutureObj_clear(fut);
    Py_TYPE(fut)->tp_free(fut);
}


/*********************** Future Iterator **************************/

typedef struct {
    PyObject_HEAD
    FutureObj *future;
} futureiterobject;

static void
FutureIter_dealloc(futureiterobject *it)
{
    PyObject_GC_UnTrack(it);
    Py_XDECREF(it->future);
    PyObject_GC_Del(it);
}

static PyObject *
FutureIter_iternext(futureiterobject *it)
{
    PyObject *res;
    FutureObj *fut = it->future;

    if (fut == NULL) {
        return NULL;
    }

    if (fut->fut_state == STATE_PENDING) {
        if (!fut->fut_blocking) {
            fut->fut_blocking = 1;
            Py_INCREF(fut);
            return (PyObject *)fut;
        }
        PyErr_SetString(PyExc_AssertionError,
                        "yield from wasn't used with future");
        return NULL;
    }

    it->future = NULL;
    res = _asyncio_Future_result_impl(fut);
    if (res != NULL) {
        /* The result of the Future is not an exception. */
        (void)_PyGen_SetStopIterationValue(res);
        Py_DECREF(res);
    }

    Py_DECREF(fut);
    return NULL;
}

static PyObject *
FutureIter_send(futureiterobject *self, PyObject *unused)
{
    /* Future.__iter__ doesn't care about values that are pushed to the
     * generator, it just returns "self.result().
     */
    return FutureIter_iternext(self);
}

static PyObject *
FutureIter_throw(futureiterobject *self, PyObject *args)
{
    PyObject *type, *val = NULL, *tb = NULL;
    if (!PyArg_ParseTuple(args, "O|OO", &type, &val, &tb))
        return NULL;

    if (val == Py_None) {
        val = NULL;
    }
    if (tb == Py_None) {
        tb = NULL;
    } else if (tb != NULL && !PyTraceBack_Check(tb)) {
        PyErr_SetString(PyExc_TypeError, "throw() third argument must be a traceback");
        return NULL;
    }

    Py_INCREF(type);
    Py_XINCREF(val);
    Py_XINCREF(tb);

    if (PyExceptionClass_Check(type)) {
        PyErr_NormalizeException(&type, &val, &tb);
        /* No need to call PyException_SetTraceback since we'll be calling
           PyErr_Restore for `type`, `val`, and `tb`. */
    } else if (PyExceptionInstance_Check(type)) {
        if (val) {
            PyErr_SetString(PyExc_TypeError,
                            "instance exception may not have a separate value");
            goto fail;
        }
        val = type;
        type = PyExceptionInstance_Class(type);
        Py_INCREF(type);
        if (tb == NULL)
            tb = PyException_GetTraceback(val);
    } else {
        PyErr_SetString(PyExc_TypeError,
                        "exceptions must be classes deriving BaseException or "
                        "instances of such a class");
        goto fail;
    }

    Py_CLEAR(self->future);

    PyErr_Restore(type, val, tb);

    return NULL;

  fail:
    Py_DECREF(type);
    Py_XDECREF(val);
    Py_XDECREF(tb);
    return NULL;
}

static PyObject *
FutureIter_close(futureiterobject *self, PyObject *arg)
{
    Py_CLEAR(self->future);
    Py_RETURN_NONE;
}

static int
FutureIter_traverse(futureiterobject *it, visitproc visit, void *arg)
{
    Py_VISIT(it->future);
    return 0;
}

static PyMethodDef FutureIter_methods[] = {
    {"send",  (PyCFunction)FutureIter_send, METH_O, NULL},
    {"throw", (PyCFunction)FutureIter_throw, METH_VARARGS, NULL},
    {"close", (PyCFunction)FutureIter_close, METH_NOARGS, NULL},
    {NULL, NULL}        /* Sentinel */
};

static PyTypeObject FutureIterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_asyncio.FutureIter",
    .tp_basicsize = sizeof(futureiterobject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)FutureIter_dealloc,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse = (traverseproc)FutureIter_traverse,
    .tp_iter = PyObject_SelfIter,
    .tp_iternext = (iternextfunc)FutureIter_iternext,
    .tp_methods = FutureIter_methods,
};

static PyObject *
future_new_iter(PyObject *fut)
{
    futureiterobject *it;

    if (!PyObject_TypeCheck(fut, &FutureType)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    it = PyObject_GC_New(futureiterobject, &FutureIterType);
    if (it == NULL) {
        return NULL;
    }
    Py_INCREF(fut);
    it->future = (FutureObj*)fut;
    PyObject_GC_Track(it);
    return (PyObject*)it;
}


/*********************** Task **************************/


/*[clinic input]
class _asyncio.Task "TaskObj *" "&Task_Type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=719dcef0fcc03b37]*/

static int task_call_step_soon(TaskObj *, PyObject *);
static inline PyObject * task_call_wakeup(TaskObj *, PyObject *);
static inline PyObject * task_call_step(TaskObj *, PyObject *);
static PyObject * task_wakeup(TaskObj *, PyObject *);
static PyObject * task_step(TaskObj *, PyObject *);

/* ----- Task._step wrapper */

static int
TaskStepMethWrapper_clear(TaskStepMethWrapper *o)
{
    Py_CLEAR(o->sw_task);
    Py_CLEAR(o->sw_arg);
    return 0;
}

static void
TaskStepMethWrapper_dealloc(TaskStepMethWrapper *o)
{
    PyObject_GC_UnTrack(o);
    (void)TaskStepMethWrapper_clear(o);
    Py_TYPE(o)->tp_free(o);
}

static PyObject *
TaskStepMethWrapper_call(TaskStepMethWrapper *o,
                         PyObject *args, PyObject *kwds)
{
    if (kwds != NULL && PyDict_Size(kwds) != 0) {
        PyErr_SetString(PyExc_TypeError, "function takes no keyword arguments");
        return NULL;
    }
    if (args != NULL && PyTuple_GET_SIZE(args) != 0) {
        PyErr_SetString(PyExc_TypeError, "function takes no positional arguments");
        return NULL;
    }
    return task_call_step(o->sw_task, o->sw_arg);
}

static int
TaskStepMethWrapper_traverse(TaskStepMethWrapper *o,
                             visitproc visit, void *arg)
{
    Py_VISIT(o->sw_task);
    Py_VISIT(o->sw_arg);
    return 0;
}

static PyObject *
TaskStepMethWrapper_get___self__(TaskStepMethWrapper *o, void *Py_UNUSED(ignored))
{
    if (o->sw_task) {
        Py_INCREF(o->sw_task);
        return (PyObject*)o->sw_task;
    }
    Py_RETURN_NONE;
}

static PyGetSetDef TaskStepMethWrapper_getsetlist[] = {
    {"__self__", (getter)TaskStepMethWrapper_get___self__, NULL, NULL},
    {NULL} /* Sentinel */
};

static PyTypeObject TaskStepMethWrapper_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "TaskStepMethWrapper",
    .tp_basicsize = sizeof(TaskStepMethWrapper),
    .tp_itemsize = 0,
    .tp_getset = TaskStepMethWrapper_getsetlist,
    .tp_dealloc = (destructor)TaskStepMethWrapper_dealloc,
    .tp_call = (ternaryfunc)TaskStepMethWrapper_call,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse = (traverseproc)TaskStepMethWrapper_traverse,
    .tp_clear = (inquiry)TaskStepMethWrapper_clear,
};

static PyObject *
TaskStepMethWrapper_new(TaskObj *task, PyObject *arg)
{
    TaskStepMethWrapper *o;
    o = PyObject_GC_New(TaskStepMethWrapper, &TaskStepMethWrapper_Type);
    if (o == NULL) {
        return NULL;
    }

    Py_INCREF(task);
    o->sw_task = task;

    Py_XINCREF(arg);
    o->sw_arg = arg;

    PyObject_GC_Track(o);
    return (PyObject*) o;
}

/* ----- Task._wakeup wrapper */

static PyObject *
TaskWakeupMethWrapper_call(TaskWakeupMethWrapper *o,
                           PyObject *args, PyObject *kwds)
{
    PyObject *fut;

    if (kwds != NULL && PyDict_Size(kwds) != 0) {
        PyErr_SetString(PyExc_TypeError, "function takes no keyword arguments");
        return NULL;
    }
    if (!PyArg_ParseTuple(args, "O", &fut)) {
        return NULL;
    }

    return task_call_wakeup(o->ww_task, fut);
}

static int
TaskWakeupMethWrapper_clear(TaskWakeupMethWrapper *o)
{
    Py_CLEAR(o->ww_task);
    return 0;
}

static int
TaskWakeupMethWrapper_traverse(TaskWakeupMethWrapper *o,
                               visitproc visit, void *arg)
{
    Py_VISIT(o->ww_task);
    return 0;
}

static void
TaskWakeupMethWrapper_dealloc(TaskWakeupMethWrapper *o)
{
    PyObject_GC_UnTrack(o);
    (void)TaskWakeupMethWrapper_clear(o);
    Py_TYPE(o)->tp_free(o);
}

static PyTypeObject TaskWakeupMethWrapper_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "TaskWakeupMethWrapper",
    .tp_basicsize = sizeof(TaskWakeupMethWrapper),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)TaskWakeupMethWrapper_dealloc,
    .tp_call = (ternaryfunc)TaskWakeupMethWrapper_call,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse = (traverseproc)TaskWakeupMethWrapper_traverse,
    .tp_clear = (inquiry)TaskWakeupMethWrapper_clear,
};

static PyObject *
TaskWakeupMethWrapper_new(TaskObj *task)
{
    TaskWakeupMethWrapper *o;
    o = PyObject_GC_New(TaskWakeupMethWrapper, &TaskWakeupMethWrapper_Type);
    if (o == NULL) {
        return NULL;
    }

    Py_INCREF(task);
    o->ww_task = task;

    PyObject_GC_Track(o);
    return (PyObject*) o;
}

/* ----- Task */

/*[clinic input]
_asyncio.Task.__init__

    coro: object
    *
    loop: object = None

A coroutine wrapped in a Future.
[clinic start generated code]*/

static int
_asyncio_Task___init___impl(TaskObj *self, PyObject *coro, PyObject *loop)
/*[clinic end generated code: output=9f24774c2287fc2f input=8d132974b049593e]*/
{
    PyObject *res;
    _Py_IDENTIFIER(add);

    if (future_init((FutureObj*)self, loop)) {
        return -1;
    }

    Py_CLEAR(self->task_fut_waiter);
    self->task_must_cancel = 0;
    self->task_log_destroy_pending = 1;

    Py_INCREF(coro);
    Py_XSETREF(self->task_coro, coro);

    if (task_call_step_soon(self, NULL)) {
        return -1;
    }

    res = _PyObject_CallMethodIdObjArgs(all_tasks, &PyId_add, self, NULL);
    if (res == NULL) {
        return -1;
    }
    Py_DECREF(res);

    return 0;
}

static int
TaskObj_clear(TaskObj *task)
{
    (void)FutureObj_clear((FutureObj*) task);
    Py_CLEAR(task->task_coro);
    Py_CLEAR(task->task_fut_waiter);
    return 0;
}

static int
TaskObj_traverse(TaskObj *task, visitproc visit, void *arg)
{
    Py_VISIT(task->task_coro);
    Py_VISIT(task->task_fut_waiter);
    (void)FutureObj_traverse((FutureObj*) task, visit, arg);
    return 0;
}

static PyObject *
TaskObj_get_log_destroy_pending(TaskObj *task, void *Py_UNUSED(ignored))
{
    if (task->task_log_destroy_pending) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static int
TaskObj_set_log_destroy_pending(TaskObj *task, PyObject *val, void *Py_UNUSED(ignored))
{
    int is_true = PyObject_IsTrue(val);
    if (is_true < 0) {
        return -1;
    }
    task->task_log_destroy_pending = is_true;
    return 0;
}

static PyObject *
TaskObj_get_must_cancel(TaskObj *task, void *Py_UNUSED(ignored))
{
    if (task->task_must_cancel) {
        Py_RETURN_TRUE;
    }
    else {
        Py_RETURN_FALSE;
    }
}

static PyObject *
TaskObj_get_coro(TaskObj *task, void *Py_UNUSED(ignored))
{
    if (task->task_coro) {
        Py_INCREF(task->task_coro);
        return task->task_coro;
    }

    Py_RETURN_NONE;
}

static PyObject *
TaskObj_get_fut_waiter(TaskObj *task, void *Py_UNUSED(ignored))
{
    if (task->task_fut_waiter) {
        Py_INCREF(task->task_fut_waiter);
        return task->task_fut_waiter;
    }

    Py_RETURN_NONE;
}

/*[clinic input]
@classmethod
_asyncio.Task.current_task

    loop: object = None

Return the currently running task in an event loop or None.

By default the current task for the current event loop is returned.

None is returned when called not in the context of a Task.
[clinic start generated code]*/

static PyObject *
_asyncio_Task_current_task_impl(PyTypeObject *type, PyObject *loop)
/*[clinic end generated code: output=99fbe7332c516e03 input=cd14770c5b79c7eb]*/
{
    PyObject *res;

    if (loop == Py_None) {
        loop = _PyObject_CallNoArg(asyncio_get_event_loop);
        if (loop == NULL) {
            return NULL;
        }

        res = PyDict_GetItem(current_tasks, loop);
        Py_DECREF(loop);
    }
    else {
        res = PyDict_GetItem(current_tasks, loop);
    }

    if (res == NULL) {
        Py_RETURN_NONE;
    }
    else {
        Py_INCREF(res);
        return res;
    }
}

static PyObject *
task_all_tasks(PyObject *loop)
{
    PyObject *task;
    PyObject *task_loop;
    PyObject *set;
    PyObject *iter;

    assert(loop != NULL);

    set = PySet_New(NULL);
    if (set == NULL) {
        return NULL;
    }

    iter = PyObject_GetIter(all_tasks);
    if (iter == NULL) {
        goto fail;
    }

    while ((task = PyIter_Next(iter))) {
        task_loop = PyObject_GetAttrString(task, "_loop");
        if (task_loop == NULL) {
            Py_DECREF(task);
            goto fail;
        }
        if (task_loop == loop) {
            if (PySet_Add(set, task) == -1) {
                Py_DECREF(task_loop);
                Py_DECREF(task);
                goto fail;
            }
        }
        Py_DECREF(task_loop);
        Py_DECREF(task);
    }
    if (PyErr_Occurred()) {
        goto fail;
    }
    Py_DECREF(iter);
    return set;

fail:
    Py_DECREF(set);
    Py_XDECREF(iter);
    return NULL;
}

/*[clinic input]
@classmethod
_asyncio.Task.all_tasks

    loop: object = None

Return a set of all tasks for an event loop.

By default all tasks for the current event loop are returned.
[clinic start generated code]*/

static PyObject *
_asyncio_Task_all_tasks_impl(PyTypeObject *type, PyObject *loop)
/*[clinic end generated code: output=11f9b20749ccca5d input=497f80bc9ce726b5]*/
{
    PyObject *res;

    if (loop == Py_None) {
        loop = _PyObject_CallNoArg(asyncio_get_event_loop);
        if (loop == NULL) {
            return NULL;
        }

        res = task_all_tasks(loop);
        Py_DECREF(loop);
    }
    else {
        res = task_all_tasks(loop);
    }

    return res;
}

/*[clinic input]
_asyncio.Task._repr_info
[clinic start generated code]*/

static PyObject *
_asyncio_Task__repr_info_impl(TaskObj *self)
/*[clinic end generated code: output=6a490eb66d5ba34b input=3c6d051ed3ddec8b]*/
{
    return PyObject_CallFunctionObjArgs(
        asyncio_task_repr_info_func, self, NULL);
}

/*[clinic input]
_asyncio.Task.cancel

Request that this task cancel itself.

This arranges for a CancelledError to be thrown into the
wrapped coroutine on the next cycle through the event loop.
The coroutine then has a chance to clean up or even deny
the request using try/except/finally.

Unlike Future.cancel, this does not guarantee that the
task will be cancelled: the exception might be caught and
acted upon, delaying cancellation of the task or preventing
cancellation completely.  The task may also return a value or
raise a different exception.

Immediately after this method is called, Task.cancelled() will
not return True (unless the task was already cancelled).  A
task will be marked as cancelled when the wrapped coroutine
terminates with a CancelledError exception (even if cancel()
was not called).
[clinic start generated code]*/

static PyObject *
_asyncio_Task_cancel_impl(TaskObj *self)
/*[clinic end generated code: output=6bfc0479da9d5757 input=13f9bf496695cb52]*/
{
    self->task_log_tb = 0;

    if (self->task_state != STATE_PENDING) {
        Py_RETURN_FALSE;
    }

    if (self->task_fut_waiter) {
        PyObject *res;
        int is_true;

        res = _PyObject_CallMethodId(
            self->task_fut_waiter, &PyId_cancel, NULL);
        if (res == NULL) {
            return NULL;
        }

        is_true = PyObject_IsTrue(res);
        Py_DECREF(res);
        if (is_true < 0) {
            return NULL;
        }

        if (is_true) {
            Py_RETURN_TRUE;
        }
    }

    self->task_must_cancel = 1;
    Py_RETURN_TRUE;
}

/*[clinic input]
_asyncio.Task.get_stack

    *
    limit: object = None

Return the list of stack frames for this task's coroutine.

If the coroutine is not done, this returns the stack where it is
suspended.  If the coroutine has completed successfully or was
cancelled, this returns an empty list.  If the coroutine was
terminated by an exception, this returns the list of traceback
frames.

The frames are always ordered from oldest to newest.

The optional limit gives the maximum number of frames to
return; by default all available frames are returned.  Its
meaning differs depending on whether a stack or a traceback is
returned: the newest frames of a stack are returned, but the
oldest frames of a traceback are returned.  (This matches the
behavior of the traceback module.)

For reasons beyond our control, only one stack frame is
returned for a suspended coroutine.
[clinic start generated code]*/

static PyObject *
_asyncio_Task_get_stack_impl(TaskObj *self, PyObject *limit)
/*[clinic end generated code: output=c9aeeeebd1e18118 input=05b323d42b809b90]*/
{
    return PyObject_CallFunctionObjArgs(
        asyncio_task_get_stack_func, self, limit, NULL);
}

/*[clinic input]
_asyncio.Task.print_stack

    *
    limit: object = None
    file: object = None

Print the stack or traceback for this task's coroutine.

This produces output similar to that of the traceback module,
for the frames retrieved by get_stack().  The limit argument
is passed to get_stack().  The file argument is an I/O stream
to which the output is written; by default output is written
to sys.stderr.
[clinic start generated code]*/

static PyObject *
_asyncio_Task_print_stack_impl(TaskObj *self, PyObject *limit,
                               PyObject *file)
/*[clinic end generated code: output=7339e10314cd3f4d input=1a0352913b7fcd92]*/
{
    return PyObject_CallFunctionObjArgs(
        asyncio_task_print_stack_func, self, limit, file, NULL);
}

/*[clinic input]
_asyncio.Task._step

    exc: object = None
[clinic start generated code]*/

static PyObject *
_asyncio_Task__step_impl(TaskObj *self, PyObject *exc)
/*[clinic end generated code: output=7ed23f0cefd5ae42 input=1e19a985ace87ca4]*/
{
    return task_step(self, exc == Py_None ? NULL : exc);
}

/*[clinic input]
_asyncio.Task._wakeup

    fut: object
[clinic start generated code]*/

static PyObject *
_asyncio_Task__wakeup_impl(TaskObj *self, PyObject *fut)
/*[clinic end generated code: output=75cb341c760fd071 input=6a0616406f829a7b]*/
{
    return task_wakeup(self, fut);
}

static void
TaskObj_finalize(TaskObj *task)
{
    _Py_IDENTIFIER(call_exception_handler);
    _Py_IDENTIFIER(task);
    _Py_IDENTIFIER(message);
    _Py_IDENTIFIER(source_traceback);

    PyObject *context;
    PyObject *message = NULL;
    PyObject *func;
    PyObject *error_type, *error_value, *error_traceback;

    if (task->task_state != STATE_PENDING || !task->task_log_destroy_pending) {
        goto done;
    }

    /* Save the current exception, if any. */
    PyErr_Fetch(&error_type, &error_value, &error_traceback);

    context = PyDict_New();
    if (context == NULL) {
        goto finally;
    }

    message = PyUnicode_FromString("Task was destroyed but it is pending!");
    if (message == NULL) {
        goto finally;
    }

    if (_PyDict_SetItemId(context, &PyId_message, message) < 0 ||
        _PyDict_SetItemId(context, &PyId_task, (PyObject*)task) < 0)
    {
        goto finally;
    }

    if (task->task_source_tb != NULL) {
        if (_PyDict_SetItemId(context, &PyId_source_traceback,
                              task->task_source_tb) < 0)
        {
            goto finally;
        }
    }

    func = _PyObject_GetAttrId(task->task_loop, &PyId_call_exception_handler);
    if (func != NULL) {
        PyObject *res = _PyObject_CallArg1(func, context);
        if (res == NULL) {
            PyErr_WriteUnraisable(func);
        }
        else {
            Py_DECREF(res);
        }
        Py_DECREF(func);
    }

finally:
    Py_XDECREF(context);
    Py_XDECREF(message);

    /* Restore the saved exception. */
    PyErr_Restore(error_type, error_value, error_traceback);

done:
    FutureObj_finalize((FutureObj*)task);
}

static void TaskObj_dealloc(PyObject *);  /* Needs Task_CheckExact */

static PyMethodDef TaskType_methods[] = {
    _ASYNCIO_FUTURE_RESULT_METHODDEF
    _ASYNCIO_FUTURE_EXCEPTION_METHODDEF
    _ASYNCIO_FUTURE_SET_RESULT_METHODDEF
    _ASYNCIO_FUTURE_SET_EXCEPTION_METHODDEF
    _ASYNCIO_FUTURE_ADD_DONE_CALLBACK_METHODDEF
    _ASYNCIO_FUTURE_REMOVE_DONE_CALLBACK_METHODDEF
    _ASYNCIO_FUTURE_CANCELLED_METHODDEF
    _ASYNCIO_FUTURE_DONE_METHODDEF
    _ASYNCIO_TASK_CURRENT_TASK_METHODDEF
    _ASYNCIO_TASK_ALL_TASKS_METHODDEF
    _ASYNCIO_TASK_CANCEL_METHODDEF
    _ASYNCIO_TASK_GET_STACK_METHODDEF
    _ASYNCIO_TASK_PRINT_STACK_METHODDEF
    _ASYNCIO_TASK__WAKEUP_METHODDEF
    _ASYNCIO_TASK__STEP_METHODDEF
    _ASYNCIO_TASK__REPR_INFO_METHODDEF
    {NULL, NULL}        /* Sentinel */
};

static PyGetSetDef TaskType_getsetlist[] = {
    FUTURE_COMMON_GETSETLIST
    {"_log_destroy_pending", (getter)TaskObj_get_log_destroy_pending,
                             (setter)TaskObj_set_log_destroy_pending, NULL},
    {"_must_cancel", (getter)TaskObj_get_must_cancel, NULL, NULL},
    {"_coro", (getter)TaskObj_get_coro, NULL, NULL},
    {"_fut_waiter", (getter)TaskObj_get_fut_waiter, NULL, NULL},
    {NULL} /* Sentinel */
};

static PyTypeObject TaskType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_asyncio.Task",
    sizeof(TaskObj),                       /* tp_basicsize */
    .tp_base = &FutureType,
    .tp_dealloc = TaskObj_dealloc,
    .tp_as_async = &FutureType_as_async,
    .tp_repr = (reprfunc)FutureObj_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE
        | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = _asyncio_Task___init____doc__,
    .tp_traverse = (traverseproc)TaskObj_traverse,
    .tp_clear = (inquiry)TaskObj_clear,
    .tp_weaklistoffset = offsetof(TaskObj, task_weakreflist),
    .tp_iter = (getiterfunc)future_new_iter,
    .tp_methods = TaskType_methods,
    .tp_getset = TaskType_getsetlist,
    .tp_dictoffset = offsetof(TaskObj, dict),
    .tp_init = (initproc)_asyncio_Task___init__,
    .tp_new = PyType_GenericNew,
    .tp_finalize = (destructor)TaskObj_finalize,
};

#define Task_CheckExact(obj) (Py_TYPE(obj) == &TaskType)

static void
TaskObj_dealloc(PyObject *self)
{
    TaskObj *task = (TaskObj *)self;

    if (Task_CheckExact(self)) {
        /* When fut is subclass of Task, finalizer is called from
         * subtype_dealloc.
         */
        if (PyObject_CallFinalizerFromDealloc(self) < 0) {
            // resurrected.
            return;
        }
    }

    PyObject_GC_UnTrack(self);

    if (task->task_weakreflist != NULL) {
        PyObject_ClearWeakRefs(self);
    }

    (void)TaskObj_clear(task);
    Py_TYPE(task)->tp_free(task);
}

static inline PyObject *
task_call_wakeup(TaskObj *task, PyObject *fut)
{
    if (Task_CheckExact(task)) {
        return task_wakeup(task, fut);
    }
    else {
        /* `task` is a subclass of Task */
        return _PyObject_CallMethodIdObjArgs((PyObject*)task, &PyId__wakeup,
                                             fut, NULL);
    }
}

static inline PyObject *
task_call_step(TaskObj *task, PyObject *arg)
{
    if (Task_CheckExact(task)) {
        return task_step(task, arg);
    }
    else {
        /* `task` is a subclass of Task */
        return _PyObject_CallMethodIdObjArgs((PyObject*)task, &PyId__step,
                                             arg, NULL);
    }
}

static int
task_call_step_soon(TaskObj *task, PyObject *arg)
{
    PyObject *handle;

    PyObject *cb = TaskStepMethWrapper_new(task, arg);
    if (cb == NULL) {
        return -1;
    }

    handle = _PyObject_CallMethodIdObjArgs(task->task_loop, &PyId_call_soon,
                                           cb, NULL);
    Py_DECREF(cb);
    if (handle == NULL) {
        return -1;
    }

    Py_DECREF(handle);
    return 0;
}

static PyObject *
task_set_error_soon(TaskObj *task, PyObject *et, const char *format, ...)
{
    PyObject* msg;

    va_list vargs;
#ifdef HAVE_STDARG_PROTOTYPES
    va_start(vargs, format);
#else
    va_start(vargs);
#endif
    msg = PyUnicode_FromFormatV(format, vargs);
    va_end(vargs);

    if (msg == NULL) {
        return NULL;
    }

    PyObject *e = PyObject_CallFunctionObjArgs(et, msg, NULL);
    Py_DECREF(msg);
    if (e == NULL) {
        return NULL;
    }

    if (task_call_step_soon(task, e) == -1) {
        Py_DECREF(e);
        return NULL;
    }

    Py_DECREF(e);
    Py_RETURN_NONE;
}

static PyObject *
task_step_impl(TaskObj *task, PyObject *exc)
{
    int res;
    int clear_exc = 0;
    PyObject *result = NULL;
    PyObject *coro;
    PyObject *o;

    if (task->task_state != STATE_PENDING) {
        PyErr_Format(PyExc_AssertionError,
                     "_step(): already done: %R %R",
                     task,
                     exc ? exc : Py_None);
        goto fail;
    }

    if (task->task_must_cancel) {
        assert(exc != Py_None);

        if (exc) {
            /* Check if exc is a CancelledError */
            res = PyObject_IsInstance(exc, asyncio_CancelledError);
            if (res == -1) {
                /* An error occurred, abort */
                goto fail;
            }
            if (res == 0) {
                /* exc is not CancelledError; reset it to NULL */
                exc = NULL;
            }
        }

        if (!exc) {
            /* exc was not a CancelledError */
            exc = PyObject_CallFunctionObjArgs(asyncio_CancelledError, NULL);
            if (!exc) {
                goto fail;
            }
            clear_exc = 1;
        }

        task->task_must_cancel = 0;
    }

    Py_CLEAR(task->task_fut_waiter);

    coro = task->task_coro;
    if (coro == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "uninitialized Task object");
        return NULL;
    }

    if (exc == NULL) {
        if (PyGen_CheckExact(coro) || PyCoro_CheckExact(coro)) {
            result = _PyGen_Send((PyGenObject*)coro, Py_None);
        }
        else {
            result = _PyObject_CallMethodIdObjArgs(
                coro, &PyId_send, Py_None, NULL);
        }
    }
    else {
        result = _PyObject_CallMethodIdObjArgs(
            coro, &PyId_throw, exc, NULL);
        if (clear_exc) {
            /* We created 'exc' during this call */
            Py_DECREF(exc);
        }
    }

    if (result == NULL) {
        PyObject *et, *ev, *tb;

        if (_PyGen_FetchStopIterationValue(&o) == 0) {
            /* The error is StopIteration and that means that
               the underlying coroutine has resolved */
            if (task->task_must_cancel) {
                // Task is cancelled right before coro stops.
                Py_DECREF(o);
                task->task_must_cancel = 0;
                et = asyncio_CancelledError;
                Py_INCREF(et);
                ev = NULL;
                tb = NULL;
                goto set_exception;
            }
            PyObject *res = future_set_result((FutureObj*)task, o);
            Py_DECREF(o);
            if (res == NULL) {
                return NULL;
            }
            Py_DECREF(res);
            Py_RETURN_NONE;
        }

        if (PyErr_ExceptionMatches(asyncio_CancelledError)) {
            /* CancelledError */
            PyErr_Clear();
            return future_cancel((FutureObj*)task);
        }

        /* Some other exception; pop it and call Task.set_exception() */
        PyErr_Fetch(&et, &ev, &tb);

set_exception:
        assert(et);
        if (!ev || !PyObject_TypeCheck(ev, (PyTypeObject *) et)) {
            PyErr_NormalizeException(&et, &ev, &tb);
        }
        if (tb != NULL) {
            PyException_SetTraceback(ev, tb);
        }
        o = future_set_exception((FutureObj*)task, ev);
        if (!o) {
            /* An exception in Task.set_exception() */
            Py_DECREF(et);
            Py_XDECREF(tb);
            Py_XDECREF(ev);
            goto fail;
        }
        assert(o == Py_None);
        Py_DECREF(o);

        if (!PyErr_GivenExceptionMatches(et, PyExc_Exception)) {
            /* We've got a BaseException; re-raise it */
            PyErr_Restore(et, ev, tb);
            goto fail;
        }

        Py_DECREF(et);
        Py_XDECREF(tb);
        Py_XDECREF(ev);

        Py_RETURN_NONE;
    }

    if (result == (PyObject*)task) {
        /* We have a task that wants to await on itself */
        goto self_await;
    }

    /* Check if `result` is FutureObj or TaskObj (and not a subclass) */
    if (Future_CheckExact(result) || Task_CheckExact(result)) {
        PyObject *wrapper;
        PyObject *res;
        FutureObj *fut = (FutureObj*)result;

        /* Check if `result` future is attached to a different loop */
        if (fut->fut_loop != task->task_loop) {
            goto different_loop;
        }

        if (fut->fut_blocking) {
            fut->fut_blocking = 0;

            /* result.add_done_callback(task._wakeup) */
            wrapper = TaskWakeupMethWrapper_new(task);
            if (wrapper == NULL) {
                goto fail;
            }
            res = future_add_done_callback((FutureObj*)result, wrapper);
            Py_DECREF(wrapper);
            if (res == NULL) {
                goto fail;
            }
            Py_DECREF(res);

            /* task._fut_waiter = result */
            task->task_fut_waiter = result;  /* no incref is necessary */

            if (task->task_must_cancel) {
                PyObject *r;
                int is_true;
                r = _PyObject_CallMethodId(result, &PyId_cancel, NULL);
                if (r == NULL) {
                    return NULL;
                }
                is_true = PyObject_IsTrue(r);
                Py_DECREF(r);
                if (is_true < 0) {
                    return NULL;
                }
                else if (is_true) {
                    task->task_must_cancel = 0;
                }
            }

            Py_RETURN_NONE;
        }
        else {
            goto yield_insteadof_yf;
        }
    }

    /* Check if `result` is a Future-compatible object */
    o = PyObject_GetAttrString(result, "_asyncio_future_blocking");
    if (o == NULL) {
        if (PyErr_ExceptionMatches(PyExc_AttributeError)) {
            PyErr_Clear();
        }
        else {
            goto fail;
        }
    }
    else {
        if (o == Py_None) {
            Py_DECREF(o);
        }
        else {
            /* `result` is a Future-compatible object */
            PyObject *wrapper;
            PyObject *res;

            int blocking = PyObject_IsTrue(o);
            Py_DECREF(o);
            if (blocking < 0) {
                goto fail;
            }

            /* Check if `result` future is attached to a different loop */
            PyObject *oloop = PyObject_GetAttrString(result, "_loop");
            if (oloop == NULL) {
                goto fail;
            }
            if (oloop != task->task_loop) {
                Py_DECREF(oloop);
                goto different_loop;
            }
            else {
                Py_DECREF(oloop);
            }

            if (blocking) {
                /* result._asyncio_future_blocking = False */
                if (PyObject_SetAttrString(
                        result, "_asyncio_future_blocking", Py_False) == -1) {
                    goto fail;
                }

                /* result.add_done_callback(task._wakeup) */
                wrapper = TaskWakeupMethWrapper_new(task);
                if (wrapper == NULL) {
                    goto fail;
                }
                res = _PyObject_CallMethodIdObjArgs(result,
                                                    &PyId_add_done_callback,
                                                    wrapper, NULL);
                Py_DECREF(wrapper);
                if (res == NULL) {
                    goto fail;
                }
                Py_DECREF(res);

                /* task._fut_waiter = result */
                task->task_fut_waiter = result;  /* no incref is necessary */

                if (task->task_must_cancel) {
                    PyObject *r;
                    int is_true;
                    r = _PyObject_CallMethodId(result, &PyId_cancel, NULL);
                    if (r == NULL) {
                        return NULL;
                    }
                    is_true = PyObject_IsTrue(r);
                    Py_DECREF(r);
                    if (is_true < 0) {
                        return NULL;
                    }
                    else if (is_true) {
                        task->task_must_cancel = 0;
                    }
                }

                Py_RETURN_NONE;
            }
            else {
                goto yield_insteadof_yf;
            }
        }
    }

    /* Check if `result` is None */
    if (result == Py_None) {
        /* Bare yield relinquishes control for one event loop iteration. */
        if (task_call_step_soon(task, NULL)) {
            goto fail;
        }
        return result;
    }

    /* Check if `result` is a generator */
    o = PyObject_CallFunctionObjArgs(inspect_isgenerator, result, NULL);
    if (o == NULL) {
        /* An exception in inspect.isgenerator */
        goto fail;
    }
    res = PyObject_IsTrue(o);
    Py_DECREF(o);
    if (res == -1) {
        /* An exception while checking if 'val' is True */
        goto fail;
    }
    if (res == 1) {
        /* `result` is a generator */
        o = task_set_error_soon(
            task, PyExc_RuntimeError,
            "yield was used instead of yield from for "
            "generator in task %R with %R", task, result);
        Py_DECREF(result);
        return o;
    }

    /* The `result` is none of the above */
    o = task_set_error_soon(
        task, PyExc_RuntimeError, "Task got bad yield: %R", result);
    Py_DECREF(result);
    return o;

self_await:
    o = task_set_error_soon(
        task, PyExc_RuntimeError,
        "Task cannot await on itself: %R", task);
    Py_DECREF(result);
    return o;

yield_insteadof_yf:
    o = task_set_error_soon(
        task, PyExc_RuntimeError,
        "yield was used instead of yield from "
        "in task %R with %R",
        task, result);
    Py_DECREF(result);
    return o;

different_loop:
    o = task_set_error_soon(
        task, PyExc_RuntimeError,
        "Task %R got Future %R attached to a different loop",
        task, result);
    Py_DECREF(result);
    return o;

fail:
    Py_XDECREF(result);
    return NULL;
}

static PyObject *
task_step(TaskObj *task, PyObject *exc)
{
    PyObject *res;
    PyObject *ot;

    if (PyDict_SetItem(current_tasks,
                       task->task_loop, (PyObject*)task) == -1)
    {
        return NULL;
    }

    res = task_step_impl(task, exc);

    if (res == NULL) {
        PyObject *et, *ev, *tb;
        PyErr_Fetch(&et, &ev, &tb);
        ot = _PyDict_Pop(current_tasks, task->task_loop, NULL);
        Py_XDECREF(ot);
        _PyErr_ChainExceptions(et, ev, tb);
        return NULL;
    }
    else {
        ot = _PyDict_Pop(current_tasks, task->task_loop, NULL);
        if (ot == NULL) {
            Py_DECREF(res);
            return NULL;
        }
        else {
            Py_DECREF(ot);
            return res;
        }
    }
}

static PyObject *
task_wakeup(TaskObj *task, PyObject *o)
{
    PyObject *et, *ev, *tb;
    PyObject *result;
    assert(o);

    if (Future_CheckExact(o) || Task_CheckExact(o)) {
        PyObject *fut_result = NULL;
        int res = future_get_result((FutureObj*)o, &fut_result);

        switch(res) {
        case -1:
            assert(fut_result == NULL);
            break; /* exception raised */
        case 0:
            Py_DECREF(fut_result);
            return task_call_step(task, NULL);
        default:
            assert(res == 1);
            result = task_call_step(task, fut_result);
            Py_DECREF(fut_result);
            return result;
        }
    }
    else {
        PyObject *fut_result = PyObject_CallMethod(o, "result", NULL);
        if (fut_result != NULL) {
            Py_DECREF(fut_result);
            return task_call_step(task, NULL);
        }
        /* exception raised */
    }

    PyErr_Fetch(&et, &ev, &tb);
    if (!PyErr_GivenExceptionMatches(et, PyExc_Exception)) {
        /* We've got a BaseException; re-raise it */
        PyErr_Restore(et, ev, tb);
        return NULL;
    }
    if (!ev || !PyObject_TypeCheck(ev, (PyTypeObject *) et)) {
        PyErr_NormalizeException(&et, &ev, &tb);
    }

    result = task_call_step(task, ev);

    Py_DECREF(et);
    Py_XDECREF(tb);
    Py_XDECREF(ev);

    return result;
}


/*********************** Module **************************/


static void
module_free(void *m)
{
    Py_CLEAR(current_tasks);
    Py_CLEAR(all_tasks);
    Py_CLEAR(traceback_extract_stack);
    Py_CLEAR(asyncio_get_event_loop);
    Py_CLEAR(asyncio_future_repr_info_func);
    Py_CLEAR(asyncio_task_repr_info_func);
    Py_CLEAR(asyncio_task_get_stack_func);
    Py_CLEAR(asyncio_task_print_stack_func);
    Py_CLEAR(asyncio_InvalidStateError);
    Py_CLEAR(asyncio_CancelledError);
    Py_CLEAR(inspect_isgenerator);
}

static int
module_init(void)
{
    PyObject *module = NULL;
    PyObject *cls;

#define WITH_MOD(NAME) \
    Py_CLEAR(module); \
    module = PyImport_ImportModule(NAME); \
    if (module == NULL) { \
        goto fail; \
    }

#define GET_MOD_ATTR(VAR, NAME) \
    VAR = PyObject_GetAttrString(module, NAME); \
    if (VAR == NULL) { \
        goto fail; \
    }

    WITH_MOD("asyncio.events")
    GET_MOD_ATTR(asyncio_get_event_loop, "get_event_loop")

    WITH_MOD("asyncio.base_futures")
    GET_MOD_ATTR(asyncio_future_repr_info_func, "_future_repr_info")
    GET_MOD_ATTR(asyncio_InvalidStateError, "InvalidStateError")
    GET_MOD_ATTR(asyncio_CancelledError, "CancelledError")

    WITH_MOD("asyncio.base_tasks")
    GET_MOD_ATTR(asyncio_task_repr_info_func, "_task_repr_info")
    GET_MOD_ATTR(asyncio_task_get_stack_func, "_task_get_stack")
    GET_MOD_ATTR(asyncio_task_print_stack_func, "_task_print_stack")

    WITH_MOD("inspect")
    GET_MOD_ATTR(inspect_isgenerator, "isgenerator")

    WITH_MOD("traceback")
    GET_MOD_ATTR(traceback_extract_stack, "extract_stack")

    WITH_MOD("weakref")
    GET_MOD_ATTR(cls, "WeakSet")
    all_tasks = _PyObject_CallNoArg(cls);
    Py_DECREF(cls);
    if (all_tasks == NULL) {
        goto fail;
    }

    current_tasks = PyDict_New();
    if (current_tasks == NULL) {
        goto fail;
    }

    Py_DECREF(module);
    return 0;

fail:
    Py_CLEAR(module);
    module_free(NULL);
    return -1;

#undef WITH_MOD
#undef GET_MOD_ATTR
}

PyDoc_STRVAR(module_doc, "Accelerator module for asyncio");

static struct PyModuleDef _asynciomodule = {
    PyModuleDef_HEAD_INIT,      /* m_base */
    "_asyncio",                 /* m_name */
    module_doc,                 /* m_doc */
    -1,                         /* m_size */
    NULL,                       /* m_methods */
    NULL,                       /* m_slots */
    NULL,                       /* m_traverse */
    NULL,                       /* m_clear */
    (freefunc)module_free       /* m_free */
};


PyMODINIT_FUNC
PyInit__asyncio(void)
{
    if (module_init() < 0) {
        return NULL;
    }
    if (PyType_Ready(&FutureType) < 0) {
        return NULL;
    }
    if (PyType_Ready(&FutureIterType) < 0) {
        return NULL;
    }
    if (PyType_Ready(&TaskStepMethWrapper_Type) < 0) {
        return NULL;
    }
    if(PyType_Ready(&TaskWakeupMethWrapper_Type) < 0) {
        return NULL;
    }
    if (PyType_Ready(&TaskType) < 0) {
        return NULL;
    }

    PyObject *m = PyModule_Create(&_asynciomodule);
    if (m == NULL) {
        return NULL;
    }

    Py_INCREF(&FutureType);
    if (PyModule_AddObject(m, "Future", (PyObject *)&FutureType) < 0) {
        Py_DECREF(&FutureType);
        return NULL;
    }

    Py_INCREF(&TaskType);
    if (PyModule_AddObject(m, "Task", (PyObject *)&TaskType) < 0) {
        Py_DECREF(&TaskType);
        return NULL;
    }

    return m;
}
