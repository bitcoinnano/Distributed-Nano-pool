/*
 The MIT License (MIT)

 Copyright (c) [2016] [BTC.COM]

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>

#include <boost/interprocess/sync/file_lock.hpp>
#include <glog/logging.h>
#include <libconfig.h++>
#include <event2/thread.h>

#include "Utils.h"
#include "Watcher.h"

using namespace std;
using namespace libconfig;

// container for pool watch clients
ClientContainer *gClientContainer = nullptr;

void handler(int sig) {
  if (gClientContainer) {
    gClientContainer->stop();
  }
}

void usage() {
  fprintf(stderr, "Usage:\n\tpoolwatcher -c \"poolwatcher.cfg\" -l \"log_poolwatcher\"\n");
}

int main(int argc, char **argv) {
  char *optLogDir = NULL;
  char *optConf   = NULL;
  int c;

  if (argc <= 1) {
    usage();
    return 1;
  }
  while ((c = getopt(argc, argv, "c:l:h")) != -1) {
    switch (c) {
      case 'c':
        optConf = optarg;
        break;
      case 'l':
        optLogDir = optarg;
        break;
      case 'h': default:
        usage();
        exit(0);
    }
  }

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir         = string(optLogDir);
  // Log messages at a level >= this flag are automatically sent to
  // stderr in addition to log files.
  FLAGS_stderrthreshold = 3;    // 3: FATAL
  FLAGS_max_log_size    = 100;  // max log file size 100 MB
  FLAGS_logbuflevel     = -1;   // don't buffer logs
  FLAGS_stop_logging_if_full_disk = true;

  // Read the file. If there is an error, report it and exit.
  Config cfg;
  try
  {
    cfg.readFile(optConf);
  } catch(const FileIOException &fioex) {
    std::cerr << "I/O error while reading file." << std::endl;
    return(EXIT_FAILURE);
  } catch(const ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
    << " - " << pex.getError() << std::endl;
    return(EXIT_FAILURE);
  }

  // lock cfg file:
  //    you can't run more than one process with the same config file
  boost::interprocess::file_lock pidFileLock(optConf);
  if (pidFileLock.try_lock() == false) {
    LOG(FATAL) << "lock cfg file fail";
    return(EXIT_FAILURE);
  }

  signal(SIGTERM, handler);
  signal(SIGINT,  handler);

  // check if we are using testnet3
  bool isTestnet3 = true;
  cfg.lookupValue("testnet", isTestnet3);
  if (isTestnet3) {
    SelectParams(CBaseChainParams::TESTNET);
    LOG(WARNING) << "using bitcoin testnet3";
  } else {
    SelectParams(CBaseChainParams::MAIN);
  }

  gClientContainer = new ClientContainer(cfg.lookup("kafka.brokers"));

  // add pools
  {
    const Setting &root = cfg.getRoot();
    const Setting &pools = root["pools"];

    for (int i = 0; i < pools.getLength(); i++) {
      string name, host, worker;
      int32_t port;
      pools[i].lookupValue("name", name);
      pools[i].lookupValue("host", host);
      pools[i].lookupValue("port", port);
      pools[i].lookupValue("worker", worker);

      gClientContainer->addPools(name, host, (int16_t)port, worker);
    }
  }

  try {
    if (gClientContainer->init() == false) {
      LOG(ERROR) << "init failure";
    } else {
      gClientContainer->run();
    }
    delete gClientContainer;
  }
  catch (std::exception & e) {
    LOG(FATAL) << "exception: " << e.what();
    return 1;
  }

  google::ShutdownGoogleLogging();
  return 0;
}
