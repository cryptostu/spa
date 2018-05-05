# leveldb VS rocksdb 

## 背景
本文的测试场景是模拟BCH中，验证交易的过程中使用DB的场景，这种场景可以总结为脉冲式的读写和删除操作，也就是说，大约每10分钟会触发一次大规模的读操作（交易验证），写操作（交易插入/更新/删除），所以，测试过程中会把读操作和写操作进行分开测试。

本文的测试细节较多，读者可以review测试代码，以及使用本测试代码和相似的软硬件测试环境对最终的测试结果进行验证。


## 测试环境

### 硬件环境
```
$ cat /proc/cpuinfo | grep 'model name' | uniq
model name      : Intel(R) Core(TM) i5-7500 CPU @ 3.40GHz

$ cat /proc/meminfo | grep MemTotal
MemTotal:       16316464 kB


$ sudo smartctl -a /dev/sda
smartctl 6.5 2016-01-24 r4214 [x86_64-linux-4.4.0-87-generic] (local build)
Copyright (C) 2002-16, Bruce Allen, Christian Franke, www.smartmontools.org

=== START OF INFORMATION SECTION ===
Device Model:     INTEL SSDSC2KW512G8
Serial Number:    BTLA750307HD512DGN
LU WWN Device Id: 5 5cd2e4 14ee7450c
Firmware Version: LHF002C
User Capacity:    512,110,190,592 bytes [512 GB]
Sector Size:      512 bytes logical/physical
Rotation Rate:    Solid State Device
Form Factor:      2.5 inches
Device is:        Not in smartctl database [for details use: -P showall]
ATA Version is:   ACS-3 (minor revision not indicated)
SATA Version is:  SATA 3.2, 6.0 Gb/s (current: 6.0 Gb/s)
Local Time is:    Tue Apr 24 17:06:05 2018 HKT
SMART support is: Available - device has SMART capability.
SMART support is: Enabled

=== START OF READ SMART DATA SECTION ===
SMART overall-health self-assessment test result: PASSED

General SMART Values:
Offline data collection status:  (0x00) Offline data collection activity
                                        was never started.
                                        Auto Offline Data Collection: Disabled.
Self-test execution status:      (   0) The previous self-test routine completed
                                        without error or no self-test has ever
                                        been run.
Total time to complete Offline
data collection:                (    0) seconds.
Offline data collection
capabilities:                    (0x53) SMART execute Offline immediate.
                                        Auto Offline data collection on/off support.
                                        Suspend Offline collection upon new
                                        command.
                                        No Offline surface scan supported.
                                        Self-test supported.
                                        No Conveyance Self-test supported.
                                        Selective Self-test supported.
SMART capabilities:            (0x0003) Saves SMART data before entering
                                        power-saving mode.
                                        Supports SMART auto save timer.
Error logging capability:        (0x01) Error logging supported.
                                        General Purpose Logging supported.
Short self-test routine
recommended polling time:        (   2) minutes.
Extended self-test routine
recommended polling time:        (  30) minutes.
SCT capabilities:              (0x003d) SCT Status supported.
                                        SCT Error Recovery Control supported.
                                        SCT Feature Control supported.
                                        SCT Data Table supported.

SMART Attributes Data Structure revision number: 1
Vendor Specific SMART Attributes with Thresholds:
ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
  5 Reallocated_Sector_Ct   0x0032   100   100   000    Old_age   Always       -       0
  9 Power_On_Hours          0x0032   100   100   000    Old_age   Always       -       1266
 12 Power_Cycle_Count       0x0032   100   100   000    Old_age   Always       -       12
170 Unknown_Attribute       0x0033   100   100   010    Pre-fail  Always       -       0
171 Unknown_Attribute       0x0032   100   100   000    Old_age   Always       -       0
172 Unknown_Attribute       0x0032   100   100   000    Old_age   Always       -       0
173 Unknown_Attribute       0x0033   098   098   005    Pre-fail  Always       -       150329491458
174 Unknown_Attribute       0x0032   100   100   000    Old_age   Always       -       2
183 Runtime_Bad_Block       0x0032   100   100   000    Old_age   Always       -       0
184 End-to-End_Error        0x0033   100   100   090    Pre-fail  Always       -       0
187 Reported_Uncorrect      0x0032   100   100   000    Old_age   Always       -       0
190 Airflow_Temperature_Cel 0x0032   033   052   000    Old_age   Always       -       33 (Min/Max 19/52)
192 Power-Off_Retract_Count 0x0032   100   100   000    Old_age   Always       -       2
199 UDMA_CRC_Error_Count    0x0032   100   100   000    Old_age   Always       -       0
225 Unknown_SSD_Attribute   0x0032   100   100   000    Old_age   Always       -       373785
226 Unknown_SSD_Attribute   0x0032   100   100   000    Old_age   Always       -       0
227 Unknown_SSD_Attribute   0x0032   100   100   000    Old_age   Always       -       0
228 Power-off_Retract_Count 0x0032   100   100   000    Old_age   Always       -       0
232 Available_Reservd_Space 0x0033   100   100   010    Pre-fail  Always       -       0
233 Media_Wearout_Indicator 0x0032   098   098   000    Old_age   Always       -       0
236 Unknown_Attribute       0x0032   099   099   000    Old_age   Always       -       0
241 Total_LBAs_Written      0x0032   100   100   000    Old_age   Always       -       373785
242 Total_LBAs_Read         0x0032   100   100   000    Old_age   Always       -       184275
249 Unknown_Attribute       0x0032   100   100   000    Old_age   Always       -       18778
252 Unknown_Attribute       0x0032   100   100   000    Old_age   Always       -       35

SMART Error Log Version: 1
No Errors Logged

SMART Self-test log structure revision number 1
No self-tests have been logged.  [To run self-tests, use: smartctl -t]

SMART Selective self-test log data structure revision number 1
 SPAN  MIN_LBA  MAX_LBA  CURRENT_TEST_STATUS
    1        0        0  Not_testing
    2        0        0  Not_testing
    3        0        0  Not_testing
    4        0        0  Not_testing
    5        0        0  Not_testing
Selective self-test flags (0x0):
  After scanning selected spans, do NOT read-scan remainder of disk.
If Selective self-test is pending on power-up, resume after 0 minute delay.

```


### 软件环境
```
$ lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 16.04.4 LTS
Release:        16.04
Codename:       xenial

rocksdb version: 5.13.0
leveldb version: Release 1.19
```

* 为了防止swap影响测试性能，测试过程中关闭了swap
* 为了防止内存过大，cache过多，测试过程中使用malloc分配了约13GB的内存，防止page cache占用过多内存 `./eatmem 13421772800`
* 使用python3
* 需要安装python-rocksdb
* 发起测试时，磁盘利用率约为55%，并在ssd上创建了LVM
* 使用-O2选项进行编译rocksdb和leveldb
* 安装libsnappy


### 小结

* 使用dd开启direct io，4K顺序读的性能只有120MB/s, 使用1M顺序读时，可以达到500MB/s的性能，所以针对我们的测试场景，理论上的单线程的极限带宽就是120MB/s。
* 选用的这款SSD文档中说的随机读的IOPS对于我们的场景是没有意义的，因为它的前提是8G的跨度，而我们的使用场景，跨度最大是100GB. 


## 测试用例

* 测试用例1: 10分钟一个脉冲，先读约9万个key，然后写21万个key，删除3万个key
* 测试用例2: 每10分钟读9万个key
* key是40字节，value为30字节到101字节随机生成。
* 两个测试用例中，做读操作时候，都需要先对key做一次排序，然后再读，尽力降低读key时候的随机性，提高性能
* 理论上，21万个insert和3万个delete的compaction绝对可以在10分钟内消化掉，且测试中发现写性能非常好，只需要0.5s左右，所以下面的测试主要基于测试用例2

## 编译二进制测试文件

### rocksdb
* 把pulserocks.cc 和Makefile放在rocksdb源码下的example目录下
* 进入rocksdb，make static_lib来编译release static_lib
* cd 到rocksdb/example目录下，执行make pulserocks 生成**pulserocks**用于rocksdb的性能测试

### leveldb

* 编译leveldb
* `g++ -o pulselevel  pulselevel.cc  -std=c++11 ~/leveldb/out-static/libleveldb.a -O2 -lpthread -lsnappy -I ~/leveldb/include` 生成**pulselevel**用于leveldb的性能测试

### 测试注意事项
* 因为rocksdb or leveldb有可能会打开大量的文件，所以测试时要把操作系统的最大文件打开数量调整到65535，否则测试过程中可能会因为too many open files导致无法打开数据库
* 为了防止内存cache影响性能测试，在必要的时候使用eatmem.cc编译出的二进制，吃掉内存，防止被page cache使用. eatmem的参数单位是字节。
`g++ eatmem.cc -o eatmem && ./eatmem 13958643712`
* 如果测试用例有写操作(测试用例1)，开始新一轮的测试需要重新导数据到数据库，保证每次测试尽力是一样
* `sync; echo 3 > /proc/sys/vm/drop_caches` 防止page cache带来问题
* 如果需要测试bloom filter的性能，必须在导入数据的时候，就要带着bloom filter的选项；如果测试不带bloom filter的选项，需要在导入数据库的时候就不指定bloom filter；
* 测试结果中，CPU是通过TOP统计出的峰值，memory是通过TOP统计的RES的峰值
* 因为UTXO的场景是脉冲式的，且脉冲时间是10分钟，所以可以认为下一轮脉冲开始的时候，前一轮的compaction是结束的状态，也就是说，下一轮脉冲的读操作不会受到前一轮compaction的影响; 使用测试用例2测试数据库过程中，为了防止compaction带来的性能影响，导完数据库以后，需要保持open数据库一段时间，保证之前的写操作compaction结束后再进行测试。但是测试过程中发现，leveldb无论如何都会做compaction，即使导完数据库后，保持数据打开很久以后，保证测试过程中只有读操作，仍然有compaction，为了防止测试代码有误还加入了下面的断点，然而断点没有触发(测试文件，见pulselevel.cc)：
```
(gdb) info break
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x0000000000413dd1 in leveldb::DB::Put(leveldb::WriteOptions const&, leveldb::Slice const&, leveldb::Slice const&) at db/db_impl.cc:1475
2       breakpoint     keep y   0x000000000043260e in leveldb::WriteBatch::Put(leveldb::Slice const&, leveldb::Slice const&) at db/write_batch.cc:99
3       breakpoint     keep y   0x0000000000413e85 in leveldb::DB::Delete(leveldb::WriteOptions const&, leveldb::Slice const&) at db/db_impl.cc:1481
```


## 10GB场景测试

生成测试数据`python genKeyValue.py 10737418240` 的到文件trx.bin，重命名为trx10GB.bin

### leveldb

#### 测试命令
* `./eatmem 15032385536`  # 防止page cache占用15032385536(14G)个字节的内存
* `swapoff <swap dev path>`   # 关闭swap
* `./pulselevel load pulserw ~/trx10GB.bin` 生成的数据导入数据库，数据导入之后数据变为12GB左右，属于正常现象。pulserw是数据库名字。
* `./pulselevel sort pulserw ~/trx10GB.bin` 用于进行单线程读写测试
* `./pulselevel mread pulserw ~/trx10GB.bin 8` 用于进行8线程的read
* `./pulselevel mread pulserw ~/trx10GB.bin 16` 用于进行16线程的read
* `./pulselevel mread pulserw ~/trx10GB.bin 32` 用于进行32线程的read

执行以上命令，输出如下：
```
now:Tue Apr 24 16:25:39 2018
read:90000 elapsed:20.8294
now:Tue Apr 24 16:25:39 2018
add:210000 delete:30000 elapsed:0.373496
 
now:Tue Apr 24 16:30:59 2018
read:90000 elapsed:19.425
now:Tue Apr 24 16:30:59 2018
add:210000 delete:30000 elapsed:0.337532

now:Tue Apr 24 16:36:19 2018
read:90000 elapsed:19.7889
now:Tue Apr 24 16:36:20 2018
add:210000 delete:30000 elapsed:0.348685
```

* 以上结果输出了3 轮的测试，read:xxx 表示读key的数量，add：xxx表示插入key的数量，delete:xxx表示删除key的数量，elapsed表示对应操作的耗时，单位是秒
* 这个结果是测试用例1的测试，测试用例2需要简单调整测试代码，调整后add和delete的数量会变成0，之后的测试主要以测试用例2为主

#### 测试结果


##### 10GB，leveldb使用默认option：

| 线程数量 | 耗时        | cpu使用率 | 内存使用 |
|----------|-------------|-----------|----------|
|        1 | 37s-58s     |       70% | 900MB    |
|        8 | 18-21s 左右 |       80% | 750MB    |
|       16 | 17-18s      |       92% | 780MB    |


#### 10GB, leveldb使用max_open_files=11000:
| 线程数量 | 耗时       | cpu使用率 | 内存使用 |
|----------|------------|-----------|----------|
|        1 | 28s-40s    |       90% | 800M     |
|        8 | 9.5s-11.7s |      120% | 900M     |
|       16 | 5s-6.5s    |      115% | 900M     |


##### 10GB，leveldb使用max_open_files=11000, bloom_filter(10):
| 线程数量 | 耗时      | cpu使用率 | 内存使用 |
|----------|-----------|-----------|----------|
|        1 | 14s-21s   |       50% | 1G       |
|        8 | 2.7s-3.5s |       50% | 1G       |
|       16 | 3.4-4.3s  |      100% | 1G       |

##### 小结
* max_open_files和bloom_filter对leveldb读性能提升帮助很大


### rocksdb

#### 测试命令
* `/eatmem 15032385536` 
* `./pulserocks load pulserw ~/trx10GB.bin 1 1 ` 生成的数据导入数据库，数据导入之后数据变为12GB左右，属于正常现象。pulserw是数据库名字。
* `./pulserocks sort pulserw ~/trx10GB.bin 1 1` 用于进行单线程读写测试
* `./pulserocks mread pulserw ~/trx10GB.bin 8 300` 用于进行8线程的read
* `./pulserocks mread pulserw ~/trx10GB.bin 16 300` 用于进行16线程的read


#### 10GB，rocksdb使用默认option

| 线程数量 | 耗时      | cpu使用率 | 内存使用 |
|----------|-----------|-----------|----------|
|        1 | 10.8s-14s |   10%-26% | 100MB    |
|        8 | 1.9s-2.5s |   30%-53% | 120MB    |
|       16 | 1.7-1.9s  |   24%-45% | 130MB    |

#### 小结
* leveldb比rocksdb消耗更多的CPU，消耗更多的内存
* 定制了max_open_files和bloom filter后，leveldb的性能只是比rocksdb差了一点点，使用的内存也比rocksdb多了（前提是rocksdb没有发生compaction）
* 理论上，在其它资源一样的情况下，rocksdb和leveldb在读性能上应该差距不大，而测试中发现，即使是全读，leveldb也要做compaction
* rocksdb默认64MB sst文件，而leveldb默认2MB sst文件，所以在打开文件数量一样的情况下，rocksdb在内存中可存放更多的index block，从而加速读

***

## 50GB场景测试

生成测试数据 `python genKeyValue.py 53687091200`得到文件trx.bin，重命名为trx50GB.bin

### leveldb

#### 测试命令
见10GB使用场景，关闭了swap，并且占用13421772800字节的内存

#### 50GB, leveldb使用默认option:

| 线程数量 | 内存 | 耗时         | cpu利用率 |
|----------|------|--------------|-----------|
|        1 | 600M | 70s-100s左右 |       86% |
|        8 | 680M | 20-53s 左右  |      100% |
|       16 | 750M | 15-36s 左右  |      106% |

* 16线程之后性能并没有提升，说明单纯的增加线程数量对性能没有多大影响，主要是io已经成为整个数据库的瓶颈

#### 50GB, leveldb使用option.max_open_file = 11000,bloom_filter(10):
| 线程数量 | 内存 | 耗时         | cpu利用率 |
|----------|------|--------------|-----------|
|        1 | 700M | 34s-83s      |       91% |
|        8 | 860M | 6.4s-27s     |      120% |
|       16 | 860M | 4.8s-14.3s   |      130% |


### rocksdb

#### 测试命令
见10GB使用场景，关闭了swap，并且占用13421772800字节的内存，防止被page cache占用

#### 50GB, rocksdb使用默认option:

| 线程数量 | 内存 | 耗时     | cpu利用率 |
|----------|------|----------|-----------|
|        1 | 500M | 12s-16s  |   15%-20% |
|        8 | 400M | 3.2-3.7s |   40%-60% |
|       16 | 400M | 2.1-2.4s |   30%-%40 |


***

## 100GB 数据量场景测试
* 因为测试用例1中的随机函数的熵较低，最多只能生成50G的key/value，所以测试用例2使用顺序的key生成数据库。缺点是，它和随机生成的key的db还是有一些差别的，
  * 测试用例1使用随机生成的key，多个level之间会有overlap，如果启用bloom filter会有较明显的效果
  * 测试用例2使用顺序的整数，按照顺序的方式插入数据库，导致多个level之间没有overlap，bloom filter不会带来任何收益，反而产生空间放大问题，浪费内存

#### 测试命令

* `python createTestDB.py 100G pul100GB`   # 使用顺序的整数，从1开始直到生成的kv size达到100GB，经过40字节的大端的串行化，写入数据库，value是30～101字节随机生成. key是按照顺序插入数据库。输出如下

```
total number of keys: 1210292943, total size in bytes: 107374182600
```
其中，1210292943表示生成的数据库中key的数量，也就是说它生成了一个包含1到1210292943的key的数据库pul100GB.db(这些整数在数据库中是40字节大端格式)。我们的测试程序需要在1～1210292943之间进行随机读取。

* `python genrandomRead.py 1210292943` 生成用于随机读的输入文件，指定随机的最大key是1210292943。经过这一步生成文件trxread.bin.1210292943用于做随机读的输入文件
* `./pulserocks sort pul100GB.db trxread.bin.1210292943 1 300`  用于做单线程随机读测试, 每300秒做一轮
* `./pulserocks Mro pul100GB.db trxread.bin.1210292943 2 300 ` 用于做2线程随机读测试
* `./pulserocks Mro pul100GB.db trxread.bin.1210292943 4 300 ` 用于做4线程随机读测试
* `./pulserocks Mro pul100GB.db trxread.bin.1210292943 8 300 ` 用于做8线程随机读测试


### rocksdb

#### 使用rocksdb默认参数:
| 线程数量 | 内存  | 耗时      | cpu利用率 |
|   ------ | ----  | -------   | ------    |
|        1 | 1.46g | 12.8~16   | 11~26%    |
|        2 | 1.46g | 6.9~7.3   | 18~31%    |
|        4 | 1.46g | 4.6~4.95  | 73~91%    |
|        8 | 1.46g | 2.7~2.9   | 80~92%    |
|       16 | 1.46g | 1.75~1.76 | 21~38%    |
|       32 | 1.46g | 1.53~1.55 | 19~51%    |
|       64 | 1.46g | 1.56~1.58 | 23~37%    |

后来，使用如下参数，测试的性能和上表基本一致，没有性能提升，且内存占用大于14GB，且eatmem程序被kill了，看起来没法开启options.OptimizeForPointLookup：
```c++
   options.IncreaseParallelism();
   options.OptimizeForPointLookup(8);
   options.max_open_files = -1;
```

##### 小结

* block cache命中率约0.16% (因为leveldb不支持查看内部命中率)
* 开启bloom filter没有进一步性能提升的原因是，导入DB时，数据是顺序导入的，因此导入的数据不会和high level的文件有overlap，所以不经过compaction而直接放到最下层，各层之间没有overlap,所以bloom filter没有发挥优势
* 同样大小的内存，用来开启bloom filter，还是用来做cache，很难说哪一个可以带来更大的收益。
* 开启bloom filter会导致空间放大，而默认需要把所有的bloom filter放在内存中，而导致内存占用过大，所以性价比太低。
* 在key全顺序的场景下，CPU利用率并不低



### leveldb

#### 测试命令

* `python pul100GB.db trx100GB.seq.bin`  # 用于生成导入到leveldb中的二进制文件trx100GB.seq.bin
* `./pulselevel load pul100GBlevel trx100GB.seq.bin` # 用于把100GB的数据导入leveldb，所有的key都是按照顺序写入的
* `./pulselevel sort pul100GBlevel trxread.bin.1210292943 1`  # 单线程测试随机读
* `./pulselevel mread pul100GBlevel trxread.bin.1210292943 8`  # 8线程测试随机读
* `./pulselevel mread pul100GBlevel trxread.bin.1210292943 16`  # 16线程测试随机读


#### 优化option
*  options.max\_open\_files=11000
*  options.filter_policy = NewBloomFilterPolicy(10);

| 线程数量 | 内存      | 耗时         |   cpu利用率 |
|----------|-----------|--------------|-------------|
|        1 | 800M-850M | 5.3-5.7s左右 |     15%-20% |
|        8 | 900M-1GB  | 1-1.3左右    |     20%-30% |
|       16 | 900M-1GB  | 1s左右       | 30%-%35左右 |


#### 小结

* 和之前的测试相比，CPU占用低了很多，因为完全没有compaction的影响，且各个level的sst文件没有overlap。
* 这个测试用例不是特别符合UTXO的使用场景，这里是用来测试100GB场景下的极限性能
* 在完全顺序的情况下，没有compaction的影响，leveldb性能更好，实际场景下，key完全顺序的情况基本不存在

#### 总结

* 测试过程中，随机写21000个key，无论rocksdb还是leveldb耗时都是大约0.5s，所以写性能完全不是问题
* 导数据到两个DB的过程中，发现leveldb随机写性能比rocksdb明显要差，但是UTXO的随机写场景对leveldb来说还是够用的;对客户端节点来说，如果以后数据量进一步增大（>100GB）的时候，客户端首次同步，耗时会非常长，这时rocksdb的优势就明显了。
* leveldb 读过程中（没有写和删除操作），在LOG中发现有compaction，iostat也可以看到写操作,所以导致内存和CPU波动较大（compaction占用内存和CPU），这是leveldb的一个弱点。
* 除了100GB的情况，本测试看起来对leveldb不是很公平，因为测试过程中,leveldb有compaction（测试前，1. 保持DB打开，且确认日志中没有compaction后才开始测试，2. 只发读操作，从leveldb日志中仍然可以看到compaction，并且无法确定什么时候compaction终止, iostat也可以看到写操作），而rocksdb没有compaction
* 因为compaction的问题，leveldb比rocksdb有更高的读写放大问题，这么高的读写放大对ssd的性能和寿命都是挑战
* 10GB场景下，leveldb经过简单优化耗时可以降低为原来的1/10; 开启多线程的rocksdb相比无优化的leveldb，耗时可以降低为无优化leveldb的1/20，且占用内存更少，cpu利用率也更少，ssd寿命会更长

#### TODO

* 集成rocksdb到真实的UTXO验证场景下，使用真实的数据测试，而不是模拟测试
* 使用1 到1.2 billion的整数按照大端生成40字节的key，按照随机的方式导入的数据库生成100GB的DB用于测试
* 调查leveldb compaction触发机制以及compaction机制,为什么读的时候会有大量compaction
