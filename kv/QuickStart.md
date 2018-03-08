
# boltDB

## install

```
go get github.com/boltdb/bolt
```

## use

1. First, we need to create bucket, so please open comments of 28 to 30 lines.
2. In main, we only open insert method and run.
3. Using above the method, we can test select and delete, but need to close 28 to 30 lines comments, 
   otherwise it will be wrong.
   
# levelDB

## install

```
go get github.com/syndtr/goleveldb/leveldb
```

## use 

In order to test `insert` `select` and `delete` respectively, we can choose to annotate other method in main().
