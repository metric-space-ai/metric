from metric.exceptions import OptionalDependencyError

try:
    import metric._impl.mgc as impl
except ModuleNotFoundError as exc:
    if exc.name is not None and not exc.name.startswith("metric._impl"):
        # A genuinely unrelated missing dependency (e.g. a transitive import
        # inside the native binding) must surface unchanged.
        raise
    raise OptionalDependencyError(
        "native correlation binding metric._impl.mgc is unavailable; "
        "correlation is an adapter boundary until the native binding is promoted"
    ) from None

from metric.distance import Euclidean


def MGC(metric1=Euclidean(), metric2=Euclidean()) -> object:
    """ Multiscale Graph Correlation factory

    :param metric1: metric for first container
    :param metric2: metric for second container
    :return: instance of MGC
    """
    return impl.create_mgc(metric1=metric1, metric2=metric2)


MGC_direct = impl.MGC_direct
