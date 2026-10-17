"""ctypes binding to libmai (see src/mai.h in the mai repository)."""
import ctypes as C
import os
from pathlib import Path

MAI_OK, MAI_EPARSE, MAI_ERUN, MAI_EARG = range(4)
MAI_NONE, MAI_BOOLEAN, MAI_INT, MAI_DOUBLE, MAI_OTHER = range(5)


class mai_value_t(C.Structure):
    _fields_ = [
        ("type", C.c_int),
        ("length", C.c_int),
        ("rank", C.c_int),
        ("shape", C.POINTER(C.c_int)),
        ("data", C.c_void_p),
    ]


def _load():
    path = os.environ.get("MAI_LIB") or Path(__file__).with_name("libmai.so")
    # PyDLL keeps the GIL held during calls: libmai is a single,
    # process-wide interpreter and must not be entered from two threads.
    lib = C.PyDLL(str(path))
    lib.mai_init.argtypes = []
    lib.mai_init.restype = None
    lib.mai_fini.argtypes = []
    lib.mai_fini.restype = None
    lib.mai_eval.argtypes = [C.c_char_p, C.POINTER(C.c_void_p)]
    lib.mai_eval.restype = C.c_int
    lib.mai_get.argtypes = [C.c_char_p, C.POINTER(mai_value_t)]
    lib.mai_get.restype = C.c_int
    lib.mai_set.argtypes = [C.c_char_p, C.POINTER(mai_value_t)]
    lib.mai_set.restype = C.c_int
    lib.mai_free.argtypes = [C.c_void_p]
    lib.mai_free.restype = None
    lib.mai_init()
    return lib


lib = _load()
