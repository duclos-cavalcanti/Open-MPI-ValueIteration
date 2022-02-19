#ifndef SEND_RECV_GATHERER_H
#define SEND_RECV_GATHERER_H

#include "value_iteration.h"

/**
 * SendRecv-Bcast Class, inherits from ValueIterator
 */
class SendRecvBcastValueIterator : public ValueIterator {
public:
  /** SendRecv-Bcast Constructor
  *
  * There is no added functionality to this constructor, it passes the
  * overloaded arguments to the parent/super ValueIterator class.
  */
  SendRecvBcastValueIterator(struct Transformer& t, int frequency)
          : ValueIterator(t, frequency)
          {}

protected:
  /**
   * The MPI Scheme implementation
   *
   * This communication scheme of this class is based
   * on the Send, Recv and Bcast functions. Each process
   * calcualtes their begin and end states and start to
   * process their chunk of the workload. The update however
   * is done by blocking sends to master, which also uses
   * blocking receives to collect the data. Finally master
   * will broadcast the max error and the unified J to all
   * other processes.
   * @return: void
   */
  void implementation(void) override;

  /**
   * The per process value iteration calculation
   *
   * In this communication scheme, this function will loop
   * through all epochs and step/iterate through the range
   * of states between begin and end and find the maximum error
   * among them. How the J's and error are updated however
   * is done by blocking sends to master, which also uses
   * blocking receives to collect the data. Finally master
   * will broadcast the max error and the unified J to all
   * other processes.
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

  std::vector<int> partitions_; /**< Buffer containing the begin and end states of all processes */
  std::vector<int> g_counts_; /**< Number of elements to be collected per process during all Gather calls */
  std::vector<int> g_displacements_; /**< Displacements / indexes to retreive data during all Gather calls */
};
#endif
