package main

import (
	"fmt"
	"sync"
	"strconv"

	"github.com/syndtr/goleveldb/leveldb"
	"spa/kv/util"
)

var (
	lDB    *leveldb.DB
	batch = 10000
	group = 50
)

func init() {
	timeStart := util.Microsecond()
	// if the OpenFile path not exist, will be automatically created,
	// but does not support the use of absolute path
	lDB, _ = leveldb.OpenFile("Users/wolf4j/testDB/leveldb", nil)
	//if err != nil {
	//	panic("haha")
	//}
	//defer db.Close()
	fmt.Println("db init time is :", util.Microsecond()-timeStart)
}

func Insert(nums int) {
	var wg sync.WaitGroup
	timeStart := util.Microsecond()
	p := nums / group
	p += group - p%group
	for g := 0; g < group; g++ {
		ss := g*p + 1
		se := (g + 1) * p
		if se > nums {
			se = nums
		}
		wg.Add(1)
		go func(start, end int) {
			b := new(leveldb.Batch)
			for i := start; i <= end; i++ {
				key := []byte("key_" + strconv.Itoa(i))     //10 bytes
				value := []byte("value_" + strconv.Itoa(i)) //12 bytes
				b.Put(key, value)
				if i%batch == 0 {
					lDB.Write(b, nil)
					b.Reset()
				}
			}
			lDB.Write(b, nil)
			b.Reset()
			wg.Done()
		}(ss, se)
	}
	wg.Wait()
	fmt.Println("Insert data time is :", util.Microsecond()-timeStart)
}

func Select(nums int) {
	var wg sync.WaitGroup
	timeStart := util.Microsecond()
	p := nums / group
	p += group - p%group
	for g := 0; g < group; g++ {
		ss := g*p + 1
		se := (g + 1) * p
		if se > nums {
			se = nums
		}
		wg.Add(1)
		go func(start, end int) {
			for i := start; i <= end; i++ {
				key := []byte("key_" + strconv.Itoa(i))
				if r, _ := lDB.Get(key, nil); r == nil {
					fmt.Println("select value not found the index:", i)
				}
			}
			wg.Done()
		}(ss, se)
	}
	wg.Wait()
	fmt.Println("Select data time is :", util.Microsecond()-timeStart)
}

func Delete(nums int) {
	var wg sync.WaitGroup
	timeStart := util.Microsecond()
	p := nums / group
	p += group - p%group
	for g := 0; g < group; g++ {
		ss := g*p + 1
		se := (g + 1) * p
		if se > nums {
			se = nums
		}
		wg.Add(1)
		go func(start, end int) {
			b := new(leveldb.Batch)
			for i := start; i <= end; i++ {
				key := []byte("key_" + strconv.Itoa(i))
				b.Delete(key)
				if i%batch == 0 {
					lDB.Write(b, nil)
					b.Reset()
				}
			}
			lDB.Write(b, nil)
			b.Reset()
			wg.Done()
		}(ss, se)
	}
	wg.Wait()
	fmt.Println("Delete data time is :", util.Microsecond()-timeStart)
}

func main() {
	nums := 10000000
	//Insert(nums)
	//Select(nums)
	Delete(nums)

}
