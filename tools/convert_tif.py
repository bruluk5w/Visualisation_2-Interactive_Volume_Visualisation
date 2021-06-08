import os.path
import re
import struct
from enum import Enum
import pydicom
import numpy as np
import tifffile
from scipy.ndimage import zoom

from common import rescale

pattern = re.compile(r'/[0-9]{3,}/', re.ASCII)

SRC_PATH = r'./../../BrowlerEngine/Assets/raw/'
OUT_PATH = r'./../../BrowlerEngine/Assets/DataSets'
OUT_EXT = r'.dat'

DATASET_NAME = r'VHF-Head'

xy = 0.02148 / 0.04715
SAMPLE_DIM = (1, 1, 1)  # x, y, z
ROTATE_X = 1


class DATA_SET_TYPE(Enum):
    TIFF_SLICES = 0
    DICOM_SLICES = 1


TYPE = DATA_SET_TYPE.DICOM_SLICES


def load_tiff_list(file_list):
    return tifffile.imread(file_list).astype(np.float32)


def load_dicom_list(file_list):
    zDim = len(file_list)
    arr = None
    for i, p in enumerate(file_list):
        d = pydicom.read_file(p)
        if arr is None:
            arr = np.ndarray((zDim, *d.pixel_array.shape), dtype=np.float32)
        arr[i, :, :] = d.pixel_array

    return arr




load_functions = {
    DATA_SET_TYPE.TIFF_SLICES: load_tiff_list,
    DATA_SET_TYPE.DICOM_SLICES: load_dicom_list,
}


def main():
    folder = os.path.join(SRC_PATH, DATASET_NAME);
    assert os.path.isdir(folder)
    for dirpath, dirnames, filenames in os.walk(folder):

        files, ext = zip(*sorted((os.path.join(dirpath, f), ext) for f, ext in (os.path.splitext(filename) for filename in filenames) if len(f) > 2 and f[-3:].isnumeric()))
        assert all(ext[i] == ext[i+1] for i in range(len(ext) - 1)), "Not the same file extension"
        files = [file + e for file, e, in zip(files, ext)]
        arr = load_functions[TYPE](files)  # type: np.ndarray

        dims = np.array(list(reversed(SAMPLE_DIM)))
        min_dim = dims.min(axis=None)
        scale = dims / min_dim
        max = arr.max(axis=None)
        min = arr.min(axis=None)
        if not all(dims[i] == dims[i+1] for i in range(len(dims) - 1)):
            res = np.ndarray((scale * np.array(arr.shape)).astype(np.int32), dtype=np.float32)
            zoom(arr, scale, output=res)
        else:
            res = arr
        np.subtract(res, min, out=res)
        np.divide(res, max - min, out=res)
        rescale(res)
        res[0, :, :] = 0
        res[:, 0, :] = 0
        res[:, :, 0] = 0
        res[-1, :, :] = 0
        res[:, -1, :] = 0
        res[:, :, -1] = 0

        if ROTATE_X:
            res = np.rot90(res, ROTATE_X, (0, 1))

        out_path = os.path.join(OUT_PATH, DATASET_NAME + OUT_EXT)
        with open(out_path, 'wb') as out:
            out.write(struct.pack('<H', res.shape[2]))
            out.write(struct.pack('<H', res.shape[1]))
            out.write(struct.pack('<H', res.shape[0]))

            out.write(np.ascontiguousarray(res, np.dtype('<i2')).tobytes())

        print("Written to {}".format(out_path))


if __name__ == '__main__':
    main()
