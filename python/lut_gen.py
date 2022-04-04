import numpy as np
import matplotlib.pyplot as plt

MAX_VAL = 255

R_BOUNDS = (0.0431 * MAX_VAL, 0.7231 * MAX_VAL)
G_BOUNDS = (0.0471 * MAX_VAL, 0.9960 * MAX_VAL)
B_BOUNDS = (0.0471 * MAX_VAL, 0.9960 * MAX_VAL)

r_vals = np.arange(0, MAX_VAL)
g_vals = np.arange(0, MAX_VAL)
b_vals = np.arange(0, MAX_VAL)

r_vals = np.round(np.interp(r_vals, [R_BOUNDS[0], R_BOUNDS[1]], [0, MAX_VAL]))
g_vals = np.round(np.interp(g_vals, [G_BOUNDS[0], G_BOUNDS[1]], [0, MAX_VAL]))
b_vals = np.round(np.interp(b_vals, [B_BOUNDS[0], B_BOUNDS[1]], [0, MAX_VAL]))

print("{", end='')
for x in r_vals:
    print(int(x), end=', ')
print("255};")

print("{", end='')
for x in g_vals:
    print(int(x), end=', ')
print("255};")

print("{", end='')
for x in b_vals:
    print(int(x), end=', ')
print("255};")
