from setuptools import setup, Extension, find_packages
from Cython.Build import cythonize

ext_modules = [
    Extension("strgraphcpp.cppext", ["strgraph.pyx"], include_dirs=["./include"]),
]

setup(
    name="strgraphcpp",
    packages=find_packages(),
    package_dir={'strgraphcpp': '.'},
    ext_modules=cythonize(ext_modules,
                          language_level=3),
)
