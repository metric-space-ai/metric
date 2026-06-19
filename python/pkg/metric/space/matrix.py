try:
    import metric._impl.space as impl
except ModuleNotFoundError:
    impl = None

try:
    from metric._impl.standards import Euclidean
except ModuleNotFoundError:
    Euclidean = None


def Matrix(data=None, metric=None) -> object:
    """ Matrix factory

    :param data: container with items or one item
    :type data: :class:`numpy.array`
    :param callable metric: metric for container
    :return: instance of Matrix_<Metric>
    """
    if metric is None:
        if Euclidean is None:
            raise RuntimeError(
                "metric.space.Matrix requires a metric argument in the core wheel; "
                "the default Euclidean binding is available only in the full build"
            )
        metric = Euclidean()
    if impl is None:
        raise RuntimeError("metric.space.Matrix requires the compiled space extension")

    if data is None:
        return impl.create_matrix(metric=metric)
    shape = getattr(data, "shape", None)
    if shape is not None and len(shape) > 1:
        return impl.create_matrix(items=data, metric=metric)
    if shape is None and data and hasattr(data[0], "__len__"):
        return impl.create_matrix(items=data, metric=metric)
    return impl.create_matrix(item=data, metric=metric)
