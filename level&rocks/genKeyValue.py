#!/usr/bin/python
import random
import struct


def generateKey():
    bytes_length = 40
    return bytes_length, random.getrandbits(bytes_length << 3)


def generateValue():
    bytes_length = random.randint(30, 101)
    return bytes_length, random.getrandbits(bytes_length << 3)


def doGen(kvTotalSize, seed=None):
    random.seed(seed)
    with open("trx.bin", 'wb+') as f:
        while f.tell() < kvTotalSize:
            key_len, key = generateKey()
            value_len, value = generateValue()
            f.write(struct.pack('<H', key_len))
            f.write(key.to_bytes(key_len, "big"))

            f.write(struct.pack('<H', value_len))
            f.write(value.to_bytes(value_len, "big"))

if __name__ == '__main__':
    import sys
    doGen(int(sys.argv[1]))
