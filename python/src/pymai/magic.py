"""IPython magics: ``%load_ext pymai``, then ``%%mai`` or ``%mai``."""
from IPython.core.magic import Magics, line_cell_magic, magics_class

import pymai


@magics_class
class MaiMagics(Magics):
    @line_cell_magic
    def mai(self, line, cell=None):
        """Run mai code. As ``%mai expr`` it returns the value's text; as a
        ``%%mai`` cell it prints what the code prints, then the value of
        the last expression."""
        code = line if cell is None else cell
        try:
            value, output = pymai.eval(code, capture=True)
        except pymai.MaiError as e:
            print(f"{type(e).__name__}: {e}")
            return None
        if output:
            print(output, end="" if output.endswith("\n") else "\n")
        if cell is None:
            return value
        if value is not None:
            print(value)
