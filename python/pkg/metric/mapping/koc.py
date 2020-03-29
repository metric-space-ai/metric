import _koc as impl
from metric._common import cpp_implementation_router


@cpp_implementation_router(impl=impl, graph='grid6', metric='euclidean')
def KOC_factory(*args, **kwargs):
    pass
