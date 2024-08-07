import numpy as np
import time

from functools import reduce
from operator import add

data = np.fromfile("data.int32", np.int32).tolist()

def do_everything(data):

    out = 0
    for x in data:
        out += x * x
    return out

square = lambda x: x * x

for repeat in range(10):
    start = time.perf_counter()

    result = reduce(add, map(square, data))

    stop = time.perf_counter()
    print(f"{result = } ({stop - start:.8f} seconds)")
