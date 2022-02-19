#include <chrono>
#include <iostream>
#include <iterator>
#include <string>

#include "isend_ireceive.h"
#include "utils.h"
#include "logger.h"

static const int master = 0;
static Timer timer;

void IsendIreceiveValueIterator::partialValueIteration(const int begin, const int end) {
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
                     /*MPI_Datatype send_datatype = */ MPI_FLOAT,
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

void IsendIreceiveValueIterator::implementation() {
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc_);

  offset_ = (findNextDivisable(this->nr_states_, nproc_) / nproc_);
  
  // calculate partitions of states
  if (rank_ == master) {
    m_offset_ = offset_ + (this->nr_states_ - findNextDivisable(this->nr_states_, nproc_));
  }

  // scatter vectors / arrays
  s_partitions_.resize(nproc_ + 1, 0);

  // start and finish states
  int begin = 0, end;

  timer.start();
  MPI_Bcast(&m_offset_, 
            1,
            MPI_INT,
            master,
            MPI_COMM_WORLD);
  this->comm_time_ += timer.diff();

  if(rank_ == master) {
    end = begin + m_offset_;    
  } else if(rank_ != master) {
    begin = m_offset_ + ((rank_-1) * offset_);
    end = begin + offset_;  
  }  

  for (int i = 0; i < nproc_; i++) {
    s_partitions_[i + 1] = m_offset_ + (i * offset_);
  } 

  this->partialValueIteration(begin, end); 

  MPI_Request requests[2] =
         {MPI_REQUEST_NULL, MPI_REQUEST_NULL};

  if(rank_ == master) {
    timer.start();
    for (int proc = 1; proc < nproc_; ++proc) {          
        MPI_Irecv(this->Pi_.data() + m_offset_ + ((proc-1) * offset_), 
                  offset_, 
                  MPI_FLOAT, 
                  proc,
                  1, 
                  MPI_COMM_WORLD,      
                  &requests[0]);
        MPI_Wait(&requests[0], MPI_STATUSES_IGNORE);
    }
    this->comm_time_ += timer.diff();

} else if(rank_ != master) {
    timer.start();
    MPI_Isend(this->Pi_.data() + begin, 
              offset_, 
              MPI_FLOAT,     
              master, 
              1, 
              MPI_COMM_WORLD,      
              &requests[1]);
    this->comm_time_ += timer.diff();
  }
}
