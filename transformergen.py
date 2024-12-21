import numpy as np
import os
from scipy.special import softmax

def generate_and_save_matrices(lq, lk,  channel,directory):
    # Generate random matrices
    Q = np.random.rand(channel, lq)
    K = np.random.rand(lk, channel)
    V = K.transpose()
    
    Wq = np.random.rand(channel, channel)/10.0
    biasq = np.random.rand(channel, lq)
    Wk = np.random.rand(channel, channel)/10.0
    biask = np.random.rand(lk, channel)
    zeros = np.zeros((channel, channel))
    Wv = Wk.transpose()
    biasv = biask.transpose()

    # Save matrices to files
    np.savetxt(f"{directory}/matrix_Q.txt", Q, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_K.txt", K, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_V.txt", V, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_Wq.txt", Wq, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_Wk.txt", Wk, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_biasq.txt", biasq, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_biask.txt", biask, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_Wv.txt", Wv, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_biasv.txt", biasv, fmt="%.10f")
    np.savetxt(f"{directory}/matrix_zeros.txt", zeros, fmt="%.10f")


def read_matrices(directory):
    # Read matrices from files
    Q = np.loadtxt(f"{directory}/matrix_Q.txt")
    K = np.loadtxt(f"{directory}/matrix_K.txt")
    V = np.loadtxt(f"{directory}/matrix_V.txt")
    Wq = np.loadtxt(f"{directory}/matrix_Wq.txt")
    Wk = np.loadtxt(f"{directory}/matrix_Wk.txt")
    biasq = np.loadtxt(f"{directory}/matrix_biasq.txt")
    biask = np.loadtxt(f"{directory}/matrix_biask.txt")
    Wv = np.loadtxt(f"{directory}/matrix_Wv.txt")
    biasv = np.loadtxt(f"{directory}/matrix_biasv.txt")
    
    return Q, K, V, Wq, Wk, Wv, biasq, biask,  biasv


if __name__ == "__main__":
    # Example dimensions
    # lora_transformer
    lq=16
    lk=8
    channel = 16
    directory = "E:/chip_simulator/matrices_transformer"
    os.makedirs(directory, exist_ok=True)

    generate_and_save_matrices(lq, lk, channel, directory)
    Q, KT, V, Wq, Wk, Wv, biasq, biask, biasv = read_matrices(directory)

    # Compute the output of the transformer
    q = Wq @ Q + biasq
    np.savetxt(f"{directory}/qin.txt", q, fmt="%.10f")
    
    kT = KT @ Wk + biask
    np.savetxt(f"{directory}/kTin.txt", kT, fmt="%.10f")
    v = Wv @ V + biasv
    np.savetxt(f"{directory}/vin.txt", v, fmt="%.10f")
    # import pdb;pdb.set_trace()
    attn = kT @ q/np.sqrt(channel)
    soft_attn = softmax(kT @ q/np.sqrt(channel), axis=0)
    np.savetxt(f"{directory}/attn.txt", attn, fmt="%.10f")
    np.savetxt(f"{directory}/soft_attn.txt", soft_attn, fmt="%.10f")
    out = v @ soft_attn
    np.savetxt(f"{directory}/output.txt", out, fmt="%.10f")
