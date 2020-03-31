import metric._impl.space as impl
from metric._impl.distance import Euclidean


def Matrix(data=None, metric=Euclidean()) -> object:
    """ Matrix factory

    :param data: container with data
    :param metric: metric for second container
    :return: instance of Matrix_<Metric>
    """
    return impl.create_matrix(data=data, metric=metric) if data else impl.create_matrix(metric=metric)
