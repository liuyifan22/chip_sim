import numpy as np
import os


def generate_and_save_matrices(m, r, n, lm_h,directory):
    # Generate random matrices
    A = np.random.rand(m, r)
    B = np.random.rand(r, n)
    C = np.random.rand(m, n)
    LM = np.random.rand(lm_h, m)
    LMC = np.random.rand(lm_h, n)

    # Save matrices to files
    np.savetxt(f"{directory}/matrix_A.txt", A, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_B.txt", B, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_C.txt", C, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_LM.txt", LM, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_LMC.txt", LMC, fmt="%.10f")


def read_matrices(directory):
    # Read matrices from files
    A = np.loadtxt(f"{directory}/matrix_A.txt")
    B = np.loadtxt(f"{directory}/matrix_B.txt")
    C = np.loadtxt(f"{directory}/matrix_C.txt")
    LM = np.loadtxt(f"{directory}/matrix_LM.txt")
    LMC = np.loadtxt(f"{directory}/matrix_LMC.txt")
    return A, B, C,LM,LMC


if __name__ == "__main__":
    # Example dimensions
    m, r, n = 15, 12, 14
    lm_h = 17
    directory = "E:/chip_simulator/matrices_continue"
    os.makedirs(directory, exist_ok=True)

    # generate_and_save_matrices(m, r, n, lm_h,directory)
    A, B, C,LM,LMC = read_matrices(directory)
    np.savetxt(f"{directory}/output_mid.txt", (A @ B) + C, fmt="%.10f")
    np.savetxt(f"{directory}/output.txt", (LM @((A @ B )+ C))+LMC, fmt="%.10f")
