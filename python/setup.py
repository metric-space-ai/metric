import setuptools
from cmake_setuptools import CMakeExtension, CMakeBuildExt
setuptools.setup(
     name='metric',
     version='0.0.1',
     ext_modules=[CMakeExtension('metric_py')],
     cmdclass={'build_ext': CMakeBuildExt},
     author="Ilya Gavrilov",
     author_email="gilyav@gmail.com",
     description="TimeSwipe python3 module",
     long_description="pydoc metric",
     long_description_content_type="text/markdown",
     url="https://github.com/panda-official/metric",
     packages=setuptools.find_packages(),
#data_files=['../modules/*'],
     install_requires = ['cmake_setuptools'],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: MPL 2.0 License",
         "Operating System :: OS Independent",
     ],
 )
