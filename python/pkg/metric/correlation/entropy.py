from functools import partial
from metric._impl import entropy as impl
from metric._impl.distance import Euclidean


class Entropy:
    def __init__(self, metric=Euclidean(), k=7, p=70, exp=False):
        self.metric = metric
        self.entropy_impl = partial(impl.entropy, metric=metric, k=k, p=p, exp=exp)
        self.estimate_impl = partial(impl.estimate, entropy=self.entropy_impl)

    def __call__(self, *args, **kwargs) -> float:
        return self.entropy_impl(*args, **kwargs)

    def estimate(self, sample_size=250, threshold=0.05, max_iterations=1000) -> float:
        return self.estimate_impl(sample_size=sample_size, threshold=threshold, max_iterations=max_iterations)

    def __str__(self):
        return "{cls}(metric={metric})".format(cls=self.__class__.__name__, metric=self.metric)
