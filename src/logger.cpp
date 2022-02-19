#include "logger.h"

#include <sstream>
#include <fstream>

Logger::Logger(const std::string& p, int suffix) {
  if (suffix >= 0)
    this->path_ = "../" + p + "_" + std::to_string(suffix) + ".log";
  else
    this->path_ = "../" + p + ".log";
}

void Logger::log(std::string& message) {
  this->log_.push_back(message);
}

void Logger::logInt(int value, const std::string& header) {
  std::ostringstream ss;
  ss << header << " " << value;
  std::string message = ss.str();
  this->log(message);
}

void Logger::dump(void) {
  std::ofstream file(this->path_, std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Check the filename");

  for (auto& l : this->log_)
    file << "[info] " << l << std::endl;
  file.close();
}

void Logger::logIntArray(int arr[], int size, const std::string& header) {
  std::ostringstream ss;
  ss << header << " ";
  ss << "[ ";
  for (int i = 0; i < size; i++) {
    if (i == size - 1)
      ss <<  arr[i] << " ]" ;
    else
      ss <<  arr[i] << ", " ;
  }

  std::string message = ss.str();
  this->log(message);
}

void Logger::logFloatArray(float arr[], int size, const std::string& header) {
  std::ostringstream ss;
  ss << header << " ";
  ss << "[ ";
  for (int i = 0; i < size; i++) {
    if (i == size - 1)
      ss <<  arr[i] << " ]" ;
    else
      ss <<  arr[i] << ", " ;
  }

  std::string message = ss.str();
  this->log(message);
}

void Logger::logIntVector(std::vector<int> vec, const std::string& header) {
  std::ostringstream ss;
  ss << header << " ";
  ss << "[ ";

  for (auto &v : vec) {
      ss <<  v << ", " ;
  }
  ss << " ]" ;

  std::string message = ss.str();
  this->log(message);
}


void Logger::logFloatVector(std::vector<float> vec, const std::string& header) {
  std::ostringstream ss;
  ss << header << " ";
  ss << "[ ";

  for (auto &v : vec) {
      ss <<  v << ", " ;
  }
  ss << " ]" ;

  std::string message = ss.str();
  this->log(message);
}

