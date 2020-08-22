import sys
import math
import struct
from PIL import Image
import numpy

data = []

with open(sys.argv[1], "rb") as f:
    data = f.read()

width, height = struct.unpack("II", data[:8])

data = struct.unpack("{}f".format(width*height*3), data[8:])

data = numpy.array(data, dtype=numpy.float).reshape(height, width, 3) 

mVal = data.flatten()
mVal = numpy.unique(mVal)
mVal.sort()
print(mVal)
mVal = mVal[-2]
miVal = data.min()

data = 255 * ((data - miVal) / (mVal - miVal))

data = data.astype(numpy.uint8)

print(mVal, miVal)
print(data.shape)

img = Image.fromarray(data, 'RGB')
#img.show()
img.save("trashmouse.png")
