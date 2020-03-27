import setuptools
from os import path
import re
from cmake_ext import CMakeExtension, CMakeBuildExt


HERE = path.abspath(path.dirname(__file__))


def get_file_content(*paths):
    with open(path.join(HERE, *paths), 'r', encoding='utf-8') as fp:
        return fp.read()


def find_version(*file_paths):
    version_file = get_file_content(*file_paths)
    version_match = re.search(r"^__version__ = ['\"]([^'\"]*)['\"]",
                              version_file, re.M)
    if version_match:
        return version_match.group(1)
    raise RuntimeError("Unable to find version string.")


setuptools.setup(
    name='metric-py',
    version=find_version('pkg', 'metric', '__init__.py'),
    ext_modules=[CMakeExtension('all')],
    cmdclass={'build_ext': CMakeBuildExt},
    author="Jura Gresko",
    author_email="juragresko@gmail.com",
    description="Metric python3 module",
    long_description=get_file_content('README.md'),
    long_description_content_type="text/markdown",
    url="https://github.com/panda-official/metric",
    packages=setuptools.find_packages(where='pkg'),
    package_dir={
        '': 'pkg',
    },
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
