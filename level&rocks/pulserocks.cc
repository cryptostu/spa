#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <thread>

#include <utility>
#include <sys/time.h>


#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/statistics.h"

std::string kDBPath = "pulserw";
const uint64_t pulse = 300000;
using namespace rocksdb;

void writer() {
  std::ifstream is ("trx.bin", std::ifstream::binary);
  uint64_t i = 0;
  unsigned short keyLen, valueLen;
  char keyBuf[40];
  char valueBuf[100];


  while (true) {
    is.read((char *)&keyLen, 2);
    is.read(keyBuf, keyLen);

    is.read((char *)&valueLen, 2);
    is.read(valueBuf, valueLen);

    if (++i % pulse) {
      //std::this_thread::sleep_for (std::chrono::minutes(10));
    }
  }

  is.close();
}

std::string random_string( size_t length )
{
  auto randchar = []() -> char
                  {
                    // const char charset[] =
                    //   "0123456789"
                    //   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    //   "abcdefghijklmnopqrstuvwxyz";
                    // const char charset
                    // const size_t max_index = (sizeof(charset) - 1);
                    // return charset[ rand() % max_index ];
                    return rand() % 256;
                  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}
Slice random_slice(size_t length)
{
  return Slice(random_string(length));
}

void prepareReadKeys(std::ifstream &fin, std::vector<std::string> &readKeys,
                     int readlength) {
  readKeys.clear();
  unsigned short keysize, valuesize;
  char keybuf[40], valuebuf[101];

  for (int i = 0; i < readlength; ++i) {
    fin.read((char*)&keysize, 2);
    fin.read(keybuf, keysize);
    assert(keysize == 40);

    readKeys.emplace_back(keybuf, keysize);

    fin.read((char*)&valuesize, 2);
    assert((valuesize >= 30 && valuesize <=101) || valuesize==0);
    if (valuesize > 0) {
        fin.read(valuebuf, valuesize);
    }
    //assert(valuesize == 0);
  }
}

void preparekeys(std::ifstream &fin,
                 std::vector<std::string> &readKeys,
                 std::vector< std::pair<std::string, std::string> > &addkeys,
                 std::vector<std::string> &deleteKeys,
                 int readlength, int addlength, int deletelength) {

  deleteKeys.clear();
  if (readKeys.size() > deletelength) {
    std::copy(readKeys.begin(), readKeys.begin()+deletelength, back_inserter(deleteKeys));
  }

  readKeys.clear();
  addkeys.clear();

  unsigned short keysize, valuesize;
  char keybuf[40], valuebuf[101];
  for (int i = 0; i < addlength; ++i) {
    fin.read((char*)&keysize, 2);
    assert(keysize <= 40);
    fin.read(keybuf, keysize);

    fin.read((char*)&valuesize, 2);
    assert(valuesize <= 101);
    fin.read(valuebuf, valuesize);

    std::string k = std::string(keybuf, keysize);
    std::string v = std::string(valuebuf, valuesize);

    addkeys.push_back(std::make_pair(k, v));

    // readKeys[rand() % readKeys.size()] = addkeys[i].first;
    // deleteKeys[rand() % deleteKeys.size()] = addkeys[i].first;

    if (readKeys.size() < readlength) {
      readKeys.push_back(addkeys[i].first);
    }
    // if (deleteKeys.size() < deletelength) {
    //   deleteKeys.push_back(addkeys[i].first);
    // }
    if (!fin) {
      return;
    }
  }
}

void storeData2DB(DB* db, std::string data_file_path) {

  std::cout << data_file_path << std::endl;

  std::ifstream fin(data_file_path, std::ios::in | std::ios::binary);
  WriteOptions writeOpt;

  unsigned short keysize, valuesize;
  char keybuf[40], valuebuf[100];
  Status s;
  while (fin) {
    fin.read((char*)&keysize, 2);
    fin.read(keybuf, keysize);
    //assert(keysize == 40);
    if (keysize != 40)  {
      std::cerr << "key size is " << keysize << std::endl;
    }

    fin.read((char*)&valuesize, 2);
    fin.read(valuebuf, valuesize);
    //assert(valuesize <= 101);
    //assert(valuesize >=30);
    if (valuesize < 30 || valuesize > 101) {
      std::cerr << "key value is " << valuesize << std::endl;
    }

    s = db->Put(writeOpt, Slice(keybuf, keysize), Slice(valuebuf, valuesize));
    if (!s.ok()) {
      std::cerr << "db::put error: " << s.code() << std::endl;
    }
  }
}


void doReadWorker(DB* db, const ReadOptions& ropt, std::vector<std::string>::iterator begin,
                  std::vector<std::string>::iterator end) {

  std::string value;
  for (; begin != end; ++begin) {
    db->Get(ropt, *begin, &value);
  }
}

void printStringContent(std::string &str) {
  assert(str.size() == 40);

  printf("0x");
  for (std::size_t i = 0; i < str.size(); ++i) {
    printf("%02x", (unsigned char)str.c_str()[i]);
  }
  // for (auto it = str.begin(); it != str.end(); ++it) {
  //   printf("%02x", *it);
  // }
}

// USAGE:
// 1. load data from binary file to DB:   ./pulse load
int main(int argc, char *argv[])
{
  DB* db;
  Options options;
  //options.IncreaseParallelism();
  //options.OptimizeLevelStyleCompaction();
  //options.OptimizeForPointLookup(512);   // for OptimizeForPointLookup

  //options.write_buffer_size = 4 << 20;  // for OptimizeForPointLookup
  // options.max_write_buffer_number = 2;
  // options.min_write_buffer_number_to_merge = 1;
  options.max_open_files = -1;

  Status s;
  std::string cmd(argv[1]);
  unsigned int sleepseconds;
  std::cout<< cmd << std::endl;
  sscanf(argv[5], "%u", &sleepseconds);


  if (argc < 5) {
    std::cerr << "error number of parameters" << std::endl;
    return -1;
  }



  if (cmd == "load") {

    std::cout << argv[2] << ", " << argv[3] << std::endl;
    options.create_if_missing = true;
    s = DB::Open(options, argv[2], &db);
    std::cout << s.code() << std::endl;
    storeData2DB(db, argv[3]);
    delete db;
    return 0;
  }

  std::ifstream fin(argv[3], std::ios::in | std::ios::binary);
  if (!fin) {
    std::cerr << "open file error " << argv[3] << std::endl;
    delete db;
    return -1;
  }


  options.statistics = rocksdb::CreateDBStatistics();
  options.statistics->stats_level_ = StatsLevel::kExceptDetailedTimers;
  options.create_if_missing = false;
  s = DB::Open(options, argv[2], &db);
  printf("openDB code: %d\n", s.code());
  std::cout << std::flush;
  assert(s.ok());

  if (cmd == "sleep") {
      sleep(9000);
  }

  int readkeylength = pulse*0.3;
  int addkeylength = pulse*0.7;
  int deletekeylength = pulse*0.1;

  std::vector<std::string> readKeys;
  std::vector<std::pair<std::string, std::string> > addKeys;
  std::vector<std::string> deleteKeys;


  ReadOptions readopt;
  WriteOptions writeopt;

  if (cmd == "MrSw" || cmd == "single") {
    preparekeys(fin, readKeys, addKeys, deleteKeys,
                readkeylength, addkeylength, deletekeylength);
  } else if (cmd == "Mro") {
    prepareReadKeys(fin, readKeys, readkeylength);
  }

  while (true) {
    std::string value;

    options.statistics->Reset();
    auto start = std::chrono::system_clock::now();

    if (cmd == "single") {
      std::sort(readKeys.begin(), readKeys.end());
    }
    if (cmd == "MrSw" || cmd == "Mro") {
      std::sort(readKeys.begin(), readKeys.end());

      size_t n = 2;
      if (argc >= 5) {
        sscanf(argv[4], "%zu", &n);
      }
      std::cout << "read thread cnt: " << n << std::endl << std::flush;
      std::vector<std::thread> workers;
      std::size_t length = readKeys.size() / n;
      std::size_t remain = readKeys.size() % n;
      std::size_t begin = 0;
      std::size_t end = 0;
      for (size_t i = 0; i < std::min(n, readKeys.size()); ++i) {
        end += remain > 0 ? length + !!(remain--): length;

        workers.emplace_back(doReadWorker, db, readopt, readKeys.begin()+begin, readKeys.begin()+end);

        std::cout << "from: ";
        printStringContent(*(readKeys.begin()+begin));
        std::cout << " to: ";
        printStringContent(*(readKeys.begin()+end-1));
        std::cout << std::endl << std::flush;

        std::cout << "from " << begin << " to " << end << std::endl;
        begin = end;
      }

      for (auto it = workers.begin();it != workers.end(); ++it) {
        it->join();
      }

      assert(workers.size() == n);
    } else {
      for (auto k: readKeys) {
        // TODO: read with multi-thread?
        db->Get(readopt, k, &value);
      }
    }


    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << options.statistics->ToString() << std::endl << std::flush;
    std::cout << "now:" << std::ctime(&end_time) <<  " read:" <<
      readKeys.size() << " elapsed:" << elapsed_seconds.count() << std::endl << std::flush;

    start = std::chrono::system_clock::now();
    WriteBatch batch;

    for (auto p: addKeys) {
      batch.Put(p.first, p.second);
    }

    for (auto k: deleteKeys) {
      if (k.size()) {
        batch.Delete(k);
      }
    }
    if (batch.Count()) {
      s = db->Write(writeopt, &batch);
      assert(s.ok());
    }
    end = std::chrono::system_clock::now();

    end_time = std::chrono::system_clock::to_time_t(end);
    elapsed_seconds = end-start;
    std::cout << "now:" << std::ctime(&end_time) <<  " add:" <<
      addKeys.size() << " delete:"<< deleteKeys.size() <<
      " elapsed:" << elapsed_seconds.count() << std::endl << std::flush;

    if (cmd == "MrSw") {
      preparekeys(fin, readKeys, addKeys, deleteKeys,
                  readkeylength, addkeylength, deletekeylength);
    }  else {
      prepareReadKeys(fin, readKeys, readkeylength);
    }

    if (!fin) {
      break;
    }
    sleep(sleepseconds);
  }
  delete db;
  return 0;
}
