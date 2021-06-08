import numpy as np

BIT_DEPTH = 12


def rescale(arr):
    np.multiply(arr, 1 << BIT_DEPTH, out=arr)
