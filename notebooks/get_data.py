import os
import numpy as np
matrix = np.loadtxt("E:\chip_simulator\\notebooks\\fc1.bias.txt")
max_element = np.max(matrix)
min_element = np.min(matrix)

print(f"Maximum element: {max_element}")
print(f"Minimum element: {min_element}")