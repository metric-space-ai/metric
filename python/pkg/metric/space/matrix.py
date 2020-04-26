import metric._impl.space as impl
from metric._impl.standards import Euclidean


def Matrix(data=None, metric=Euclidean()) -> object:
    """ Matrix factory

    :param data: container with items or one item
    :type data: :class:`numpy.array`
    :param callable metric: metric for container
    :return: instance of Matrix_<Metric>
    """
    if data is None:
        return impl.create_matrix(metric=metric)
    if len(data.shape) > 1:
        return impl.create_matrix(items=data, metric=metric)
    return impl.create_matrix(item=data, metric=metric)
