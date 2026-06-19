import os
import subprocess
import shutil
import sys
from setuptools import Extension
from setuptools.command.build_ext import build_ext

CMAKE_EXE = os.environ.get('CMAKE_EXE', shutil.which('cmake'))


def check_for_cmake():
    if not CMAKE_EXE:
        print('cmake executable not found. '
              'Set CMAKE_EXE environment or update your path')
        sys.exit(1)


def pybind11_cmake_dir():
    try:
        import pybind11
    except ImportError:
        return None

    get_cmake_dir = getattr(pybind11, 'get_cmake_dir', None)
    if get_cmake_dir:
        return get_cmake_dir()

    try:
        return subprocess.check_output(
            [sys.executable, '-m', 'pybind11', '--cmakedir'],
            text=True
        ).strip()
    except (subprocess.CalledProcessError, OSError):
        return None


class CMakeExtension(Extension):
    """
    setuptools.Extension for cmake
    """

    def __init__(self, name, source_dir='', output_dir=''):
        check_for_cmake()
        Extension.__init__(self, name, sources=[])
        self.source_dir = os.path.abspath(source_dir)
        self.output_dir = output_dir


class CMakeBuildExt(build_ext):
    """
    setuptools build_exit which builds using cmake & make
    You can add cmake args with the CMAKE_COMMON_VARIABLES environment variable
    """

    def build_extension(self, ext):
        check_for_cmake()
        if isinstance(ext, CMakeExtension):
            output_dir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

            build_type = 'Debug' if self.debug else 'Release'
            cmake_args = [CMAKE_EXE,
                          ext.source_dir,
                          '-DPYTHON_EXECUTABLE:FILEPATH=' + sys.executable,
                          '-DPython_EXECUTABLE:FILEPATH=' + sys.executable,
                          '-DPython3_EXECUTABLE:FILEPATH=' + sys.executable,
                          '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + os.path.join(output_dir, ext.output_dir),
                          '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=' + os.path.join(output_dir, ext.output_dir),
                          '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=' + os.path.join(output_dir, ext.output_dir),
                          '-DCMAKE_BUILD_TYPE=' + build_type]
            pybind11_dir = pybind11_cmake_dir()
            if pybind11_dir:
                cmake_args.append('-Dpybind11_DIR:PATH=' + pybind11_dir)
            cmake_args.extend(
                [x for x in
                 os.environ.get('CMAKE_COMMON_VARIABLES', '').split(' ')
                 if x])

            env = os.environ.copy()
            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            subprocess.check_call(cmake_args,
                                  cwd=self.build_temp,
                                  env=env)
            build_args = [CMAKE_EXE, '--build', '.', '--config', build_type]
            if ext.name != 'all':
                build_args.extend(['--target', ext.name])
            subprocess.check_call(build_args,
                                  cwd=self.build_temp,
                                  env=env)
        else:
            super().build_extension(ext)


__all__ = ['CMakeBuildExt', 'CMakeExtension']
