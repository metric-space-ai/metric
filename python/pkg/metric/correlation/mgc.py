import metric._impl.mgc as impl
from metric._impl.standards import Euclidean


def MGC(metric1=Euclidean(), metric2=Euclidean()) -> object:
    """ Multiscale Graph Correlation factory

    :param metric1: metric for first container
    :param metric2: metric for second container
    :return: instance of MGC
    """
    return impl.create_mgc(metric1=metric1, metric2=metric2)


MGC_direct = impl.MGC_direct
