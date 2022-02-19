#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include "value_iteration.h"

/**
 * Sequential Gatherer Class, inherits from ValueIterator
 */
class SequentialValueIterator : public ValueIterator {
public:
  /** Sequential Constructor
  *
  * There is no added functionality to this constructor, it passes the
  * overloaded arguments to the parent/super ValueIterator class.
  */
  SequentialValueIterator(struct Transformer& t, int frequency)
          : ValueIterator(t, frequency)
          {}

protected:
  /**
   * The MPI Scheme implementation.
   *
   * No actual MPI API's used as this is supposed to run
   * emulating a 'single threaded' execution of the algorithm.
   * @return: void
   */
  void implementation(void) override;

  /**
   * The per process value iteration calculation.
   *
   * In this implementation this function will loop
   * through all epochs and step/iterate all
   * states and find the maximum error
   * among them.
   * @param begin: unsigned int of the specific begin state
   * @param end: unsigned int of the specific end state
   * @return void:
   */
  void partialValueIteration(const int begin, const int end) override;

private:

};


#endif
