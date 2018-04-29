import rocksdb
import sys

def main():
    dbName = sys.argv[1] + ".db"
    db = rocksdb.DB(dbName, rocksdb.Options())
    db.iterkeys()
    it = db.iteritems()
    it.seek_to_first()
    for _ in range(100):
        print(it.get())
        next(it)

if __name__ == '__main__':
    main()
