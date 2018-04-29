import rocksdb
import sys
import struct


def main():
    dbName, outBinary = sys.argv[1], sys.argv[2]
    opts = rocksdb.Options()
    opts.max_open_files = -1
    db = rocksdb.DB(dbName, opts)
    kviter = db.iteritems()
    kviter.seek_to_first()

    f = open(outBinary, 'wb')
    for k, v in kviter:
        f.write(struct.pack('<H', len(k)))
        f.write(k)

        f.write(struct.pack('<H', len(v)))
        f.write(v)

    f.close()

if __name__ == '__main__':
    main()
