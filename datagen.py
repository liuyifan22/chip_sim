import numpy as np
import os


def generate_and_save_matrices(m, r, n, directory):
    # Generate random matrices
    A = np.random.rand(m, r)
    B = np.random.rand(r, n)
    C = np.random.rand(m, n)

    # Save matrices to files
    np.savetxt(f"{directory}/matrix_A.txt", A, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_B.txt", B, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_C.txt", C, fmt="%.10f")


def read_matrices(directory):
    # Read matrices from files
    A = np.loadtxt(f"{directory}/matrix_A.txt")
    B = np.loadtxt(f"{directory}/matrix_B.txt")
    C = np.loadtxt(f"{directory}/matrix_C.txt")
    return A, B, C


if __name__ == "__main__":
    # Example dimensions
    m, r, n = 16, 8, 24
    directory = "E:/chip_simulator/matrices_medium"
    # os.makedirs(directory, exist_ok=True)

    generate_and_save_matrices(m, r, n, directory)
    A, B, C = read_matrices(directory)
    print("Matrix A:")
    print(A)
    print("Matrix B:")
    print(B)
    print("Matrix C:")
    print(C)
    print((A @ B + C).shape)
    np.savetxt(f"{directory}/output.txt", A @ B + C, fmt="%.10f")
