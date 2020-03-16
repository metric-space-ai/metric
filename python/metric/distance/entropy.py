import _voi as impl
from metric._common import cpp_implementation_router


@cpp_implementation_router(impl=impl, metric='euclidean')
def entropy(*args, **kwargs):
    """
    """
    pass
