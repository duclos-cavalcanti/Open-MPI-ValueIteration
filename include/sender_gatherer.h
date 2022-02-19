#ifndef SENDER_GATHERER_H
#define SENDER_GATHERER_H

#include "value_iteration.h"

/**
 * Sender-Gatherer Class, inherits the ValueIterator
 */
class SenderGathererValueIterator : public ValueIterator {
public:

 /** Sender Gatherer Constructor
  *
  * There is no added functionality to this constructor, it just calls the constructor of the value iterator class.
  */
  SenderGathererValueIterator(struct Transformer& t, int frequency)
          : ValueIterator(t, frequency)
          {}

protected:

  /**
   * The MPI Scheme implementation.
   *
   * This communication scheme of this class uses the Gatherv function to create the complete Pi_ vector.
   * Each process finds calculates the begin and end states and the displacements and counts_recv vectors
   * According to its own world_rank and world_size
   * @return: void
   */
  void implementation(void) override;

  /**
   * The per process value iteration calculation.
   *
   * This function will loop until the max_iter_ and call the step function
   * with the parameters begin and end for the specific world_rank
   * and find the maximum error among all the processes. 
   * If this maximum error over all processes is smaller than the tolerance the iteration will break.
   * How the J's and error are updated among processes
   * is done through MPI_Allgatherv and MPI_Allreduce.
   * @param begin: unsigned int of the begin state for the process
   * @param end: unsigned int of the specific end state for the process
   * @return void:
   */
  void partialValueIteration(const int begin, const int end) override;

private:
  int begin, end;
  int world_rank, world_size;

  // Number of states (counts_recv) and displacements per process 
  std::vector<int> counts_recv;    // Is the amount of elements in J between the process specific begin and end 
  std::vector<int> displacements;  // Displacements is basically the process specific begin

};
#endif
