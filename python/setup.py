# Builds libmai from the enclosing mai checkout and ships it inside the
# package. Set MAI_LIB to a prebuilt libmai.so to skip the build.
import os
import shutil
import subprocess
from pathlib import Path

from setuptools import setup
from setuptools.command.build_py import build_py
from setuptools.dist import Distribution

ROOT = Path(__file__).resolve().parent.parent


class build_with_libmai(build_py):
    def run(self):
        super().run()
        lib = os.environ.get("MAI_LIB")
        if not lib:
            subprocess.check_call(["make", "-C", str(ROOT), "lib", "CTUNE="])
            lib = ROOT / "build" / "libmai.so"
        dest = Path(self.build_lib) / "pymai"
        dest.mkdir(parents=True, exist_ok=True)
        shutil.copy(lib, dest / "libmai.so")


class BinaryDistribution(Distribution):
    # the wheel carries a native library, so it is platform specific
    def has_ext_modules(self):
        return True


setup(cmdclass={"build_py": build_with_libmai}, distclass=BinaryDistribution)
