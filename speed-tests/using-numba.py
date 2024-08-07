import numpy as np
import numba as nb
import time

data = np.fromfile("data.int32", np.int32)

@nb.jit
def do_everything(data):
    out = 0
    for x in data:
        out += x * x
    return out

for repeat in range(10):
    start = time.perf_counter()

    result = do_everything(data)

    stop = time.perf_counter()
    print(f"{result = } ({stop - start:.8f} seconds)")
