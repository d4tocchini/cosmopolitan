#ifndef Py_LIMITED_API
#ifndef Py_FRAMEOBJECT_H
#define Py_FRAMEOBJECT_H
#include "third_party/python/Include/code.h"
#include "third_party/python/Include/object.h"
#include "third_party/python/Include/pystate.h"
COSMOPOLITAN_C_START_
/* clang-format off */

typedef struct {
    int b_type;                 /* what kind of block this is */
    int b_handler;              /* where to jump to find handler */
    int b_level;                /* value stack level to pop to */
} PyTryBlock;

typedef struct _frame {
    PyObject_VAR_HEAD
    struct _frame *f_back;      /* previous frame, or NULL */
    PyCodeObject *f_code;       /* code segment */
    PyObject *f_builtins;       /* builtin symbol table (PyDictObject) */
    PyObject *f_globals;        /* global symbol table (PyDictObject) */
    PyObject *f_locals;         /* local symbol table (any mapping) */
    PyObject **f_valuestack;    /* points after the last local */
    /* Next free slot in f_valuestack.  Frame creation sets to f_valuestack.
       Frame evaluation usually NULLs it, but a frame that yields sets it
       to the current stack top. */
    PyObject **f_stacktop;
    PyObject *f_trace;          /* Trace function */

    /* In a generator, we need to be able to swap between the exception
       state inside the generator and the exception state of the calling
       frame (which shouldn't be impacted when the generator "yields"
       from an except handler).
       These three fields exist exactly for that, and are unused for
       non-generator frames. See the save_exc_state and swap_exc_state
       functions in ceval.c for details of their use. */
    PyObject *f_exc_type, *f_exc_value, *f_exc_traceback;
    /* Borrowed reference to a generator, or NULL */
    PyObject *f_gen;

    int f_lasti;                /* Last instruction if called */
    /* Call PyFrame_GetLineNumber() instead of reading this field
       directly.  As of 2.3 f_lineno is only valid when tracing is
       active (i.e. when f_trace is set).  At other times we use
       PyCode_Addr2Line to calculate the line from the current
       bytecode index. */
    int f_lineno;               /* Current line number */
    int f_iblock;               /* index in f_blockstack */
    char f_executing;           /* whether the frame is still executing */
    PyTryBlock f_blockstack[CO_MAXBLOCKS]; /* for try and loop blocks */
    PyObject *f_localsplus[1];  /* locals+stack, dynamically sized */
} PyFrameObject;


/* Standard object interface */

extern PyTypeObject PyFrame_Type;

#define PyFrame_Check(op) (Py_TYPE(op) == &PyFrame_Type)

PyFrameObject * PyFrame_New(PyThreadState *, PyCodeObject *,
                            PyObject *, PyObject *);


/* The rest of the interface is specific for frame objects */

/* Block management functions */

void PyFrame_BlockSetup(PyFrameObject *, int, int, int);
PyTryBlock * PyFrame_BlockPop(PyFrameObject *);

/* Extend the value stack */

PyObject ** PyFrame_ExtendStack(PyFrameObject *, int, int);

/* Conversions between "fast locals" and locals in dictionary */

void PyFrame_LocalsToFast(PyFrameObject *, int);

int PyFrame_FastToLocalsWithError(PyFrameObject *f);
void PyFrame_FastToLocals(PyFrameObject *);

int PyFrame_ClearFreeList(void);

void _PyFrame_DebugMallocStats(FILE *out);

/* Return the line of code the frame is currently executing. */
int PyFrame_GetLineNumber(PyFrameObject *);

COSMOPOLITAN_C_END_
#endif /* !Py_FRAMEOBJECT_H */
#endif /* Py_LIMITED_API */
