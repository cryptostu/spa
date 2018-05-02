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
* 为了防止内存过大，cache过多，测试过程中使用malloc分配了约13GB的内存，防止page cache占用这过多内存 `./eatmem 13421772800**
* 使用python3
* 需要安装python-rocksdb
* 发起测试时，磁盘利用率约为55%，并在ssd上创建了LVM
* 使用-O2选项进行编译rocksdb和leveldb
* 安装libsnappy


### 小结

* 使用dd开启direct io，4K顺序读的性能只有120MB/s, 使用1M顺序读时，可以达到500MB/s的性能，所以针对我们的测试场景，理论上的单线程的极限带宽就是120MB/s。
* 选用的这款SSD文档中说的随机读的IOPS对于我们的场景是没有意义的，因为它的前提是8G的跨度，而我们的使用场景，跨度最大是100GB. 


## 测试用例

* 测试用例1:UTXO的使用场景是，10分钟一个脉冲，先读约9万个key，然后写21万个key，删除3万个key
* 测试用例2: 生成100GB的数据库，只测试读9万个key的场景
  * 由于测试过程中发现写和删除性能非常好，所以本测试用例只测试读
  * 因为测试用例1中的随机函数的熵较低，最多只能生成50G的key/value，所以测试用例2使用顺序的key生成数据库。缺点是，它和随机生成的key的db还是有一些差别的，
    * 测试用例1使用随机生成的key，多个level之间会有overlap，如果启用bloom filter会有较明显的效果
    * 测试用例2使用顺序的整数，按照顺序的方式插入数据库，导致多个level之间没有overlap，bloom filter不会带来任何收益
* key是40字节，value为30字节到101字节随机生成。
* 两个测试用例中，做读操作时候，都需要先对key做一次排序，然后再读，尽力降低读key时候的随机性，提高性能

## 编译二进制测试文件

### rocksdb
* 把pulserocks.cc 和Makefile放在rocksdb源码下的example目录下
* 进入rocksdb，make static_lib来编译release static_lib
* cd 到rocksdb/example目录下，执行make pulserocks 生成**pulserocks**用于rocksdb的性能测试

### leveldb

* 编译leveldb
* `g++ -o pulselevel  pulselevel.cc  -std=c++11 ~/leveldb/out-static/libleveldb.a -g -lpthread -lsnappy -I ~/leveldb/include` 生成**pulselevel**用于leveldb的性能测试

### 测试注意事项
* 因为rocksdb or leveldb有可能会打开大量的文件，所以测试时要把操作系统的最大文件打开数量调整到10w以上，否则测试过程中可能会因为too many open files导致无法打开数据库
* 为了防止内存cache影响性能测试，在必要的时候使用eatmem.cc编译出的二进制，吃掉内存，防止被page cache使用. eatmem的参数单位是字节。
`g++ eatmem.cc -o eatmem && ./eatmem 13958643712`


## 10GB场景测试

生成测试数据`python genKeyValue.py 10737418240` 的到文件trx.bin，重命名为trx10GB.bin

### leveldb

#### 测试命令
* `./eatmem 13421772800`  # 防止page cache占用13421772800个字节的内存
* `swapoff <swap dev path>`   # 关闭page cache
* `./pulselevel load pulserw ~/trx10GB.bin` 生成的数据导入数据库，数据导入之后数据变为12GB左右，属于正常现象。pulserw是数据库名字。
* `./pulselevel sort pulserw ~/trx10GB.bin` 用于进行单线程读写测试
* `./pulselevel mread pulserw ~/trx10GB.bin 8` 用于进行8线程的read
* `./pulselevel mread pulserw ~/trx10GB.bin 16` 用于进行16线程的read
* `./pulselevel mread pulserw ~/trx10GB.bin 32` 用于进行32线程的read

执行以上命令，输出如下：
```
now:Tue Apr 24 16:20:18 2018
read:90000 elapsed:24.3973
now:Tue Apr 24 16:20:18 2018
add:210000 delete:0 elapsed:0.333606
 
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

* 以上结果输出了4轮的测试，read:xxx 表示读key的数量，add：xxx表示插入key的数量，delete:xxx表示删除key的数量，elapsed表示对应操作的耗时，单位是秒

#### 测试结果



##### 10GB，leveldb使用默认option：

| 线程数量 | 耗时     | cpu使用率 | 内存使用       |
|----------|----------|-----------|----------------|
|        8 | 20s 左右 |   20%-40% | 1GB-1.1GB 左右 |
|       16 | 17s 左右 |   30%-45% | 900M-1GB 左右  |
|       32 | 16s 左右 |   40%-43% | 1GB 左右       |


##### 10GB，leveldb使用max_open_files=5000:
| 线程数量 | 耗时 | cpu使用率 |cache|
|----------|------|-----------|----|
|        1 |   17.3-21s 左右(偶尔会有28s左右)   |     90%左右      |900M-1.3GB
|        8 |  6-7s左右(偶尔会有10s左右)    |      70%左右     | 600-900M左右
|       16 |    5-6.5s左右  |      40%-60%     | 500-650M左右


线程数开到32时，平均耗时在16s左右，相比16线程有1s左右的提升，考虑到增加线程数量带来的收益并不是很明显，没有继续增加线程的数量。


### rocksdb

#### 测试命令

* `./pulserocks load pulserw ~/trx10GB.bin 1 1 ` 生成的数据导入数据库，数据导入之后数据变为12GB左右，属于正常现象。pulserw是数据库名字。
* `./pulserocks sort pulserw ~/trx10GB.bin 1 1` 用于进行单线程读写测试
* `./pulserocks mread pulserw ~/trx10GB.bin 8 300` 用于进行8线程的read
* `./pulserocks mread pulserw ~/trx10GB.bin 16 300` 用于进行16线程的read


#### 10GB，rocksdb使用默认option

| 线程数量 | 耗时      | cpu使用率 | 内存使用  |
|----------|-----------|-----------|-----------|
|        1 | 10.8s-13s |   30%-36% | 800-900M  |
|        8 | 2.3s-2.5s |   30%-35% | 1.1-1.2GB |
|       16 | 1.1-1.6s  |   24%-26% | 1GB       |

***

## 50GB场景测试

生成测试数据 `python genKeyValue.py 53687091200`得到文件trx.bin，重命名为trx50GB.bin

### leveldb

#### 测试命令
见10GB使用场景，关闭了swap，并且占用13421772800字节的内存，防止被page cache占用

#### 50GB, leveldb使用默认option:

| 线程数量 | 内存       | 耗时        |   cpu利用率 |
|----------|------------|-------------|-------------|
|        1 | 700M-900M  | 100s左右    |     20%-40% |
|        8 | 1.5M-1.8GB | 45s 左右    |     20%-30% |
|       16 | 1.7-2GB    | 42-46s 左右 | 15%-%35左右 |

* 16线程之后性能并没有提升，说明单纯的增加线程数量对性能没有多大影响，主要是io已经成为整个数据库的瓶颈。**
* 虽然内存占用看起来没有达到100%，但是因为使用的是top命令，统计周期和被测试程序的运行周期不同，所以，有可能在某个短暂的时间cpu已经成为了瓶颈

#### 50GB, leveldb使用option.max_open_file = 11000:
| 线程数量 | 耗时 | cpu利用率 |内存
|----------|------|-----------|----|
|        1 |   64s左右   |    93%左右       |2.6GB左右
|        8 |    12-13s左右  |    95%左右       | 3GB左右（已经到达最大允许值）
|       16 |    6s-8s左右（有时会有12s的情况出现）  |      87-92%     | 3GB左右（已经到达最大允许值）

### rocksdb

#### 测试命令
见10GB使用场景，关闭了swap，并且占用13421772800字节的内存，防止被page cache占用

#### 50GB, rocksdb使用默认option:

| 线程数量 | 内存      | 耗时     | cpu利用率 |
|----------|-----------|----------|-----------|
|        1 | 1.2GB     | 12s-16s  |   20%-30% |
|        4 | 1.2GB     | 4s-5s    |   40%-60% |
|        8 | 1.1GB     | 2.6-2.7s |   40%-60% |
|       16 | 1.1-1.2GB | 1.5-2s   |   30%-%40 |


***

## 100GB 数据量场景测试

因为根据之前的测试场景，已经能够确定，写操作性能非常好，所以在本场景下，只做读操作的测试，使用前文提到的测试用例2

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
|    线程数量 | 耗时                   | cpu利用率         |
| ----------- | ---------------------- | ----------------- |
|           1 | 12.8~16                | 11~26%            |
|           2 | 6.9~7.3                | 18~31%            |
|           4 | 4.6~4.95               | 73~91%            |
|           8 | 2.7~2.9                | 80~92%            |
|          16 | 1.75~1.76              | 21~38%            |
|          32 | 1.53~1.55              | 19~51%            |
|          64 | 1.56~1.58              | 23~37%            |

后来，使用如下参数，测试的性能和上表基本一致，没有性能提升，且内存占用大于14GB，且eatmem程序被kill了：
```c++
   options.IncreaseParallelism();
   options.OptimizeForPointLookup(8);
   options.max_open_files = -1;
```

##### 小结

* 测试rocksdb内存占用约1.443GB
* block cache命中率约0.16% (因为leveldb不支持查看内部命中率，所以之前的leveldb中的结果没有给出block cache命中率**
* page/buffer cahce约515MB~560MB
* 开启bloom filter以后，不是立即生效，只有开启后被写过或compact过的文件才会产生bloom filter block；或者可以选择把老库的kv导入到开启bloom filter的新库中.
* 开启bloom filter会导致空间放大
* 开启bloom filter没有进一步性能提升的原因是，导入DB时，数据是顺序导入的，因此导入的数据不会和high level的文件有overlap，所以不经过compaction而直接放到最下层，各层之间没有overlap,所以bloom filter没有发挥优势，而实际场景下，开启bloom filter肯定会有性能提升。


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

|线程数量|内存|耗时|cpu利用率|
|------|----------|------------|----------|
|1     |800M-850M | 5.3-5.7s左右|15%-20%   |
|8     |900M-1GB  | 1-1.3左右   |20%-30%   |
|16    |900M-1GB  | 1s左右      |30%-%35左右|

 

#### 小结

通过不断调整`max_open_files`的数量，发现当`max_open_files`达到11000的时候，性能会有较大提升，耗时大约稳定在1.3s左右，当`max_open_files`的数量在10000左右的时候，耗时大约在10s左右（leveldb默认`max_open_files`的数量为1000），因为当数据量达到100GB的时候，文件的数量急剧上升，当`max_open_files`的值较小时，leveldb会根据 LRU 算法的策略，不断的关闭打开相应的文件，并且在这个过程中需要把`index_block`读出来，所以对性能还是有较大的损耗。

通过不断增加线程的数量，发现leveldb在此场景下，8线程的随机读性能较好，再继续增加线程的数量所带来的收益并不是很高。


#### 总结

* 随机写21000个key，耗时大约0.5s，所以写性能完全不是问题
* UTXO验证交易（只读）的场景下，稍微优化leveldb的参数，leveldb性能是rocksdb性能的两倍
* 在DB容量是100GB的情况下，CPU使用双核四线程，在options.max_open_files=11000，options.block_cache 8MB，8线程读的情况下，levelDB读性能达到最佳，且内存占用约800MB到1GB之间，在实际场景下，启用bloom_filter，应该会获得更高的性能。

* leveldb单线程的场景下，适当增加options.max_open_files的数量，也可以获得更好的性能，但是相应的会耗费更多的内存。
* 本测试的结果，只能说明在只有读的场景下leveldb性能要好过rocksdb，rocksdb在混合读写的场景下有可能还是会有更好的表现的，精心实现的skiplist以及支持hashseach索引的block based table都是很好的想法。

#### TODO

* 调查在只读场景下，为什么rocksdb读性能比leveldb差
* 集成rocksdb到真实的UTXO验证场景下，使用真实的数据测试，而不是模拟测试
* 使用1 到1.2 billion的整数按照大端生成40字节的key，按照随机的方式导入的数据库生成100GB的DB用于测试
* 测试一下rocksdb开启bloom filter，且数据库容量在10GB的场景下的性能
