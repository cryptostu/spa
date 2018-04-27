# leveldb VS rocksdb 

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
* 为了防止内存过大，cache过多，测试过程中使用malloc分配了13GB的内存，防止cache占用这13GB内存
* 使用python3
* 需要安装python-rocksdb
* 发起测试时，磁盘利用率约为55%，并在ssd上创建了LVM
* 使用-O2选项进行编译rocksdb和leveldb


### 小结

* 使用dd开启direct io，4K顺序读的性能只有120MB/s, 使用1M顺序读时，可以达到500MB/s的性能，所以针对我们的测试场景，理论上的单线程的极限带宽就是120MB/s。
* 选用的这款SSD文档中说的随机读的IOPS对于我们的场景是没有意义的，因为它的前提是8G的跨度，而我们的使用场景，跨度最大是100GB. 


## 测试用例

* 测试用例1:UTXO的使用场景是，10分钟一个脉冲，先读约9万个key，然后写21万个key，删除3万个key。
* 测试用例2: 只测试读9万个key的场景（由于测试过程中发现写和删除性能非常好，所以单独为读设计一个测试用例。
* key是40字节，value为30字节到101字节随机生成。
* 两个测试用例中，做读操作时候，都需要先对key做一次排序，然后再读。






## 100GB 数据量场景测试：


### leveldb(优化之后):

#### 测试步骤

* `./pulse load <DB path> <input key/value file>`用来将100GB的数据导入leveldb中
* `./pulse sort <DB path> <input key/value file>` 用来，排序key，然后单线程随机读测试
* `./pulse mread <DB path> <input key/value file> num ` 用来，排序key，然后多线程随机读测试


#### 测试结果

|线程数量|内存|耗时|cpu利用率|
|------|----------|------------|----------|
|1     |800M-850M | 5.3-5.7s左右|15%-20%   |
|8     |900M-1GB  | 1-1.3左右   |20%-30%   |
|16    |900M-1GB  | 1s左右      |30%-%35左右|

 小结：对leveldb的参数进行优化，具体优化内容如下：
 
*  options.max\_open\_files=11000
*  options.filter_policy = NewBloomFilterPolicy(10);

#### 小结

通过不断调整`max_open_files`的数量，发现当`max_open_files`达到11000的时候，性能会有较大提升，耗时大约稳定在1.3s左右，当`max_open_files`的数量在10000左右的时候，耗时大约在10s左右（leveldb默认`max_open_files`的数量为1000）。

通过不断增加线程的数量，发现leveldb在此场景下，8线程的随机读性能较好，再继续增加线程的数量所带来的收益并不是很高。

### rocksdb(优化之后（没有打开bloom_filter）):

测试步骤：

1. python createSeqKV.py 100G pulDB   // 生成用于100GB的测试数据库，运行结束后，会显示key的数量，例如1210292943，表示key的最大值是它
2. python genrandomRead.py <上一步key的最大值>    // 用于生成用于随机读的数据，例如，执行 


```python
python genrandomRead.py 1210292943   

```

生成trxread.bin.1210292943文件


因为使用测试用例一生成的key的范围不够大，最多只能生成50GB，所以100GB数据集的测试使用完全顺序的整数作为key（按照大端格式，40字节做serialize),

rocksdb with below options:

```c++
   options.IncreaseParallelism();
   options.OptimizeForPointLookup(8);

   options.write_buffer_size = 4 << 20;
   options.max_open_files = -1;
```

| thread number | time cost in seconds | peak of cpu usage |
| ----------- | -------------    | ---             |
|             1 | 12.8~16              | 11~26%            |
|             2 | 6.9~7.3              | 18~31%            |
|             4 | 4.6~4.95             | 73~91%            |
|             8 | 2.7~2.9              | 80~92%            |
|            16 | 1.75~1.76            | 21~38%            |
|            32 | 1.53~1.55            | 19~51%            |
|            64 | 1.56~1.58            | 23~37%            |


* 本次测试rocksdb内存占用约1.443GB
* block cache命中率约0.16%
* page/buffer cahce约515MB~560MB
* 后来，使用完全默认的rokcsdb参数,也基本能达到上述表格中的性能


#### 小结

32线程读就可以达到了最好的性能，由于线程数量的增加，导致给调度器带来的额外的负担，所以64线程的测试还低了一点点。8线程和16线程相比，16线程CPU利用率明显降低，估计也是一样的原因。所以使用8线程，可能是最好的方案。

## 测试脚本和代码

以下脚本用于生成随机的key/value pair


```python
#!/usr/bin/python

# genKV.py
# 生成随机的kv用于测试用例1


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
    
```


```python
#!/usr/bin/python

# createSeqKV.py
# 用于在rocksdb中直接生成测试用例二中的KV

import rocksdb
import sys
import os
import re
import random

def generateValue():
    bytes_length = random.randint(30, 101)
    return random.getrandbits(bytes_length << 3).to_bytes(bytes_length, "big")


def main():
    kvTotalSize, dbName = sys.argv[1], sys.argv[2] + ".db"
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
    db = rocksdb.DB(dbName, opts)
    opts.create_if_missing = True
    curkvSize = 0
    it = db.iterkeys()
    it.seek_to_last()
    last_key = it.get()
    seqi = int.from_bytes(last_key, "big")
    print(seqi)
    while curkvSize < kvTotalSize:
        seqi += 1
        key = seqi.to_bytes(40, "big")
        value = generateValue()
        db.put(key, value)

        curkvSize += len(key) + len(value)
    print("total number of keys: {}, total size in bytes: {}".format(seqi, curkvSize))


if __name__ == '__main__':
    main()

```



```python
#!/usr/bin/python

# genrandomRead.py
# 使用在100GB的场景下，需要随机的读key，本文件用于生成随机的key，保证生成的key一定在100G的DB中


import sys
import random
import struct

if __name__ == '__main__':
    totalSize = 2 << 30
    with open("trxread.bin."+sys.argv[1], 'wb+') as f:
        while f.tell() < totalSize:
            key = random.randint(1, int(sys.argv[1]) - 1)
            f.write(struct.pack('<H', 40))
            f.write(key.to_bytes(40, "big"))
            f.write(struct.pack('<H', 0))

```



```python
#!/usr/bin/python
# 用于把rocksdb中的kv导出成二进制格式，用来给leveldb做导入的源

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

```


```c++

// eatmem.cpp
// 用于吃掉空闲内存，防止这些内存被page cache占用
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  std::string inputsz(argv[1]);
  std::stringstream ss(inputsz);
  size_t sz;
  ss >> sz;
  std::cout << sz << std::endl;
  char *buf = (char*)malloc(sz);
  memset(buf, '-', sz);

  std::cout << "success" << std::endl;
  std::cin.get();
  free(buf);
  return 0;
}

```
