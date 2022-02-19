#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <vector>

/**
 * Logger Class, helper class to facilitate debugging
 */
class Logger {
  public:
  /** Logger Constructor
  *
  * Takes in the name as a string of the file to save the log and also
  * the rank of the current process, which will be used as a suffix to 
  * the file that get's saved by dump().
  */
    Logger(const std::string& p, int suffix=-1);

    ~Logger() {};

  /**
   * Logs string into logger
   * @param message: string message
   * @return: void
   */
    void log(std::string& message);
  /**
   * Logs Int value into logger
   * @param value: int value
   * @param header: string header string that identifies value in log
   * @return: void
   */
    void logInt(int value, const std::string& header);
  /**
   * Logs Int array into logger
   * @param arr[]: int array
   * @param size: int size of array
   * @param header: string header string that identifies value in log
   * @return: void
   */
    void logIntArray(int arr[], int size, const std::string& header);
  /**
   * Logs Float array into logger
   * @param arr[]: float array
   * @param size: int size of array
   * @param header: string header string that identifies value in log
   * @return: void
   */
    void logFloatArray(float arr[], int size, const std::string& header);
  /**
   * Logs Int Vector into logger
   * @param vec: std::vector<int> vector of values
   * @param header: string header string that identifies value in log
   * @return: void
   */
    void logIntVector(std::vector<int> vec, const std::string& header);
  /**
   * Logs Float Vector into logger
   * @param vec: std::vector<float> vector of values
   * @param header: string header string that identifies value in log
   * @return: void
   */
    void logFloatVector(std::vector<float> vec, const std::string& header);

  /**
   * Dumps all the logs in log_ into a log file.
   * @return: void
   */
    void dump(void);

private:
    std::vector<std::string> log_; /**< Vector holding all logs in string form */
    std::string path_; /**< Path to where this log will be stored */

};

#endif 
