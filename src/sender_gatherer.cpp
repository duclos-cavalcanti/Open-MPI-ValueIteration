#include <chrono>
#include <iostream>
#include <iterator>
#include <string>

#include "sender_gatherer.h"
#include "utils.h"
#include "logger.h"

static const int master = 0;
static Timer timer;

/**
   * This function returns the begin state for the specific rank as an int
   * @param world_size: unsigned int, the total number of processes
   * @param world_rank: unsigned int, the id of the current process
   * @param j_size: unsigned int, the size of the J EigenVector
   * @return int: the first state for the specific process/ rank
   */
int getBeginState(int world_size, int world_rank, int j_size){
  
  int num_states_in_rank = ceil(float(j_size) / world_size);
  return world_rank * num_states_in_rank;
}

/**
   * This function returns the end state for the specific rank as an int
   * @param world_size: unsigned int, the total number of processes
   * @param world_rank: unsigned int, the id of the current process
   * @param j_size: unsigned int, the size of the J EigenVector
   * @param begin: int, the begin state for the specific process
   * @return int: the last state for the specific process/ rank
   */
int getEndState(int world_size, int world_rank, int j_size, int begin){

  int num_states_in_rank = ceil(float(j_size) / world_size);
  // if the process is the last one, then the last state is the total size of J -1
  if (world_rank == world_size - 1){
      return j_size - 1;
  }
  else{
    // otherwise it's the begin state plus the amount of states in each rank - 1
    return begin + num_states_in_rank -1;
  }
}

void SenderGathererValueIterator::partialValueIteration(const int begin, const int end) {

  float error;

  for (unsigned int epoch = 0; epoch < this->max_iter_; epoch++) {
    error = 0.0;
    
    // process specific error is calculated
    error = this->step(begin, end + 1);

    if (epoch % freq_ == 0) {
      timer.start();
      // gathering trajectory / J data over all processes
      MPI_Allgatherv(/*const void*  buffer_send =  */ this->J_.data() + begin,
                     /*int          count_send =    */ end - begin + 1,
                     /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                     /*void*        buffer_recv =   */ this->J_.data(),
                     /*const int*   counts_recv =   */ counts_recv.data(),
                     /*const int*   displacements = */ displacements.data(),
                     /*MPI_Datatype recv_datatype = */ MPI_FLOAT,
                     /*MPI_Comm     communicator =  */ MPI_COMM_WORLD);

      // Because we don't yet know if all processes have yet converged we use Allreduce to get the maximum error over all of the processes
      MPI_Allreduce(&error, &error, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
      this->comm_time_ += timer.diff();
 
      // The convergence is checked and the loop is broken
      if (error <= this->tolerance_)
        break;
    }
  }
}

void SenderGathererValueIterator::implementation() {
    //saving j_size in a variable
    const int j_size = J_.size();
    
    // Initializing world_size and world_rank
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Find the begin and end states for process = world_rank
    begin = getBeginState(world_size, world_rank, j_size);
    end =  getEndState(world_size, world_rank, j_size, begin);

    for(int i=0; i < world_size; i++)
    {
      // Find the begin and end states for process i. The last process also gets the remaining states, if there are any
      int begin_rank = getBeginState(world_size, i, j_size);
      int end_rank =  getEndState(world_size, i, j_size, begin_rank);

      int num_states = end_rank - begin_rank + 1;
      counts_recv.push_back(num_states);
      displacements.push_back(begin_rank); 
    }

    // This is the main function where the step function is called and the error, J and Pi are calculated and convergence is checked
    this->partialValueIteration(begin, end);
    
    timer.start();
    // The policy is gathered into one vector from the process specific Pis
    MPI_Gatherv(/*const void* buffer_send =    */ this->Pi_.data() + begin,
                /*int count_send =             */ end - begin + 1,
                /*MPI_Datatype send_datatype = */ MPI_FLOAT,
                /*void* buffer_recv =          */ this->Pi_.data(),
                /*const int* counts_recv =     */ counts_recv.data(),
                /*const int* displacements =   */ displacements.data(),
                /*MPI_Datatype recv_datatype = */ MPI_FLOAT,
                /*int root =                   */ master,
                /*MPI_Comm communicator =      */ MPI_COMM_WORLD);
    this->comm_time_ += timer.diff();

}


