import _KOC as impl
from metric._common import cpp_implementation_router


@cpp_implementation_router(impl=impl, metric='euclidean', graph='grid6')
def KOC_factory(*args, **kwargs):
    pass
