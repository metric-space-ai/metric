import metric._impl.koc as impl
from metric._common import cpp_implementation_router


@cpp_implementation_router(impl=impl, graph='Grid6', metric='Euclidean')
def KOC_factory(*args, **kwargs):
    pass
