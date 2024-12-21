import numpy as np
import os
from scipy.special import softmax

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
    # lora_transformer
    m, r, batch = 8, 16,16
    lm_h = 16
    directory = "E:/chip_simulator/matrices_transformer"
    os.makedirs(directory, exist_ok=True)

    generate_and_save_matrices(m, r, batch, lm_h,directory)
    A, B, C,LM,LMC = read_matrices(directory)

    first_out= ((A @ B )+ C) #(length, batch)
    first_out = softmax(first_out, axis=0)
    
    np.savetxt(f"{directory}/output.txt", (LM @first_out)+LMC, fmt="%.10f")
