import numpy as np
import os


def read_matrix(file_path):
    return np.loadtxt(file_path)


def save_matrix(matrix, file_path):
    np.savetxt(file_path, matrix, fmt="%.10f")


def main():
    base_path = "E:/chip_simulator/matrices_medium"
    file1 = os.path.join(base_path, "sim_matrix_output.txt")
    file2 = os.path.join(base_path, "output.txt")
    output_file = os.path.join(base_path, "difference.txt")

    matrix1 = read_matrix(file1)
    matrix2 = read_matrix(file2)

    result_matrix = matrix1 - matrix2

    save_matrix(result_matrix, output_file)


if __name__ == "__main__":
    main()
