from functools import wraps


def cpp_implementation_router(impl, **template_parameters):
    def concrete_wrapper(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            params = [kwargs.pop(k, v) for k, v in template_parameters.items()]
            prefix = func.__name__
            return getattr(impl, '_'.join([prefix, *params]))(*args, **kwargs)
        return wrapper
    return concrete_wrapper


def factory_with_default(**template_parameters):
    def concrete_wrapper(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            return func(*args, **{**template_parameters, **kwargs})
        return wrapper
    return concrete_wrapper
