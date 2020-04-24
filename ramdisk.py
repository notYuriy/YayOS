import os
import struct
import ntpath


def writeHeader(path: str, ftype: int, size: int, outputStream):
    outputStream.write(struct.pack(
        '<QQ256s', ftype, size, ntpath.basename(path)))


def writeFile(path: str, outputStream):
    if os.path.isfile(path):
        writeHeader(path, os.fstat(path).size, 0, outputStream)
        with os.open(path, 'rb') as inputfile:
            contents = inputfile.read()
            contentsLen = ((len(inputfile) + 15) // 16) * 16
            contents.append(' ' * (contentsLen - len(contents)))
            outputStream.write(contents)
    elif os.path.isdir(path):
        entries = else:
        raise AssertionError('file type not supported')
