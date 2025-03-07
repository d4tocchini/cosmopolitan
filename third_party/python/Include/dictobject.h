#ifndef Py_DICTOBJECT_H
#define Py_DICTOBJECT_H
#include "third_party/python/Include/object.h"
COSMOPOLITAN_C_START_
/* clang-format off */

/* Dictionary object type -- mapping from hashable object to object */

/* The distribution includes a separate file, Objects/dictnotes.txt,
   describing explorations into dictionary design and optimization.
   It covers typical dictionary use patterns, the parameters for
   tuning dictionaries, and several ideas for possible optimizations.
*/

#ifndef Py_LIMITED_API

typedef struct _dictkeysobject PyDictKeysObject;

/* The ma_values pointer is NULL for a combined table
 * or points to an array of PyObject* for a split table
 */
typedef struct {
    PyObject_HEAD

    /* Number of items in the dictionary */
    Py_ssize_t ma_used;

    /* Dictionary version: globally unique, value change each time
       the dictionary is modified */
    uint64_t ma_version_tag;

    PyDictKeysObject *ma_keys;

    /* If ma_values is NULL, the table is "combined": keys and values
       are stored in ma_keys.

       If ma_values is not NULL, the table is splitted:
       keys are stored in ma_keys and values are stored in ma_values */
    PyObject **ma_values;
} PyDictObject;

typedef struct {
    PyObject_HEAD
    PyDictObject *dv_dict;
} _PyDictViewObject;

#endif /* Py_LIMITED_API */

extern PyTypeObject PyDict_Type;
extern PyTypeObject PyDictIterKey_Type;
extern PyTypeObject PyDictIterValue_Type;
extern PyTypeObject PyDictIterItem_Type;
extern PyTypeObject PyDictKeys_Type;
extern PyTypeObject PyDictItems_Type;
extern PyTypeObject PyDictValues_Type;

#define PyDict_Check(op) \
                 PyType_FastSubclass(Py_TYPE(op), Py_TPFLAGS_DICT_SUBCLASS)
#define PyDict_CheckExact(op) (Py_TYPE(op) == &PyDict_Type)
#define PyDictKeys_Check(op) PyObject_TypeCheck(op, &PyDictKeys_Type)
#define PyDictItems_Check(op) PyObject_TypeCheck(op, &PyDictItems_Type)
#define PyDictValues_Check(op) PyObject_TypeCheck(op, &PyDictValues_Type)
/* This excludes Values, since they are not sets. */
# define PyDictViewSet_Check(op) \
    (PyDictKeys_Check(op) || PyDictItems_Check(op))


PyObject * PyDict_New(void);
PyObject * PyDict_GetItem(PyObject *mp, PyObject *key);
#ifndef Py_LIMITED_API
PyObject * _PyDict_GetItem_KnownHash(PyObject *mp, PyObject *key,
                                       Py_hash_t hash);
#endif
PyObject * PyDict_GetItemWithError(PyObject *mp, PyObject *key);
#ifndef Py_LIMITED_API
PyObject * _PyDict_GetItemIdWithError(PyObject *dp,
                                                  struct _Py_Identifier *key);
PyObject * PyDict_SetDefault(
    PyObject *mp, PyObject *key, PyObject *defaultobj);
#endif
int PyDict_SetItem(PyObject *mp, PyObject *key, PyObject *item);
#ifndef Py_LIMITED_API
int _PyDict_SetItem_KnownHash(PyObject *mp, PyObject *key,
                                          PyObject *item, Py_hash_t hash);
#endif
int PyDict_DelItem(PyObject *mp, PyObject *key);
#ifndef Py_LIMITED_API
int _PyDict_DelItem_KnownHash(PyObject *mp, PyObject *key,
                                          Py_hash_t hash);
int _PyDict_DelItemIf(PyObject *mp, PyObject *key,
                                  int (*predicate)(PyObject *value));
#endif
void PyDict_Clear(PyObject *mp);
int PyDict_Next(
    PyObject *mp, Py_ssize_t *pos, PyObject **key, PyObject **value);
#ifndef Py_LIMITED_API
PyDictKeysObject *_PyDict_NewKeysForClass(void);
PyObject * PyObject_GenericGetDict(PyObject *, void *);
int _PyDict_Next(
    PyObject *mp, Py_ssize_t *pos, PyObject **key, PyObject **value, Py_hash_t *hash);
PyObject *_PyDictView_New(PyObject *, PyTypeObject *);
#endif
PyObject * PyDict_Keys(PyObject *mp);
PyObject * PyDict_Values(PyObject *mp);
PyObject * PyDict_Items(PyObject *mp);
Py_ssize_t PyDict_Size(PyObject *mp);
PyObject * PyDict_Copy(PyObject *mp);
int PyDict_Contains(PyObject *mp, PyObject *key);
#ifndef Py_LIMITED_API
int _PyDict_Contains(PyObject *mp, PyObject *key, Py_hash_t hash);
PyObject * _PyDict_NewPresized(Py_ssize_t minused);
void _PyDict_MaybeUntrack(PyObject *mp);
int _PyDict_HasOnlyStringKeys(PyObject *mp);
Py_ssize_t _PyDict_KeysSize(PyDictKeysObject *keys);
Py_ssize_t _PyDict_SizeOf(PyDictObject *);
PyObject * _PyDict_Pop(PyObject *, PyObject *, PyObject *);
PyObject *_PyDict_Pop_KnownHash(PyObject *, PyObject *, Py_hash_t, PyObject *);
PyObject *_PyDict_FromKeys(PyObject *, PyObject *, PyObject *);
#define _PyDict_HasSplitTable(d) ((d)->ma_values != NULL)

int PyDict_ClearFreeList(void);
#endif

/* PyDict_Update(mp, other) is equivalent to PyDict_Merge(mp, other, 1). */
int PyDict_Update(PyObject *mp, PyObject *other);

/* PyDict_Merge updates/merges from a mapping object (an object that
   supports PyMapping_Keys() and PyObject_GetItem()).  If override is true,
   the last occurrence of a key wins, else the first.  The Python
   dict.update(other) is equivalent to PyDict_Merge(dict, other, 1).
*/
int PyDict_Merge(PyObject *mp,
                                   PyObject *other,
                                   int override);

#ifndef Py_LIMITED_API
/* Like PyDict_Merge, but override can be 0, 1 or 2.  If override is 0,
   the first occurrence of a key wins, if override is 1, the last occurrence
   of a key wins, if override is 2, a KeyError with conflicting key as
   argument is raised.
*/
int _PyDict_MergeEx(PyObject *mp, PyObject *other, int override);
PyObject * _PyDictView_Intersect(PyObject* self, PyObject *other);
#endif

/* PyDict_MergeFromSeq2 updates/merges from an iterable object producing
   iterable objects of length 2.  If override is true, the last occurrence
   of a key wins, else the first.  The Python dict constructor dict(seq2)
   is equivalent to dict={}; PyDict_MergeFromSeq(dict, seq2, 1).
*/
int PyDict_MergeFromSeq2(PyObject *d,
                                           PyObject *seq2,
                                           int override);

PyObject * PyDict_GetItemString(PyObject *dp, const char *key);
#ifndef Py_LIMITED_API
PyObject * _PyDict_GetItemId(PyObject *dp, struct _Py_Identifier *key);
#endif /* !Py_LIMITED_API */
int PyDict_SetItemString(PyObject *dp, const char *key, PyObject *item);
#ifndef Py_LIMITED_API
int _PyDict_SetItemId(PyObject *dp, struct _Py_Identifier *key, PyObject *item);
#endif /* !Py_LIMITED_API */
int PyDict_DelItemString(PyObject *dp, const char *key);

#ifndef Py_LIMITED_API
int _PyDict_DelItemId(PyObject *mp, struct _Py_Identifier *key);
void _PyDict_DebugMallocStats(FILE *out);

int _PyObjectDict_SetItem(PyTypeObject *tp, PyObject **dictptr, PyObject *name, PyObject *value);
PyObject *_PyDict_LoadGlobal(PyDictObject *, PyDictObject *, PyObject *);
#endif

COSMOPOLITAN_C_END_
#endif /* !Py_DICTOBJECT_H */
