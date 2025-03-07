#ifndef Py_TUPLEOBJECT_H
#define Py_TUPLEOBJECT_H
#include "third_party/python/Include/object.h"
COSMOPOLITAN_C_START_
/* clang-format off */

/*
Another generally useful object type is a tuple of object pointers.
For Python, this is an immutable type.  C code can change the tuple items
(but not their number), and even use tuples are general-purpose arrays of
object references, but in general only brand new tuples should be mutated,
not ones that might already have been exposed to Python code.

*** WARNING *** PyTuple_SetItem does not increment the new item's reference
count, but does decrement the reference count of the item it replaces,
if not nil.  It does *decrement* the reference count if it is *not*
inserted in the tuple.  Similarly, PyTuple_GetItem does not increment the
returned item's reference count.
*/

#ifndef Py_LIMITED_API
typedef struct {
    PyObject_VAR_HEAD
    PyObject *ob_item[1];

    /* ob_item contains space for 'ob_size' elements.
     * Items must normally not be NULL, except during construction when
     * the tuple is not yet visible outside the function that builds it.
     */
} PyTupleObject;
#endif

extern PyTypeObject PyTuple_Type;
extern PyTypeObject PyTupleIter_Type;

#define PyTuple_Check(op) \
                 PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_TUPLE_SUBCLASS)
#define PyTuple_CheckExact(op) (Py_TYPE(op) == &PyTuple_Type)

PyObject * PyTuple_New(Py_ssize_t size);
Py_ssize_t PyTuple_Size(PyObject *);
PyObject * PyTuple_GetItem(PyObject *, Py_ssize_t);
int PyTuple_SetItem(PyObject *, Py_ssize_t, PyObject *);
PyObject * PyTuple_GetSlice(PyObject *, Py_ssize_t, Py_ssize_t);
#ifndef Py_LIMITED_API
int _PyTuple_Resize(PyObject **, Py_ssize_t);
#endif
PyObject * PyTuple_Pack(Py_ssize_t, ...);
#ifndef Py_LIMITED_API
void _PyTuple_MaybeUntrack(PyObject *);
#endif

/* Macro, trading safety for speed */
#ifndef Py_LIMITED_API
#define PyTuple_GET_ITEM(op, i) (((PyTupleObject *)(op))->ob_item[i])
#define PyTuple_GET_SIZE(op)    Py_SIZE(op)

/* Macro, *only* to be used to fill in brand new tuples */
#define PyTuple_SET_ITEM(op, i, v) (((PyTupleObject *)(op))->ob_item[i] = v)
#endif

int PyTuple_ClearFreeList(void);
#ifndef Py_LIMITED_API
void _PyTuple_DebugMallocStats(FILE *out);
#endif /* Py_LIMITED_API */

COSMOPOLITAN_C_END_
#endif /* !Py_TUPLEOBJECT_H */
