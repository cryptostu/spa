import rocksdb
import sys
import os
import re
import random


def generateKey():
    bytes_length = 40
    return random.getrandbits(bytes_length << 3).to_bytes(bytes_length, "big")

def generateValue():
    bytes_length = random.randint(30, 101)
    return random.getrandbits(bytes_length << 3).to_bytes(bytes_length, "big")


def main():
    kvTotalSize, dbName = sys.argv[1], sys.argv[2] + ".db"
    # kvcntleft = int(sys.argv[3])
    m = re.match(r'(\d+)G', kvTotalSize)
    if not m:
        print("invalid format for kv size")
        return 
    kvTotalSize = int(m.group(1)) << 30
    if os.path.exists(dbName):
        print("db already exists")
        return
    opts = rocksdb.Options()
    opts.create_if_missing = True
    opts.max_open_files = 10000
    opts.table_factory = rocksdb.BlockBasedTableFactory(
                filter_policy=rocksdb.BloomFilterPolicy(10),
                    block_cache=rocksdb.LRUCache(8<<20))
    db = rocksdb.DB(dbName, opts)
    opts.create_if_missing = True
    curkvSize = 0
    it = db.iterkeys()
    it.seek_to_last()
    last_key = it.get()
    seqi = int.from_bytes(last_key, "big")
    print(seqi)
    while curkvSize < kvTotalSize: # and kvcntleft:
        seqi += 1
        key = seqi.to_bytes(40, "big")
        value = generateValue()
        db.put(key, value)
        
        curkvSize += len(key) + len(value)
        #kvcntleft -= 1
    print("total number of keys: {}, total size in bytes: {}".format(seqi, curkvSize))


if __name__ == '__main__':
    main()
