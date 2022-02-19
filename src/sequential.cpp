#include <iostream>
#include <chrono>
#include <string>
#include "sequential.h"

void SequentialValueIterator::partialValueIteration(const int begin, const int end) {
  float error;
  for (unsigned int epoch = 0; epoch < this->max_iter_; epoch++) {
    error = 0.0;
    error = this->step(begin, end);

    if (error <= this->tolerance_)
      break;
  }
}

void SequentialValueIterator::implementation() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0)
    return;

  this->partialValueIteration(0, this->nr_states_);
}
