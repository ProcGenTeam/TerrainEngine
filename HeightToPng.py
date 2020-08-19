from PIL import Image
import sys
import struct
import numpy


# This is a dumb script

lS = 1024 + 32 * 2

filename = sys.argv[1]
fileData = []

with open(filename, "rb") as f:
    fileData = f.read()

fileLength = len(fileData) // 4
fileData = struct.unpack(str(fileLength)+"f", fileData)

maxVal = max(fileData)

print(maxVal)

fileData = numpy.array(fileData, dtype=numpy.float).reshape(lS,lS) 

print(fileData[9][234])#9 * 4096 + 1234

fileData = fileData * 65535#255#// (maxVal // 256)

im = Image.fromarray(fileData.astype(numpy.uint32), 'I')
#im.show()
im.save("meme.png")