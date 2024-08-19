# 2024-08-19-python-school-setting-stage

These are the commands we used (my prompt is `%`):

Making data:

```bash
% python -c 'import numpy as np; np.random.poisson(5, 10000000).tofile("data.int32")'
```

Compiling baby-python:

```bash
% c++ -std=c++11 -O3 baby-python.cpp -o baby-python
```

Running it in baby-python:

```bash
% ./baby-python data=data.int32
                     num = -123        add(x, x)   get(lst, i)   map(f, lst)
               oo    lst = [1, 2, 3]   mul(x, x)   len(lst)      reduce(f, lst)
. . . __/\_/\_/`'    f = def(x) single-expr   f = def(x, y) { ... ; last-expr }

>> data
[6, 2, 4, 3, 7, 6, 3, 5, 10, 1, 5, 10, 6, 3, 6, 4, 6, 2, 2, 5, 13, 7, 5, 3, 9...
(5.166e-06 seconds)
>> square = def(x) mul(x, x)
<user-defined function>
(4.042e-06 seconds)
>> result = reduce(add, map(square, data))
29961466
(0.454636 seconds)
```

Running it in Python:

```bash
% python speed-tests/using-python.py
```

(and toggle the line that switches between for loops and functional).

Compiling and running it in C++:

```bash
% c++ -std=c++14 -O3 speed-tests/just-functional.cpp -o just-functional
% c++ -std=c++14 -O3 speed-tests/just-a-loop.cpp -o just-a-loop
% ./just-functional
% ./just-a-loop    
```

You should see that baby-python is about 10 times slower than Python, and Python is orders of magnitude slower than C++.

[Check out the code for baby-python.cpp!](https://github.com/jpivarski-talks/2024-08-19-python-school-setting-stage/blob/main/baby-python.cpp)

[Check out the Python-hacking notebook!](https://github.com/jpivarski-talks/2024-08-19-python-school-setting-stage/blob/main/snake-eats-its-tail.ipynb) (We didn't do this one live. If you're on a Mac, replace `libc.so` with `libSystem.dylib`.)
