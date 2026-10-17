import numpy as np
import pytest

import pymai


def test_eval_keeps_definitions():
    assert pymai.eval("const sq = function(x) x * x") is None
    assert pymai.eval("sq(12)") == "144"


def test_errors():
    with pytest.raises(pymai.MaiSyntaxError):
        pymai.eval("1 +")
    with pytest.raises(pymai.MaiRuntimeError, match="not defined"):
        pymai.eval("no_such_thing + 1")


def test_capture_print():
    value, output = pymai.eval("print(42)\n7", capture=True)
    assert value == "7"
    assert output.strip() == "42"


def test_scalars_round_trip():
    pymai.set("n", 7)
    pymai.set("d", 2.5)
    pymai.set("b", True)
    assert pymai.eval("n + 1") == "8"
    assert pymai.get("n") == 7 and isinstance(pymai.get("n"), int)
    assert pymai.get("d") == 2.5
    assert pymai.get("b") is True


def test_vectors_round_trip():
    x = np.linspace(0, 1, 5)
    pymai.set("xs", x)
    pymai.eval("ys = xs * 2")
    np.testing.assert_array_equal(pymai.get("ys"), x * 2)
    pymai.set("is", [3, 1, 2])
    assert pymai.get("is").dtype == np.int32
    pymai.set("empty", np.array([], dtype=float))
    assert pymai.get("empty").shape == (0,)


def test_arrays_are_column_major():
    m = np.arange(6).reshape(2, 3)
    pymai.set("m", m)
    assert pymai.eval("m[2, 3]") == "5"
    assert pymai.eval("m[1, 2]") == "1"
    np.testing.assert_array_equal(pymai.get("m"), m)


def test_loop_over_numpy_data():
    pymai.set("v", np.arange(1, 1001, dtype=float))
    pymai.eval("""
const total = function(vector(double) x) : double {
    var s = 0.0
    for(i = 1; i <= length(x); i = i + 1) s = s + x[i]
    s
}
""")
    assert float(pymai.eval("total(v)")) == 500500.0


def test_get_and_set_refusals():
    with pytest.raises(KeyError):
        pymai.get("never_defined")
    with pytest.raises(TypeError):
        pymai.get("sq")
    pymai.eval("const k = 1.0")
    with pytest.raises(ValueError):
        pymai.set("k", 2.0)
    with pytest.raises(TypeError):
        pymai.set("s", ["a", "b"])


def test_cell_magic(capsys):
    IPython = pytest.importorskip("IPython")
    from IPython.testing.globalipapp import start_ipython

    ip = start_ipython()
    ip.run_line_magic("load_ext", "pymai")
    ip.run_cell_magic("mai", "", "print(1)\nconst three = 3\nthree + 1")
    out = capsys.readouterr().out
    assert "1" in out and "4" in out
    assert ip.run_line_magic("mai", "three * 2") == "6"
