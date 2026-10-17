"""Run mai code from Python.

    >>> import pymai
    >>> pymai.eval("const sq = function(x) x * x")
    >>> pymai.eval("sq(12)")
    '144'
    >>> pymai.set("xs", [1.5, 2.5, 3.0])
    >>> pymai.eval("ys = xs * 2")
    >>> pymai.get("ys")
    array([3., 5., 6.])

There is one mai interpreter per process, so globals persist from one call
to the next. In IPython or Jupyter, ``%load_ext pymai`` adds the ``%%mai``
cell magic.
"""
import ctypes as C
import os
import sys
import tempfile

import numpy as np

from . import _lib
from ._lib import lib

__all__ = ["eval", "get", "set", "MaiError", "MaiSyntaxError", "MaiRuntimeError"]

_DTYPES = {
    _lib.MAI_BOOLEAN: np.dtype(np.bool_),
    _lib.MAI_INT: np.dtype(np.int32),
    _lib.MAI_DOUBLE: np.dtype(np.float64),
}
_SCALARS = {_lib.MAI_BOOLEAN: bool, _lib.MAI_INT: int, _lib.MAI_DOUBLE: float}


class MaiError(Exception):
    """An error reported by mai."""


class MaiSyntaxError(MaiError):
    """mai could not parse or compile the code."""


class MaiRuntimeError(MaiError):
    """The code failed while it ran."""


_libc = C.CDLL(None)


def _run(code):
    out = C.c_void_p()
    status = lib.mai_eval(code.encode(), C.byref(out))
    text = C.string_at(out).decode() if out.value else ""
    if out.value:
        lib.mai_free(out)
    return status, text


def eval(code, capture=False):
    """Run ``code`` at mai's top level and return the printed value of its
    last expression, or None if it has no value.

    With ``capture=True``, return ``(value, output)`` where ``output`` is
    whatever the code printed with ``print()`` and friends, which otherwise
    goes straight to the process's standard output.
    """
    if not capture:
        status, text = _run(code)
        output = None
    else:
        sys.stdout.flush()
        _libc.fflush(None)
        saved = os.dup(1)
        with tempfile.TemporaryFile() as tmp:
            os.dup2(tmp.fileno(), 1)
            try:
                status, text = _run(code)
            finally:
                _libc.fflush(None)
                os.dup2(saved, 1)
                os.close(saved)
            tmp.seek(0)
            output = tmp.read().decode()
    if status == _lib.MAI_EPARSE:
        raise MaiSyntaxError(text.strip())
    if status != _lib.MAI_OK:
        raise MaiRuntimeError(text.strip())
    value = None if text == "<nil>" else text
    return (value, output) if capture else value


def get(name):
    """Return a copy of mai global ``name``: a bool, int or float for a
    scalar, a 1-d numpy array for a vector, or an n-d array for an array."""
    v = _lib.mai_value_t()
    if lib.mai_get(name.encode(), C.byref(v)) != _lib.MAI_OK:
        raise KeyError(name)
    if v.type not in _DTYPES:
        raise TypeError(f"mai global `{name}` is not a boolean, int or "
                        "double scalar, vector or array")
    dtype = _DTYPES[v.type]
    buf = C.string_at(v.data, v.length * dtype.itemsize) if v.length else b""
    flat = np.frombuffer(buf, dtype=dtype).copy()
    if v.rank == 0:
        return _SCALARS[v.type](flat[0])
    if v.rank == 1:
        return flat
    shape = tuple(v.shape[i] for i in range(v.rank))
    return flat.reshape(shape, order="F")


def _mai_type(arr):
    if arr.dtype == np.bool_:
        return _lib.MAI_BOOLEAN, arr
    if np.issubdtype(arr.dtype, np.integer):
        if arr.size and (arr.min() < -2**31 + 1 or arr.max() > 2**31 - 1):
            raise OverflowError("integers do not fit in mai's 32-bit int")
        return _lib.MAI_INT, arr.astype(np.int32)
    if np.issubdtype(arr.dtype, np.floating):
        return _lib.MAI_DOUBLE, arr.astype(np.float64)
    raise TypeError(f"can't pass {arr.dtype} values to mai")


def set(name, value):
    """Copy ``value`` into mai global ``name``. A Python bool, int or float
    becomes a scalar, a 1-d sequence a vector, and an n-d array an array.
    Element types map to mai's boolean, int and double."""
    arr = np.asarray(value)
    typ, arr = _mai_type(arr)
    data = np.asfortranarray(arr) if arr.ndim >= 2 else np.ascontiguousarray(arr)
    shape = (C.c_int * max(arr.ndim, 1))(*arr.shape) if arr.ndim >= 2 else None
    v = _lib.mai_value_t(typ, int(arr.size), arr.ndim, shape,
                         data.ctypes.data if arr.size else None)
    if lib.mai_set(name.encode(), C.byref(v)) != _lib.MAI_OK:
        raise ValueError(f"can't set mai global `{name}`: it is const, or "
                         "declared with a different type")


def load_ipython_extension(ipython):
    from .magic import MaiMagics
    ipython.register_magics(MaiMagics)
