#include <chrono>
#include <iostream>
#include <iterator>
#include <mpi.h>
#include <string>
#include <stdlib.h>

#include "scatter_gatherer.h"
#include "utils.h"
#include "logger.h"

static const int master = 0;
static Timer timer;

void ScatterGathererValueIterator::partialValueIteration(const int begin, const int end) {
  float error;

  // gather vectors / arrays
  g_counts_.resize(nproc_, offset_); g_counts_[0] = m_offset_;
  g_displacements_.assign(s_partitions_.begin(), s_partitions_.begin() + nproc_);

  for (unsigned int epoch = 0; epoch < this->max_iter_; epoch++) {
    error = 0.0;
    error = this->step(begin, end);

    if (epoch % freq_ == 0) {
      // gathering trajectory / J data throughout all processes
      timer.start();
      MPI_Allgatherv(/*const void*  buffer_send =   */ this->J_.data() + begin,
                     /*int          count_send =    */ (rank_ == master) ? m_offset_ : offset_,
                     /**MPI_Datatype send_datatype = */ MPI_FLOAT,
                     /*void*        buffer_recv =   */ this->J_.data(),
                     /*const int*   counts_recv =   */ g_counts_.data(),
                     /*const int*   displacements = */ g_displacements_.data(),
                     /*MPI_Datatype recv_datatype = */ MPI_FLOAT,
                     /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);
  
      MPI_Allreduce(&error, &error, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
      this->comm_time_ += timer.diff();

      if (error <= this->tolerance_) {
        break;
      }
    }
  }

}

void ScatterGathererValueIterator::implementation() {
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc_);


  offset_ = (findNextDivisable(this->nr_states_, nproc_) / nproc_);
  m_offset_ = offset_ + (this->nr_states_ - findNextDivisable(this->nr_states_, nproc_));

  // scatter vectors / arrays
  s_counts_.resize(nproc_, 2);
  s_partitions_.resize(nproc_ + 1, 0);
  s_displacements_.resize(nproc_);

  for (int i = 0; i < nproc_; i++) {
    s_partitions_[i + 1] = m_offset_ + (i * offset_);
    s_displacements_[i] = i;
  }

  timer.start();
  // [0] = begin state, [1] = end state
  int states_range[2];
  MPI_Scatterv(/*const void*  buffer_send =   */ s_partitions_.data(),
               /*const int    counts_end =    */ s_counts_.data(),
               /*const int    displacements = */ s_displacements_.data(),
               /*MPI_Datatype send_datatype = */ MPI_INT,
               /*void*        buffer_recv =   */ states_range,
               /*int          recv_count =    */ 2,
               /*MPI_Datatype recv_datatype = */ MPI_INT,
               /*int          root =          */ master,
               /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);

  this->comm_time_ += timer.diff();
  this->partialValueIteration(states_range[0], states_range[1]);

  timer.start();
  // gathering policy / Pi data
  MPI_Gatherv(/*const void*  buffer_send =   */ this->Pi_.data() + states_range[0],
              /*int          count_send =    */ (rank_ == master) ? m_offset_ : offset_,
              /*MPI_Datatype send_datatype = */ MPI_INT,
              /*void*        buffer_recv =   */ this->Pi_.data(),
              /*const int*   counts_recv =   */ g_counts_.data(),
              /*const int*   displacements = */ g_displacements_.data(),
              /*MPI_Datatype recv_datatype = */ MPI_INT,
              /*int          root =          */ master,
              /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);
  this->comm_time_ += timer.diff();
}
