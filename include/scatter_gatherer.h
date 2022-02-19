#ifndef SCATTER_GATHERER_H
#define SCATTER_GATHERER_H

#include "value_iteration.h"

/**
 * Scatter-Gatherer Class, inherits from ValueIterator
 */
class ScatterGathererValueIterator : public ValueIterator {
public:
  /** Scatter Gatherer Constructor
  *
  * There is no added functionality to this constructor, it passes the
  * overloaded arguments to the parent/super ValueIterator class.
  */
  ScatterGathererValueIterator(struct Transformer& t, int frequency)
          : ValueIterator(t, frequency)
          {}

protected:
  /**
   * The MPI Scheme implementation.
   *
   * This communication scheme of this class is based
   * on the Scatterv and Gatherv functions. The master process
   * scatters the begin and end states to all other processes, which then
   * process their chunk of the workload.
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
   * @param begin: unsigned int of the specific begin state
   * @param end: unsigned int of the specific end state
   * @return void:
   */
  void partialValueIteration(const int begin, const int end) override;

private:
  int offset_; /**< Number of states to be processed by non-master processes */
  int m_offset_; /**< Number of states to be processed by master */
  int rank_; /**< rank / process id of the current process where the class is executing */
  int nproc_; /**< total number of processes existing during execution */

  std::vector<int> s_counts_; /**< Number of elements scattered per process during the Scatter call */
  std::vector<int> s_displacements_; /**< Displacements / indexes to retreive data during the Scatter call */
  std::vector<int> s_partitions_; /**< Buffer containing the begin and end states to be sent out during the Scatter call */
  std::vector<int> g_counts_; /**< Number of elements to be collected per process during all Gather calls */
  std::vector<int> g_displacements_; /**< Displacements / indexes to retreive data during all Gather calls */
};
#endif
