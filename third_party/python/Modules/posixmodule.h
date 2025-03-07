#ifndef Py_POSIXMODULE_H
#define Py_POSIXMODULE_H
#include "libc/calls/weirdtypes.h"
#include "third_party/python/Include/object.h"
COSMOPOLITAN_C_START_

#ifndef Py_LIMITED_API
#ifndef MS_WINDOWS
PyObject *_PyLong_FromUid(uid_t);
PyObject *_PyLong_FromGid(gid_t);
int _Py_Uid_Converter(PyObject *, void *);
int _Py_Gid_Converter(PyObject *, void *);
#endif /* MS_WINDOWS */
#endif

#undef HAVE_SETGROUPS
#undef HAVE_FORKPTY
#undef HAVE_SCHED_SETPARAM
#undef HAVE_SCHED_SETSCHEDULER
#undef HAVE_FCHMODAT
#undef HAVE_LINKAT
#undef HAVE_READLINKAT

COSMOPOLITAN_C_END_
#endif /* !Py_POSIXMODULE_H */
