#ifndef TIMER_H
#define TIMER_H

#include <chrono>

/**
 * Timer Class, helper class to facilitate timing
 */
class Timer {
  using timestamp = std::chrono::time_point<std::chrono::system_clock>;
  using clock = std::chrono::system_clock;
  using duration = std::chrono::duration<double, std::milli>;

public:
    Timer() {};

  /**
   * Overwrites/Sets the ts_ variable to the current clock value.
   * @return : void
   */
    void start() { 
      this->ts_ = clock::now();
    }

  /**
   * Gives time difference from the moment this method is called and the last time start was called
   * @return double: Time difference
   */
  double diff() {
    duration diff = (clock::now() - this->ts_);
  
    return (diff.count() / 1e3);
  }

private:
    timestamp ts_;/**< Holds the current timestamp from clock::now, gets overwritten by the start method */
};

#endif
