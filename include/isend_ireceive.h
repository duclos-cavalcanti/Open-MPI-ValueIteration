#ifndef ISEND_IRECEIVE_H
#define ISEND_IRECEIVE_H

#include "value_iteration.h"

/**
 * Scatter-Gatherer Class, inherits from ValueIterator
 */
class IsendIreceiveValueIterator : public ValueIterator {
public:
  /** Isend Ireceive Constructor
  *
  * There is no added functionality to this constructor, it passes the
  * overloaded arguments to the parent/super ValueIterator class.
  */
  IsendIreceiveValueIterator(struct Transformer& t, int frequency)
          : ValueIterator(t, frequency)
          {}

protected:
  /**
   * The MPI Scheme implementation.
   *
   * This communication scheme of this class is based
   * on the non-blocking Isend and Irecv functions of the MPI library. 
   * The master process loops around all processes and waits for the updated Pi data
   * to be received. Each process is capable of calculating his begin and end states 
   * which describe the indexes of the states this process is responsible of computing, 
   * in other words the chunk of the workload to process.
   * @return: void
   */
  void implementation(void) override;

  /**
   * The per process value iteration calculation.
   *
   * In this communication scheme, this function will loop
   * through all epochs and step/iterate through the range
   * of states between begin and end and find the maximum error
   * among them. How the J's and error are updated among processes
   * is done through MPI_Allgatherv and MPI_Allreduce.
   * @param begin: unsigned int of the process-specific begin state 
   * @param end: unsigned int of the process-specific end state
   * @return void:
   */
  void partialValueIteration(const int begin, const int end) override;

private:

  int offset_; /**< Number of states to be processed by non-master processes */
  int m_offset_; /**< Number of states to be processed by master */
  int rank_; /**< rank / process id of the current process where the class is executing */
  int nproc_; /**< total number of processes existing during execution specific for this communicator */

  std::vector<int> s_partitions_; /**< Buffer containing the begin and end states to be used to populate g_displacements_ */
  std::vector<int> g_counts_; /**< Number of elements to be collected per process during all Gather calls */
  std::vector<int> g_displacements_; /**< Displacements / indexes to retreive data during all Gather calls */
};
#endif
