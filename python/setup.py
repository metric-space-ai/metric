import setuptools
from os import path
import shutil
import sys
from setuptools.command.sdist import sdist


HERE = path.abspath(path.dirname(__file__))
sys.path.insert(0, HERE)

from cmake_ext import CMakeExtension, CMakeBuildExt


class MetricSdist(sdist):
    def make_release_tree(self, base_dir, files):
        super().make_release_tree(base_dir, files)

        source_metric = path.abspath(path.join(HERE, '..', 'metric'))
        target_metric = path.join(base_dir, 'metric_cpp', 'metric')
        shutil.copytree(
            source_metric,
            target_metric,
            ignore=shutil.ignore_patterns('*.md', 'README_old.md')
        )


setuptools.setup(
    ext_modules=[CMakeExtension('all', output_dir=path.join('metric', '_impl'))],
    cmdclass={'build_ext': CMakeBuildExt, 'sdist': MetricSdist}
)
