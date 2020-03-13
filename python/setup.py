import setuptools
from cmake_setuptools import CMakeExtension, CMakeBuildExt
setuptools.setup(
     name='metric',
     version='0.0.1',
     ext_modules=[CMakeExtension('all')],
     cmdclass={'build_ext': CMakeBuildExt},
     author="Jura Gresko",
     author_email="juragresko@gmail.com",
     description="Metric python3 module",
     long_description="pydoc metric",
     long_description_content_type="text/markdown",
     url="https://github.com/panda-official/metric",
     packages=setuptools.find_packages(),
#     data_files=['metric.distance', 'metric.mapping'],
     install_requires=['cmake_setuptools'],
     classifiers=[
         "Programming Language :: Python :: 3",
         "License :: OSI Approved :: MPL 2.0 License",
         "Operating System :: OS Independent",
     ],
)
