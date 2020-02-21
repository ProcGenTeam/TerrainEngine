from PIL import Image
import sys
import struct
import numpy


# This is a dumb script

filename = sys.argv[1]
fileData = []

with open(filename, "rb") as f:
    fileData = f.read()

fileLength = len(fileData) // 4
fileData = struct.unpack(str(fileLength)+"I", fileData)

maxVal = max(fileData)

print(maxVal)

fileData = numpy.array(fileData, dtype=numpy.uint32).reshape(4096,4096)

fileData = fileData // 256#// (maxVal // 256)

im = Image.fromarray(fileData, 'I')
#im.show()
im.save("meme.png")