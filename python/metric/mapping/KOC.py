import _KOC as impl


def KOC_factory(*args, **kwargs):
    """
    :param str metric: default = 'euclidean'
    :param str graph: default = 'grid6'
    :param args:
    :param kwargs:
    :return: factory
    """
    metric = kwargs.get('metric') or 'euclidean'
    graph = kwargs.get('graph') or 'grid6'
    _impl = getattr(impl, '_'.join(['KOC_factory', graph, metric]))
    return _impl(*args, **kwargs)
