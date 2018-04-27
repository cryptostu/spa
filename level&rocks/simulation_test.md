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


## 10GB场景测试

### leveldb

执行 `./pulse load pulserw ~/trx10GB.bin` 导入数据，数据导入之后数据变为12GB左右，属于正常现象。

### 单线程测试情况：

执行 `./pulse sort pulserw ~/trx10GB.bin` 进行单线程读写测试，为了缩短测试的周期，将原先的10分钟脉冲时间修改为5分钟，执行7轮测试之后，得到的数据情景如下所示：

```
now:Mon Apr 23 10:56:35 2018
 read:90000 elapsed:10.6247
now:Mon Apr 23 10:56:35 2018
 add:210000 delete:0 elapsed:0.255138
now:Mon Apr 23 11:01:39 2018
 read:90000 elapsed:3.76854
now:Mon Apr 23 11:01:40 2018
 add:210000 delete:30000 elapsed:0.250276
now:Mon Apr 23 11:06:42 2018
 read:90000 elapsed:2.48257
now:Mon Apr 23 11:06:42 2018
 add:210000 delete:30000 elapsed:0.248712
now:Mon Apr 23 11:11:44 2018
 read:90000 elapsed:1.8994
now:Mon Apr 23 11:11:45 2018
 add:210000 delete:30000 elapsed:0.2462
now:Mon Apr 23 11:16:46 2018
 read:90000 elapsed:1.57583
now:Mon Apr 23 11:16:46 2018
 add:210000 delete:30000 elapsed:0.259944
now:Mon Apr 23 11:21:48 2018
 read:90000 elapsed:1.34458
now:Mon Apr 23 11:21:48 2018
 add:210000 delete:30000 elapsed:0.242511
now:Mon Apr 23 11:26:50 2018
 read:90000 elapsed:1.26615
now:Mon Apr 23 11:26:50 2018
 add:210000 delete:30000 elapsed:0.236334

```

**根据上述情况可以分析得出：**  

read 90000 keys 的时间刚开始为 10s 左右，随后是 3s 左右，最后时长稳定在1.5s 左右，add 210000keys delete 30000 keys 的时长平均在 0.25s 左右；查看cache 容量在 15GB 左右，因为是 10GB 数据，怀疑数据大量命中 cache；drop cache 到 1GB 左右的时候，时长达到 41s 左右（数据之所以前后会有这么大的差距，主要是因为leveldb 第一次读的时候会打开所需要的 sstable 文件以及 block index 文件，打开之后，这些文件不会关闭，方便下一次读取，所以第一次耗时的原因主要是打开这些文件的耗时），此时 cache 的容量为 1GB 左右，一轮过后，cache 的容量涨到 8GB 左右，具体如下：
    
```
now:Mon Apr 23 11:42:35 2018
 read:90000 elapsed:41.0381
now:Mon Apr 23 11:42:35 2018
 add:210000 delete:30000 elapsed:0.241762

```

此时内存使用情况如下：

![memory.png](https://upload-images.jianshu.io/upload_images/6967649-f08c063f7291879d.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

cpu使用情况：

![cpu.png](https://upload-images.jianshu.io/upload_images/6967649-e797122d8de31894.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

继续测试几轮之后的情况如下：

```
now:Mon Apr 23 11:47:48 2018
 read:90000 elapsed:12.4957
now:Mon Apr 23 11:47:48 2018
 add:210000 delete:30000 elapsed:0.22138
now:Mon Apr 23 11:52:54 2018
 read:90000 elapsed:6.08168
now:Mon Apr 23 11:52:54 2018
 add:210000 delete:30000 elapsed:0.235318
now:Mon Apr 23 11:57:58 2018
 read:90000 elapsed:3.67622
now:Mon Apr 23 11:57:58 2018
 add:210000 delete:30000 elapsed:0.25569
now:Mon Apr 23 12:03:01 2018
 read:90000 elapsed:2.58885
now:Mon Apr 23 12:03:01 2018
 add:210000 delete:30000 elapsed:0.244801
now:Mon Apr 23 12:13:05 2018
 read:90000 elapsed:1.64761
now:Mon Apr 23 12:13:05 2018
 add:210000 delete:30000 elapsed:0.230124
 
 now:Mon Apr 23 13:18:24 2018
 read:90000 elapsed:0.929464
now:Mon Apr 23 13:18:24 2018
 add:210000 delete:30000 elapsed:0.233122
now:Mon Apr 23 13:23:25 2018
 read:90000 elapsed:0.932501
now:Mon Apr 23 13:23:25 2018
 add:210000 delete:30000 elapsed:0.236744
```

可以看到时间又继续稳定到 1 s 左右。

### 8 线程测试情况

执行 `./tests mread pulserw/ ~/trx10GB.bin 8` 启动 8 条线程测试 10GB 的情况，缓存大小为 161M 具体如下：

![buff.png](https://upload-images.jianshu.io/upload_images/6967649-3e4d243568528e2b.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

此时，数据的读写情况如下所示：

```
read thread cnt: 8

now:Mon Apr 23 13:53:17 2018
read:90000 elapsed:6.13213
now:Mon Apr 23 13:53:18 2018
add:210000 delete:30000 elapsed:0.217
```

当缓存大小达到 2.6GB 左右时，

![image.png](https://upload-images.jianshu.io/upload_images/6967649-35368302c346b670.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

测试情况如下：

```
read thread cnt: 8

now:Mon Apr 23 13:58:19 2018
read:90000 elapsed:1.51856
now:Mon Apr 23 13:58:19 2018
add:210000 delete:30000 elapsed:0.230256
```
当缓存达到 3GB 左右时，

![image.png](https://upload-images.jianshu.io/upload_images/6967649-b3f7125764295ce3.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

测试情况如下：

```
read thread cnt: 8

now:Mon Apr 23 14:03:20 2018
read:90000 elapsed:0.736117
now:Mon Apr 23 14:03:21 2018
add:210000 delete:30000 elapsed:0.228423
```

此时内存使用情况如下：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-4194960c4dfad504.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

cpu使用情况如下：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-dc24f7fc1f4dfea4.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


随着内存 cache 的增加，几轮测试之后，内存 cache 的使用量达到 3-4GB 左右， 此时的读性能稳定在 0.2s 左右。

```
read thread cnt: 8

now:Mon Apr 23 14:08:21 2018
read:90000 elapsed:0.426303
now:Mon Apr 23 14:08:21 2018
add:210000 delete:30000 elapsed:0.233573

now:Mon Apr 23 14:13:22 2018
read:90000 elapsed:0.395289
now:Mon Apr 23 14:13:22 2018
add:210000 delete:30000 elapsed:0.230291

now:Mon Apr 23 14:18:22 2018
read:90000 elapsed:0.402746
now:Mon Apr 23 14:18:23 2018
add:210000 delete:30000 elapsed:0.230094

now:Mon Apr 23 14:23:23 2018
read:90000 elapsed:0.357059
now:Mon Apr 23 14:23:23 2018
add:210000 delete:30000 elapsed:0.232445

now:Mon Apr 23 14:28:24 2018
read:90000 elapsed:0.338401
now:Mon Apr 23 14:28:24 2018
add:210000 delete:30000 elapsed:0.2291

now:Mon Apr 23 14:33:24 2018
read:90000 elapsed:0.259419
now:Mon Apr 23 14:33:25 2018
add:210000 delete:30000 elapsed:0.23363

now:Mon Apr 23 14:38:25 2018
read:90000 elapsed:0.20226
now:Mon Apr 23 14:38:25 2018
add:210000 delete:30000 elapsed:0.227768

now:Mon Apr 23 14:43:25 2018
read:90000 elapsed:0.200588
now:Mon Apr 23 14:43:26 2018
add:210000 delete:30000 elapsed:0.236501

now:Mon Apr 23 14:48:26 2018
read:90000 elapsed:0.205093
now:Mon Apr 23 14:48:26 2018
add:210000 delete:30000 elapsed:0.232268
```
此时 iops 的情况大体如下：

![iops.png](https://upload-images.jianshu.io/upload_images/6967649-f87d9eea221403c2.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

**根据iops的图标分析，8线程的情况下，io 耗时明显降低。整体性能是单线程的 5 倍左右，且 cache 的占有量比单线程还少。推测：cache的占有量进一步提升之后，性能还会有所提升。但实际情况下，如此多的 cache 是否都会拿来给 DB 使用，需要根据线上服务器的具体使用情况来评估。**

**_疑问：8线程的情况下，cache为什么一直上不去？_**

***

排除缓存问题，继续测试8线程情况，此时cache的大小为 100M，

```
now:Mon Apr 23 17:09:28 2018
 read:90000 elapsed:22.9826
now:Mon Apr 23 17:09:29 2018
 add:210000 delete:0 elapsed:0.224792
```

当cache大小达到10GB的时候，

![image.png](https://upload-images.jianshu.io/upload_images/6967649-7a39d1dfda832116.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


执行情况如下：

```
now:Mon Apr 23 17:14:32 2018
 read:90000 elapsed:3.59862
now:Mon Apr 23 17:14:32 2018
 add:210000 delete:30000 elapsed:0.238799
```

当cache达到11G的时候，

![image.png](https://upload-images.jianshu.io/upload_images/6967649-7689a12acc46cfbb.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

执行情况如下：

```
now:Mon Apr 23 17:19:34 2018
 read:90000 elapsed:1.57124
now:Mon Apr 23 17:19:34 2018
 add:210000 delete:30000 elapsed:0.232258
```

当cache达到如下情况的时候，

![image.png](https://upload-images.jianshu.io/upload_images/6967649-0468af7882088f44.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

执行情况如下：

```
now:Mon Apr 23 17:24:35 2018
 read:90000 elapsed:0.847863
now:Mon Apr 23 17:24:35 2018
 add:210000 delete:30000 elapsed:0.230939
```

当cache基本稳定在11GB左右的时候，执行情况如下：

```
now:Mon Apr 23 17:29:36 2018
 read:90000 elapsed:0.698113
now:Mon Apr 23 17:29:36 2018
 add:210000 delete:30000 elapsed:0.23436
 
 now:Mon Apr 23 17:34:37 2018
 read:90000 elapsed:0.665424
now:Mon Apr 23 17:34:37 2018
 add:210000 delete:30000 elapsed:0.231065
 
 now:Mon Apr 23 17:39:38 2018
 read:90000 elapsed:0.774737
now:Mon Apr 23 17:39:39 2018
 add:210000 delete:30000 elapsed:0.236405
 
 now:Mon Apr 23 17:44:39 2018
 read:90000 elapsed:0.782709
now:Mon Apr 23 17:44:40 2018
 add:210000 delete:30000 elapsed:0.234298
 
 now:Mon Apr 23 17:49:40 2018
 read:90000 elapsed:0.698601
now:Mon Apr 23 17:49:41 2018
 add:210000 delete:30000 elapsed:0.228215
```

## 1GB 场景测试复现（重新生成数据后）


单线程第一次测试情况：

```
now:Tue Apr 24 13:52:38 2018
 read:90000 elapsed:43.196
now:Tue Apr 24 13:52:38 2018
 add:210000 delete:0 elapsed:0.346635
```

cache情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-b5ccc7f925f567d3.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

iops情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-d543a188432b9984.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

可以看出，如果完全走磁盘的话，io耗时还是挺严重的。

随后几轮测试情况：

```
now:Tue Apr 24 13:57:52 2018
 read:90000 elapsed:13.5408
now:Tue Apr 24 13:57:52 2018
 add:210000 delete:30000 elapsed:0.345466
 
 now:Tue Apr 24 14:02:59 2018
 read:90000 elapsed:6.20058
now:Tue Apr 24 14:02:59 2018
 add:210000 delete:30000 elapsed:0.333215
 
 now:Tue Apr 24 14:08:03 2018
 read:90000 elapsed:3.75586
now:Tue Apr 24 14:08:03 2018
 add:210000 delete:30000 elapsed:0.362021
 
 now:Tue Apr 24 14:13:06 2018
 read:90000 elapsed:2.87561
now:Tue Apr 24 14:13:07 2018
 add:210000 delete:30000 elapsed:0.331009
```

中途清理一次缓存之后：

```
now:Tue Apr 24 14:18:52 2018
 read:90000 elapsed:45.3277
now:Tue Apr 24 14:18:53 2018
 add:210000 delete:30000 elapsed:0.321008
```
iops的耗时明显上来了：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-c3872004f577b6a4.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

继续测试几轮得出的数据：

```
now:Tue Apr 24 14:24:07 2018
 read:90000 elapsed:13.9351
now:Tue Apr 24 14:24:08 2018
 add:210000 delete:30000 elapsed:0.310101

now:Tue Apr 24 14:29:14 2018
 read:90000 elapsed:6.43357
now:Tue Apr 24 14:29:14 2018
 add:210000 delete:30000 elapsed:0.315123
 
 now:Tue Apr 24 14:34:18 2018
 read:90000 elapsed:3.77586
now:Tue Apr 24 14:34:19 2018
 add:210000 delete:30000 elapsed:0.308785
 
 now:Tue Apr 24 14:39:22 2018
 read:90000 elapsed:2.93462
now:Tue Apr 24 14:39:22 2018
 add:210000 delete:30000 elapsed:0.307338
 
 now:Tue Apr 24 14:44:24 2018
 read:90000 elapsed:2.13011
now:Tue Apr 24 14:44:25 2018
 add:210000 delete:30000 elapsed:0.305482
 
 now:Tue Apr 24 14:49:27 2018
 read:90000 elapsed:1.89997
now:Tue Apr 24 14:49:27 2018
 add:210000 delete:30000 elapsed:0.304472

now:Tue Apr 24 14:54:29 2018
 read:90000 elapsed:1.51756
now:Tue Apr 24 14:54:29 2018
 add:210000 delete:30000 elapsed:0.301668
```

因为在整个测试情景中，数据占有的cache一直比较高，考虑到线上业务不可能将大量内存拿给leveldb做cache，所以尝试写一个程序占用机器内存(占有10GB实际物理内存)，仅给leveldb分出来少量内存使用，这时的测试场景如下：

```
now:Tue Apr 24 15:00:09 2018
 read:90000 elapsed:40.0204
now:Tue Apr 24 15:00:10 2018
 add:210000 delete:30000 elapsed:0.582522
```

继续占用，当cache达到700M左右时，执行情况如下：

```
now:Tue Apr 24 15:06:00 2018
 read:90000 elapsed:50.4235
now:Tue Apr 24 15:06:01 2018
 add:210000 delete:30000 elapsed:0.723389
 
 now:Tue Apr 24 15:11:50 2018
 read:90000 elapsed:48.5304
now:Tue Apr 24 15:11:50 2018
 add:210000 delete:30000 elapsed:0.339385
```

这时内存占有量如下：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-a21d981a8fe7b6a0.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

iops使用场景：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-30e1700064ad368d.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

### 10GB 8线程场景（修改之后）

```
now:Tue Apr 24 15:18:27 2018
 read:90000 elapsed:24.4636
now:Tue Apr 24 15:18:28 2018
 add:210000 delete:0 elapsed:0.334705
 
 now:Tue Apr 24 15:23:46 2018
 read:90000 elapsed:17.7896
now:Tue Apr 24 15:23:46 2018
 add:210000 delete:30000 elapsed:0.35213
 
 now:Tue Apr 24 15:29:06 2018
 read:90000 elapsed:19.3754
now:Tue Apr 24 15:29:06 2018
 add:210000 delete:30000 elapsed:0.33059
 
 now:Tue Apr 24 15:34:25 2018
 read:90000 elapsed:19.1046
now:Tue Apr 24 15:34:25 2018
 add:210000 delete:30000 elapsed:0.340138
 
 now:Tue Apr 24 15:39:45 2018
 read:90000 elapsed:19.8558
now:Tue Apr 24 15:39:46 2018
 add:210000 delete:30000 elapsed:0.316071
```

此时，cache使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-0d5db2d49ceae675.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

iops情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-09a75568d86d3ce7.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

cpu使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-3f5a0defb5785bdd.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 总结：在这个过程中，iops耗时较为严重，8线程较单线程性能上有明显的提升，内存已经全部使用完毕，并且使用少量的swap分区，在脉冲到来时发现cpu的资源使用也明显增高。经过几轮测试，10GB数据量，90000keys 读耗时平均稳定在 20s 左右。


### 10GB 8线程场景测试（修改后(关闭swap)，swap关闭之后缓存相对稳定到将近1GB左右）

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

#### 小结：可以看到几轮测试之后，耗时平均稳定在20s左右

### 10GB 16线程场景测试（修改后(关闭swap)swap关闭之后缓存相对稳定到将近1GB左右）

```
now:Tue Apr 24 16:06:03 2018
 read:90000 elapsed:20.6467
now:Tue Apr 24 16:06:03 2018
 add:210000 delete:0 elapsed:0.335697
 
now:Tue Apr 24 16:11:21 2018
 read:90000 elapsed:17.5517
now:Tue Apr 24 16:11:21 2018
 add:210000 delete:30000 elapsed:0.344421
 
 now:Tue Apr 24 16:16:39 2018
 read:90000 elapsed:17.7426
now:Tue Apr 24 16:16:40 2018
 add:210000 delete:30000 elapsed:0.330766
```
cache使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-4b2a4eb4e7059b0a.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 小结：可以看到当cache一直保持在这个水准之时（关闭swap），读90000keys的平均时长为 17s 左右。


### 10GB 32线程场景测试（修改后(关闭swap)swap关闭之后缓存相对稳定到将近1GB左右）

```
now:Tue Apr 24 17:01:54 2018
read:90000 elapsed:18.8075
now:Tue Apr 24 17:01:54 2018
add:210000 delete:0 elapsed:0.361226
 
now:Tue Apr 24 17:07:11 2018
read:90000 elapsed:16.0896
now:Tue Apr 24 17:07:11 2018
add:210000 delete:30000 elapsed:0.360031

now:Tue Apr 24 17:12:27 2018
 read:90000 elapsed:16.2897
now:Tue Apr 24 17:12:28 2018
 add:210000 delete:30000 elapsed:0.336072
```
#### iops

![image.png](https://upload-images.jianshu.io/upload_images/6967649-dd17e65dcd57bba7.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### cpu

![image.png](https://upload-images.jianshu.io/upload_images/6967649-70d40bb7f0ba39fc.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 小结：线程数开到32时，平均耗时在16s左右，相比16线程有1s左右的提升，考虑到增加线程数量带来的收益并不是很明显，没有继续增加线程的数量。


### rocksdb


## 50GB场景测试


### leveldb

### 单线程场景测试

cache使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-8cf367906c4b22cc.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

iops使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-8e7deb8b0e26aa21.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

cpu使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-8945b9685c4d21c9.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

耗时测试情况如下：

```
now:Tue Apr 24 19:20:20 2018
 read:90000 elapsed:274.918（4min30s左右）
now:Tue Apr 24 19:20:21 2018
 add:210000 delete:0 elapsed:0.324584
 
 now:Tue Apr 24 19:30:03 2018
 read:90000 elapsed:281.876
now:Tue Apr 24 19:30:03 2018
 add:210000 delete:30000 elapsed:0.363961
 
 now:Tue Apr 24 19:38:24 2018
 read:90000 elapsed:200.644
now:Tue Apr 24 19:38:24 2018
 add:210000 delete:30000 elapsed:0.370086
 
 前几次速度较慢，猜想在打开大量的sstable文件，snappy压缩的过程中也会有一定的耗时。
 
now:Tue Apr 24 19:45:00 2018
 read:90000 elapsed:95.2789
now:Tue Apr 24 19:45:00 2018
 add:210000 delete:30000 elapsed:0.384663
 
 now:Tue Apr 24 19:51:58 2018
 read:90000 elapsed:117.219
now:Tue Apr 24 19:51:58 2018
 add:210000 delete:30000 elapsed:0.352128
 
 now:Tue Apr 24 19:58:37 2018
 read:90000 elapsed:95.0505
now:Tue Apr 24 19:58:37 2018
 add:210000 delete:30000 elapsed:0.36115
```

**可以看出，此时cpu的资源大量使用，经分析`iowait`耗费了大量的 cpu 资源，可以发现瓶颈大多在io上，cpu的使用量平均在60%左右，几轮之后，读操作平均耗时稳定在100s左右。**

![image.png](https://upload-images.jianshu.io/upload_images/6967649-c6c445dbcaa4dddf.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

**通过perf工具，可以发现，crc32校验耗费了不少cpu资源，但是此时cpu的使用量并没有达到瓶颈，所以并不赞同上次他们所提到的避免crc32校验的思路，在cpu没有达到瓶颈的时候，数据的安全性还是有一定的必要耗费一定的时间来保证的。**

### 8线程场景测试

```
now:Wed Apr 25 10:44:29 2018
 read:90000 elapsed:58.2093
now:Wed Apr 25 10:44:29 2018
 add:210000 delete:0 elapsed:0.378422
 
 now:Wed Apr 25 11:03:16 2018
 read:90000 elapsed:58.6239
now:Wed Apr 25 11:03:17 2018
 add:210000 delete:30000 elapsed:0.343294
 
 now:Wed Apr 25 11:09:05 2018
 read:90000 elapsed:45.9392
now:Wed Apr 25 11:09:06 2018
 add:210000 delete:30000 elapsed:0.360407
 
 now:Wed Apr 25 11:14:52 2018
 read:90000 elapsed:46.1371
now:Wed Apr 25 11:14:52 2018
 add:210000 delete:30000 elapsed:0.365829
 
 now:Wed Apr 25 11:20:36 2018
 read:90000 elapsed:42.5826
now:Wed Apr 25 11:20:36 2018
 add:210000 delete:30000 elapsed:0.328528
 
 now:Wed Apr 25 11:26:21 2018
 read:90000 elapsed:44.9444
now:Wed Apr 25 11:26:21 2018
 add:210000 delete:30000 elapsed:0.351337
 
 now:Wed Apr 25 11:32:05 2018
 read:90000 elapsed:42.609
now:Wed Apr 25 11:32:05 2018
 add:210000 delete:30000 elapsed:0.363173
```

**可以看到多轮测试之后，数据基本稳定在45s左右，性能有点差。**

cache 情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-aba9225f882ca13c.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

cpu：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-ecb2e07e1248c2ce.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

iops：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-9ecf5badce23b6a2.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

### 16线程场景测试

cache使用情况：

![image.png](https://upload-images.jianshu.io/upload_images/6967649-a98dccd28fb8f783.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


```
now:Wed Apr 25 13:37:55 2018
 read:90000 elapsed:50.0132
now:Wed Apr 25 13:37:55 2018
 add:210000 delete:0 elapsed:0.358963
 
 now:Wed Apr 25 13:48:47 2018
 read:90000 elapsed:51.7314
now:Wed Apr 25 13:48:48 2018
 add:210000 delete:30000 elapsed:0.365789
 
 now:Wed Apr 25 13:59:30 2018
 read:90000 elapsed:42.6379
now:Wed Apr 25 13:59:31 2018
 add:210000 delete:30000 elapsed:0.342867
 
 now:Wed Apr 25 14:10:17 2018
 read:90000 elapsed:46.4648
now:Wed Apr 25 14:10:18 2018
 add:210000 delete:30000 elapsed:0.33144
```

**小结：16线程之后性能并没有提升，说明单纯的增加线程数量对性能没有多大影响，主要是io已经成为整个数据库的瓶颈。**


### rocksdb



### 小结

> 10GB与50GB的场景下，leveldb与rocksdb使用的均是默认参数，没有做任何优化。

***



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
