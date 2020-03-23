import setuptools
from os import path
from cmake_ext import CMakeExtension, CMakeBuildExt
from metric import __version__


this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setuptools.setup(
    name='metric-py',
    version=__version__,
    ext_modules=[CMakeExtension('all')],
    cmdclass={'build_ext': CMakeBuildExt},
    author="Jura Gresko",
    author_email="juragresko@gmail.com",
    description="Metric python3 module",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/panda-official/metric",
    packages=setuptools.find_packages(),
    python_requires='>=3.6',
    install_requires=[],
    license='MPL v2.0',
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
        "Operating System :: Unix",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: Implementation :: CPython",
        "Topic :: Software Development",
        "Topic :: Scientific/Engineering"
    ],
    keywords=''
)
