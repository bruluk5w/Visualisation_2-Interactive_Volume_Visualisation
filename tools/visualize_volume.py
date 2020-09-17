import numpy as np
import struct
from plotly import graph_objects as go

INPUT = r'E:\repos\browler_engine\BrowlerEngine\Assets\DataSets\Sphere.dat'


def main():
    with open(INPUT, 'rb') as input:
        size_x, size_y, size_z = struct.unpack('<HHH', input.read(struct.calcsize('<HHH')))

        buffer = input.read(size_x * size_y * size_z * 2)
        volume = np.ndarray((size_z, size_y, size_x), np.dtype('<i2'), buffer=buffer)

    Z, Y, X = np.mgrid[0:size_z, 0:size_y, 0:size_x]

    fig = go.Figure(data=go.Volume(
        x=X.flatten(), y=Y.flatten(), z=Z.flatten(),
        value=volume.flatten(),
        isomin=0.0,
        isomax=2**10,
        opacity=0.1,  # needs to be small to see through all surfaces
        surface_count=4  # needs to be a large number for good volume rendering
    ))
    fig.show()


if __name__ == '__main__':
    main()
