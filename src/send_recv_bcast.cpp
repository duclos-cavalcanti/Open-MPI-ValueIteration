#include <chrono>
#include <iostream>
#include <iterator>
#include <string>

#include "send_recv_bcast.h"
#include "utils.h"
#include "logger.h"

#define ERROR_TAG 1
#define TRAJECTORY_TAG 1

static const int master = 0;
static Timer timer;

void SendRecvBcastValueIterator::partialValueIteration(const int begin, const int end) {
  float error, max_error;
  std::vector<float> J_recv(offset_, 0);

  for (unsigned int epoch = 0; epoch < this->max_iter_; epoch++) {
    max_error = 0.0;
    error = 0.0;
    error = this->step(begin, end);

    if (epoch % freq_ == 0) {
      if (rank_ != master) {
        // calculated J from this process to master
        MPI_Send(/*const void*  buffer_send =   */ this->J_.data() + begin,
                 /*int          count_send =    */ offset_,
                 /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                 /*int          recipient =     */ master,
                 /*int          tag =           */ rank_,
                 /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);

        // calculated error from this process to master
        MPI_Send(/*const void*  buffer_send =   */ &error,
                 /*int          count_send =    */ 1,
                 /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                 /*int          recipient =     */ master,
                 /*int          tag =           */ ERROR_TAG,
                 /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);
      } else {
        float e;
        max_error = error;
        for (int r = master + 1; r < nproc_; r++) {
          timer.start();
          MPI_Recv(/*const void*  buffer =        */ this->J_.data() + partitions_[r],
                   /*int          count =         */ offset_,
                   /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                   /*int          sender =        */ r,
                   /*int          tag =           */ r,
                   /*MPI_Comm     communicator =  */ MPI_COMM_WORLD,
                   /*MPI_Status   status =        */ MPI_STATUS_IGNORE);

          MPI_Recv(/*const void*  buffer =        */ &e,
                   /*int          count =         */ 1,
                   /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                   /*int          sender =        */ r,
                   /*int          tag =           */ ERROR_TAG,
                   /*MPI_Comm     communicator =  */ MPI_COMM_WORLD,
                   /*MPI_Status   status =        */ MPI_STATUS_IGNORE);
          this->comm_time_ += timer.diff();

          if (e > max_error)
            max_error = e;
        }
      }

      timer.start();
      MPI_Bcast(/*void*        buffer =        */ this->J_.data(),
                /*int          count =         */ this->nr_states_,
                /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                /*int          emitter_rank =  */ master,
                /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);

      MPI_Bcast(/*void*        buffer =        */ &max_error,
                /*int          count =         */ 1,
                /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                /*int          emitter_rank =  */ master,
                /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);
      this->comm_time_ += timer.diff();

      error = max_error;

      if (error <= this->tolerance_)
        break;
      }
    }
}

void SendRecvBcastValueIterator::implementation() {
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc_);

  offset_ = (findNextDivisable(this->nr_states_, nproc_) / nproc_);
  m_offset_ = offset_ + (this->nr_states_ - findNextDivisable(this->nr_states_, nproc_));

  // partitioning of begin and end states
  partitions_.resize(nproc_ + 1, 0);
  for (int i = 0; i < nproc_; i++) {
    partitions_[i + 1] = m_offset_ + (i * offset_);
  }

  // [0] = begin state, [1] = end state
  int states_range[2];
  states_range[0] = partitions_[rank_];
  states_range[1] = partitions_[rank_ + 1];

  this->partialValueIteration(states_range[0], states_range[1]);

  g_counts_.resize(nproc_, offset_); g_counts_[0] = m_offset_;
  g_displacements_.assign(partitions_.begin(), partitions_.begin() + nproc_);

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
