import _correlation as impl
from metric._common import cpp_implementation_router


@cpp_implementation_router(impl=impl, metric1='euclidean', metric2='euclidean')
def MGC():
    pass


MGC_direct = impl.MGC_direct