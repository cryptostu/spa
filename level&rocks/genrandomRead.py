import sys
import random
import struct

if __name__ == '__main__':
    totalSize = 1 << 30
    with open("trxread.bin."+sys.argv[1], 'wb+') as f:
        while f.tell() < totalSize:
            key = random.randint(1, int(sys.argv[1]) - 1)
            f.write(struct.pack('<H', 40))
            f.write(key.to_bytes(40, "big"))
            f.write(struct.pack('<H', 0))


