from functools import wraps


def build_name(func, params) -> str:
    """ if it is not string use class name (i.e. int -> 'int', SomeClass -> 'SomeClass')

    :param func:
    :param params:
    :return:
    """
    params = [param if isinstance(param, str)
              else param.__name__ if isinstance(param, type)
              else type(param).__name__
              for param in params]
    prefix = func.__name__
    return '_'.join([prefix, *params])


def cpp_implementation_router(impl, **template_parameters):
    def concrete_wrapper(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            params = [kwargs.pop(k, v) for k, v in template_parameters.items()]
            return getattr(impl, build_name(func, params))(*args, **kwargs)
        return wrapper
    return concrete_wrapper


def factory_with_default(**template_parameters):
    def concrete_wrapper(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            return func(*args, **{**template_parameters, **kwargs})
        return wrapper
    return concrete_wrapper
