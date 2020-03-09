import _VOI as impl


def entropy(*args, **kwargs):
    """
    :param str metric: default = 'euclidean'
    :param args:
    :param kwargs:
    :return:
    """
    metric = kwargs.get('metric') or 'euclidean'
    _impl = getattr(impl, '_'.join(['entropy', metric]))
    return _impl(*args, **kwargs)
