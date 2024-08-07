import numpy as np
import numba as nb
import time

from functools import reduce

data = np.fromfile("data.int32", np.int32)


@nb.jit
def just_a_loop(data):
    out = 0
    for x in data:
        out += x * x
    return out


square = nb.jit(lambda x: x * x)
add = nb.jit(lambda x, y: x + y)

@nb.jit
def just_functional(data):
    return reduce(add, map(square, data))


for repeat in range(10):
    start = time.perf_counter()

    result = just_functional(data)

    stop = time.perf_counter()
    print(f"{result = } ({stop - start:.8f} seconds)")
