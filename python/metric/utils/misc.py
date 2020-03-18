from time import time


def time_n_log(*funcs, **func_names):
    """ Decorator to measure execution time of decorated function
    if function passed as keyword parameter its name will be taken as function name

    :param funcs: optional, default usage as decorator
    :param func_names: optional, usage in case of lambda function
    :return:
    """
    def wrapper(*_args, **_kwargs):
        if funcs:
            func = funcs[0]
            func_name = func.__name__
        else:
            func_name, func = func_names.popitem()
        t1 = time()
        result = func(*_args, **_kwargs)
        t2 = time()
        print(f'{func_name} result: {result} (Time = {(t2 - t1) * 1000:.2f} ms)')
    return wrapper
