import numpy as np
import time

from functools import reduce
from operator import add

data = np.fromfile("data.int32", np.int32).tolist()


def just_a_loop(data):
    out = 0
    for x in data:
        out += x * x
    return out


square = lambda x: x * x

def just_functional(data):
    return reduce(add, map(square, data))


for repeat in range(10):
    start = time.perf_counter()

    result = just_functional(data)

    stop = time.perf_counter()
    print(f"{result = } ({stop - start:.8f} seconds)")
