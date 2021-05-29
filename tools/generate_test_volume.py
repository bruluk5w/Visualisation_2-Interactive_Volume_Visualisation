from enum import Enum
import numpy as np
import struct


class Shape(Enum):
    SPHERE = 0
    BOX = 1


SIZE_X = 128
SIZE_Y = 64
SIZE_Z = 32

SHAPE = Shape.SPHERE

OUTPUT = r'D:\repos\browler_engine\BrowlerEngine\Assets\DataSets\Sphere.dat'

BIT_DEPTH = 10


def main():
    volume = np.ndarray((SIZE_Z, SIZE_Y, SIZE_X))
    if SHAPE == Shape.SPHERE:
        make_sphere(volume)
    elif SHAPE == Shape.BOX:
        make_box(volume)

    with open(OUTPUT, 'wb') as out:
        out.write(struct.pack('<H', SIZE_X))
        out.write(struct.pack('<H', SIZE_Y))
        out.write(struct.pack('<H', SIZE_Z))

        out.write(np.ascontiguousarray(volume, np.dtype('<i2')).tobytes())


def make_sphere(v: np.ndarray):
    dim = np.array((SIZE_X, SIZE_Y, SIZE_Z), np.float32)
    center_x = SIZE_X * 0.5
    center_y = SIZE_Y * 0.5
    center_z = SIZE_Z * 0.5
    for x in range(SIZE_X):
        x_dist = (x - center_x) / (SIZE_X * 0.5)
        v[:, :, x] = x_dist * x_dist

    for y in range(SIZE_Y):
        y_dist = (y - center_y) / (SIZE_Y * 0.5)
        v[:, y, :] += y_dist * y_dist

    for z in range(SIZE_Z):
        z_dist = (z - center_z) / (SIZE_Z * 0.5)
        v[z, :, :] += z_dist * z_dist

    np.sqrt(v, v)
    np.subtract(1, v, v)
    np.multiply(v, 1 << BIT_DEPTH, v)


def make_box(v: np.ndarray):
    dim = np.array((SIZE_X, SIZE_Y, SIZE_Z), np.float32)
    center_x = SIZE_X * 0.5
    center_y = SIZE_Y * 0.5
    center_z = SIZE_Z * 0.5
    for x in range(SIZE_X):
        x_dist = abs((x - center_x) / (SIZE_X * 0.5))
        v[:, :, x] = x_dist

    for y in range(SIZE_Y):
        y_dist = abs((y - center_y) / (SIZE_Y * 0.5))
        mask = v[:, y, :] < y_dist
        v[:, y, :][mask] = y_dist

    for z in range(SIZE_Z):
        z_dist = abs((z - center_z) / (SIZE_Z * 0.5))
        mask = v[z, :, :] < z_dist
        v[z, :, :][mask] = z_dist

    np.subtract(1, v, v)
    np.multiply(v, 1 << BIT_DEPTH, v)


if __name__ == '__main__':
    main()
