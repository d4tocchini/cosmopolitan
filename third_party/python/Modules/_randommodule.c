/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=4 sts=4 sw=4 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Python 3                                                                     │
│ https://docs.python.org/3/license.html                                       │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/calls/calls.h"
#include "third_party/python/Include/floatobject.h"
#include "third_party/python/Include/longobject.h"
#include "third_party/python/Include/modsupport.h"
#include "third_party/python/Include/objimpl.h"
#include "third_party/python/Include/pyerrors.h"
#include "third_party/python/Include/pylifecycle.h"
#include "third_party/python/Include/pymacro.h"
#include "third_party/python/Include/pymem.h"
#include "third_party/python/Include/pytime.h"
#include "third_party/python/Include/tupleobject.h"
/* clang-format off */

/* ------------------------------------------------------------------
   The code in this module was based on a download from:
      http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/emt19937ar.html

   It was modified in 2002 by Raymond Hettinger as follows:

    * the principal computational lines untouched.

    * renamed genrand_res53() to random_random() and wrapped
      in python calling/return code.

    * genrand_int32() and the helper functions, init_genrand()
      and init_by_array(), were declared static, wrapped in
      Python calling/return code.  also, their global data
      references were replaced with structure references.

    * unused functions from the original were deleted.
      new, original C python code was added to implement the
      Random() interface.

   The following are the verbatim comments from the original code:

   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/* ---------------------------------------------------------------*/

/* Period parameters -- These are all magic.  Don't change. */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU    /* constant vector a */
#define UPPER_MASK 0x80000000U  /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffU  /* least significant r bits */

typedef struct {
    PyObject_HEAD
    int index;
    uint32_t state[N];
} RandomObject;

static PyTypeObject Random_Type;

#define RandomObject_Check(v)      (Py_TYPE(v) == &Random_Type)


/* Random methods */


/* generates a random number on [0,0xffffffff]-interval */
static uint32_t
genrand_int32(RandomObject *self)
{
    uint32_t y;
    static const uint32_t mag01[2] = {0x0U, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    uint32_t *mt;

    mt = self->state;
    if (self->index >= N) { /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1U];

        self->index = 0;
    }

    y = mt[self->index++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680U;
    y ^= (y << 15) & 0xefc60000U;
    y ^= (y >> 18);
    return y;
}

/* random_random is the function named genrand_res53 in the original code;
 * generates a random number on [0,1) with 53-bit resolution; note that
 * 9007199254740992 == 2**53; I assume they're spelling "/2**53" as
 * multiply-by-reciprocal in the (likely vain) hope that the compiler will
 * optimize the division away at compile-time.  67108864 is 2**26.  In
 * effect, a contains 27 random bits shifted left 26, and b fills in the
 * lower 26 bits of the 53-bit numerator.
 * The original code credited Isaku Wada for this algorithm, 2002/01/09.
 */
static PyObject *
random_random(RandomObject *self)
{
    uint32_t a=genrand_int32(self)>>5, b=genrand_int32(self)>>6;
    return PyFloat_FromDouble((a*67108864.0+b)*(1.0/9007199254740992.0));
}

/* initializes mt[N] with a seed */
static void
init_genrand(RandomObject *self, uint32_t s)
{
    int mti;
    uint32_t *mt;

    mt = self->state;
    mt[0]= s;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253U * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                                */
        /* 2002/01/09 modified by Makoto Matsumoto                     */
    }
    self->index = mti;
    return;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
static void
init_by_array(RandomObject *self, uint32_t init_key[], size_t key_length)
{
    size_t i, j, k;       /* was signed in the original code. RDH 12/16/2002 */
    uint32_t *mt;

    mt = self->state;
    init_genrand(self, 19650218U);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525U))
                 + init_key[j] + (uint32_t)j; /* non linear */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941U))
                 - (uint32_t)i; /* non linear */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000U; /* MSB is 1; assuring non-zero initial array */
}

/*
 * The rest is Python-specific code, neither part of, nor derived from, the
 * Twister download.
 */

static int
random_seed_urandom(RandomObject *self)
{
    PY_UINT32_T key[N];

    if (_PyOS_URandomNonblock(key, sizeof(key)) < 0) {
        return -1;
    }
    init_by_array(self, key, Py_ARRAY_LENGTH(key));
    return 0;
}

static void
random_seed_time_pid(RandomObject *self)
{
    _PyTime_t now;
    uint32_t key[5];

    now = _PyTime_GetSystemClock();
    key[0] = (PY_UINT32_T)(now & 0xffffffffU);
    key[1] = (PY_UINT32_T)(now >> 32);

    key[2] = (PY_UINT32_T)getpid();

    now = _PyTime_GetMonotonicClock();
    key[3] = (PY_UINT32_T)(now & 0xffffffffU);
    key[4] = (PY_UINT32_T)(now >> 32);

    init_by_array(self, key, Py_ARRAY_LENGTH(key));
}

static PyObject *
random_seed(RandomObject *self, PyObject *args)
{
    PyObject *result = NULL;            /* guilty until proved innocent */
    PyObject *n = NULL;
    uint32_t *key = NULL;
    size_t bits, keyused;
    int res;
    PyObject *arg = NULL;

    if (!PyArg_UnpackTuple(args, "seed", 0, 1, &arg))
        return NULL;

     if (arg == NULL || arg == Py_None) {
        if (random_seed_urandom(self) < 0) {
            PyErr_Clear();

            /* Reading system entropy failed, fall back on the worst entropy:
               use the current time and process identifier. */
            random_seed_time_pid(self);
        }
        Py_RETURN_NONE;
    }

    /* This algorithm relies on the number being unsigned.
     * So: if the arg is a PyLong, use its absolute value.
     * Otherwise use its hash value, cast to unsigned.
     */
    if (PyLong_Check(arg)) {
        /* Calling int.__abs__() prevents calling arg.__abs__(), which might
           return an invalid value. See issue #31478. */
        n = PyLong_Type.tp_as_number->nb_absolute(arg);
    }
    else {
        Py_hash_t hash = PyObject_Hash(arg);
        if (hash == -1)
            goto Done;
        n = PyLong_FromSize_t((size_t)hash);
    }
    if (n == NULL)
        goto Done;

    /* Now split n into 32-bit chunks, from the right. */
    bits = _PyLong_NumBits(n);
    if (bits == (size_t)-1 && PyErr_Occurred())
        goto Done;

    /* Figure out how many 32-bit chunks this gives us. */
    keyused = bits == 0 ? 1 : (bits - 1) / 32 + 1;

    /* Convert seed to byte sequence. */
    key = (uint32_t *)PyMem_Malloc((size_t)4 * keyused);
    if (key == NULL) {
        PyErr_NoMemory();
        goto Done;
    }
    res = _PyLong_AsByteArray((PyLongObject *)n,
                              (unsigned char *)key, keyused * 4,
                              PY_LITTLE_ENDIAN,
                              0); /* unsigned */
    if (res == -1) {
        PyMem_Free(key);
        goto Done;
    }

#if PY_BIG_ENDIAN
    {
        size_t i, j;
        /* Reverse an array. */
        for (i = 0, j = keyused - 1; i < j; i++, j--) {
            uint32_t tmp = key[i];
            key[i] = key[j];
            key[j] = tmp;
        }
    }
#endif
    init_by_array(self, key, keyused);

    Py_INCREF(Py_None);
    result = Py_None;

Done:
    Py_XDECREF(n);
    PyMem_Free(key);
    return result;
}

static PyObject *
random_getstate(RandomObject *self)
{
    PyObject *state;
    PyObject *element;
    int i;

    state = PyTuple_New(N+1);
    if (state == NULL)
        return NULL;
    for (i=0; i<N ; i++) {
        element = PyLong_FromUnsignedLong(self->state[i]);
        if (element == NULL)
            goto Fail;
        PyTuple_SET_ITEM(state, i, element);
    }
    element = PyLong_FromLong((long)(self->index));
    if (element == NULL)
        goto Fail;
    PyTuple_SET_ITEM(state, i, element);
    return state;

Fail:
    Py_DECREF(state);
    return NULL;
}

static PyObject *
random_setstate(RandomObject *self, PyObject *state)
{
    int i;
    unsigned long element;
    long index;
    uint32_t new_state[N];

    if (!PyTuple_Check(state)) {
        PyErr_SetString(PyExc_TypeError,
            "state vector must be a tuple");
        return NULL;
    }
    if (PyTuple_Size(state) != N+1) {
        PyErr_SetString(PyExc_ValueError,
            "state vector is the wrong size");
        return NULL;
    }

    for (i=0; i<N ; i++) {
        element = PyLong_AsUnsignedLong(PyTuple_GET_ITEM(state, i));
        if (element == (unsigned long)-1 && PyErr_Occurred())
            return NULL;
        new_state[i] = (uint32_t)element;
    }

    index = PyLong_AsLong(PyTuple_GET_ITEM(state, i));
    if (index == -1 && PyErr_Occurred())
        return NULL;
    if (index < 0 || index > N) {
        PyErr_SetString(PyExc_ValueError, "invalid state");
        return NULL;
    }
    self->index = (int)index;
    for (i = 0; i < N; i++)
        self->state[i] = new_state[i];

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
random_getrandbits(RandomObject *self, PyObject *args)
{
    int k, i, words;
    uint32_t r;
    uint32_t *wordarray;
    PyObject *result;

    if (!PyArg_ParseTuple(args, "i:getrandbits", &k))
        return NULL;

    if (k <= 0) {
        PyErr_SetString(PyExc_ValueError,
                        "number of bits must be greater than zero");
        return NULL;
    }

    if (k <= 32)  /* Fast path */
        return PyLong_FromUnsignedLong(genrand_int32(self) >> (32 - k));

    words = (k - 1) / 32 + 1;
    wordarray = (uint32_t *)PyMem_Malloc(words * 4);
    if (wordarray == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    /* Fill-out bits of long integer, by 32-bit words, from least significant
       to most significant. */
#if PY_LITTLE_ENDIAN
    for (i = 0; i < words; i++, k -= 32)
#else
    for (i = words - 1; i >= 0; i--, k -= 32)
#endif
    {
        r = genrand_int32(self);
        if (k < 32)
            r >>= (32 - k);  /* Drop least significant bits */
        wordarray[i] = r;
    }

    result = _PyLong_FromByteArray((unsigned char *)wordarray, words * 4,
                                   PY_LITTLE_ENDIAN, 0 /* unsigned */);
    PyMem_Free(wordarray);
    return result;
}

static PyObject *
random_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    RandomObject *self;
    PyObject *tmp;

    if (type == &Random_Type && !_PyArg_NoKeywords("Random()", kwds))
        return NULL;

    self = (RandomObject *)type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;
    tmp = random_seed(self, args);
    if (tmp == NULL) {
        Py_DECREF(self);
        return NULL;
    }
    Py_DECREF(tmp);
    return (PyObject *)self;
}

static PyMethodDef random_methods[] = {
    {"random",          (PyCFunction)random_random,  METH_NOARGS,
        PyDoc_STR("random() -> x in the interval [0, 1).")},
    {"seed",            (PyCFunction)random_seed,  METH_VARARGS,
        PyDoc_STR("seed([n]) -> None.  Defaults to current time.")},
    {"getstate",        (PyCFunction)random_getstate,  METH_NOARGS,
        PyDoc_STR("getstate() -> tuple containing the current state.")},
    {"setstate",          (PyCFunction)random_setstate,  METH_O,
        PyDoc_STR("setstate(state) -> None.  Restores generator state.")},
    {"getrandbits",     (PyCFunction)random_getrandbits,  METH_VARARGS,
        PyDoc_STR("getrandbits(k) -> x.  Generates an int with "
                  "k random bits.")},
    {NULL,              NULL}           /* sentinel */
};

PyDoc_STRVAR(random_doc,
"Random() -> create a random number generator with its own internal state.");

static PyTypeObject Random_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_random.Random",                   /*tp_name*/
    sizeof(RandomObject),               /*tp_basicsize*/
    0,                                  /*tp_itemsize*/
    /* methods */
    0,                                  /*tp_dealloc*/
    0,                                  /*tp_print*/
    0,                                  /*tp_getattr*/
    0,                                  /*tp_setattr*/
    0,                                  /*tp_reserved*/
    0,                                  /*tp_repr*/
    0,                                  /*tp_as_number*/
    0,                                  /*tp_as_sequence*/
    0,                                  /*tp_as_mapping*/
    0,                                  /*tp_hash*/
    0,                                  /*tp_call*/
    0,                                  /*tp_str*/
    PyObject_GenericGetAttr,            /*tp_getattro*/
    0,                                  /*tp_setattro*/
    0,                                  /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /*tp_flags*/
    random_doc,                         /*tp_doc*/
    0,                                  /*tp_traverse*/
    0,                                  /*tp_clear*/
    0,                                  /*tp_richcompare*/
    0,                                  /*tp_weaklistoffset*/
    0,                                  /*tp_iter*/
    0,                                  /*tp_iternext*/
    random_methods,                     /*tp_methods*/
    0,                                  /*tp_members*/
    0,                                  /*tp_getset*/
    0,                                  /*tp_base*/
    0,                                  /*tp_dict*/
    0,                                  /*tp_descr_get*/
    0,                                  /*tp_descr_set*/
    0,                                  /*tp_dictoffset*/
    0,                                  /*tp_init*/
    0,                                  /*tp_alloc*/
    random_new,                         /*tp_new*/
    PyObject_Free,                      /*tp_free*/
    0,                                  /*tp_is_gc*/
};

PyDoc_STRVAR(module_doc,
"Module implements the Mersenne Twister random number generator.");


static struct PyModuleDef _randommodule = {
    PyModuleDef_HEAD_INIT,
    "_random",
    module_doc,
    -1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

PyMODINIT_FUNC
PyInit__random(void)
{
    PyObject *m;

    if (PyType_Ready(&Random_Type) < 0)
        return NULL;
    m = PyModule_Create(&_randommodule);
    if (m == NULL)
        return NULL;
    Py_INCREF(&Random_Type);
    PyModule_AddObject(m, "Random", (PyObject *)&Random_Type);
    return m;
}
