package main

import (
	"fmt"
	"log"
	"strconv"

	"github.com/boltdb/bolt"
	"spa/kv/util"
)

var bDB *bolt.DB

func init() {
	timeStart := util.Microsecond()
	bDB, _ = bolt.Open("/Users/wolf4j/testDB/boltdb", 0600, nil)
	tx, err := bDB.Begin(true)
	if err != nil {
		log.Fatal(err)
	}
	defer tx.Commit()

	/*
	  when first use, we need create bucket for db,
	  when the bucket create success, we needn't to create repeatedly, otherwise it will be wrong.
	*/

	//if _, err := tx.CreateBucket([]byte("copernicus")); err != nil {
	//	log.Fatal(err)
	//}
	fmt.Println("db init time is :", util.Microsecond()-timeStart)
}

//insert data and use cache
func insert(nums int) {
	timeStart := util.Microsecond()
	for i := 0; i < nums; i++ {
		key := []byte("key_" + strconv.Itoa(i))     // 10 bytes
		value := []byte("value_" + strconv.Itoa(i)) //12 bytes
		bDB.Update(func(tx *bolt.Tx) error {
			b := tx.Bucket([]byte("copernicus"))
			err := b.Put(key, value)
			return err
		})
	}
	fmt.Println("insert time is :", util.Microsecond()-timeStart)
}

// select data and use cache
func selects(nums int) {
	t := util.Microsecond()
	for i := 0; i < nums; i++ {
		key := []byte("key_" + strconv.Itoa(i))
		bDB.View(func(tx *bolt.Tx) error {
			b := tx.Bucket([]byte("copernicus"))
			if r := b.Get(key); r == nil {
				fmt.Println("select value not found the index:", i)
			}
			return nil
		})
	}
	fmt.Println("select time is :", util.Microsecond()-t)
}

// delete data and use cache
func delete(nums int) {
	t := util.Microsecond()
	for i := 0; i < nums; i++ {
		key := []byte("key_" + strconv.Itoa(i))
		bDB.Update(func(tx *bolt.Tx) error {
			b := tx.Bucket([]byte("copernicus"))
			if err := b.Delete(key); err != nil {
				fmt.Println(err)
			}
			return nil
		})
	}
	fmt.Println("delete time is :", util.Microsecond()-t)
}

func main() {
	nums := 10000000
	insert(nums)
	//the Select and Delete operating needs to be done after the Insert operation
	//selects(nums)
	//delete(nums)

}
