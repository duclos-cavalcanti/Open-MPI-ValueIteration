#include <iostream>
#include <vector>
#include <iomanip>
#include <mpi.h>

#include "data.h"
#include "transformer.h"
#include "sender_gatherer.h"
#include "scatter_gatherer.h"
#include "isend_ireceive.h"
#include "sequential.h"
#include "send_recv_bcast.h"

#define NUM_RUNS 2

// This is my first commit/ test
void printUsage(void) {
  
  std::cout << "Usage: ./project <data set>" << std::endl 
            << "data set: " << std::endl 
            << "\t * debug" << std::endl 
            << "\t * small" << std::endl 
            << "\t * normal" << std::endl;
}

void parseArgs(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    exit(EXIT_FAILURE);
  } else {
    std::string dataset = argv[1];
    if (dataset == "debug" || dataset == "small" || dataset == "normal") {
      return;
    } else {
      printUsage();
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {

  parseArgs(argc, argv);

  // data set
  std::string dataset = argv[1];
  struct Data data(dataset);
  struct Transformer t(data);

  MPI_Init(&argc, &argv);

  std::vector< std::unique_ptr<ValueIterator> > iterators;
  const std::vector<int> frequencies = { 2, 8, 20 };

  for (auto& f: frequencies) {
    iterators.push_back(std::unique_ptr<ValueIterator>(new ScatterGathererValueIterator(t, f)));
    iterators.push_back(std::unique_ptr<ValueIterator>(new SenderGathererValueIterator(t, f)));
    iterators.push_back(std::unique_ptr<ValueIterator>(new SendRecvBcastValueIterator(t, f)));
    iterators.push_back(std::unique_ptr<ValueIterator>(new IsendIreceiveValueIterator(t, f)));
  }

  for (auto& it: iterators)
    for (int i = 0; i < NUM_RUNS; ++i)
      it->iterateValues();

  for (auto& it: iterators)
    it->storeValues(dataset);

  MPI_Finalize();

  return 0;
}
