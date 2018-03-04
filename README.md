# Software performance analysis

## Machine performance:

memory |    disk  |  CPU   | OS
------ | -------- |--------|-------
8G     | 512G SSD | i7 Core| Mac OS

### BoltDB and LevelDB

#### write

|      |    100W  |  500W   | 1000W|
------ | -------- |-------- |-------
boltDB | 197.39s  | 1186.45s| ~
levelDB| 1.40s    | 13.79s  | 47.01s

#### select

|      |    100W  |  500W   | 1000W|
------ | -------- |-------- |-------
boltDB | 2.58s    | 15.28s  | ~
levelDB| 3.39s    | 13.38s  | 31.85s

#### delete

|        |    100W  |  500W   | 1000W|
------  | -------- |-------- |-------
boltDB  | 283.18s  | 2346.86s| ~
levelDB | 1.51s    | 11.11s  | 34.82s
