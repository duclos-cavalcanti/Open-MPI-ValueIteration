#include <string>
#include <sstream>
#include <fstream>
#include <strings.h>

#include "data.h"
#include "cnpy.h"

using cnpy::npy_load;
using cnpy::npz_load;
typedef cnpy::NpyArray NumpyArray;

/**
 * Loads parameters from parameters.pickle into the Data struct.
 * @param path: string, path to the parameters.npz file
 * @param param: string, parameter that is read from the numpy array as a string
 * @return n: NumpyArray 
 */
NumpyArray loadNumpyArray(std::string path, std::string param="") {
  cnpy::NpyArray n;
  try {
    if (param != "")
      n = npz_load(path, param);
    else
      n = npy_load(path);

  } catch (std::runtime_error& e) {
    std::cout << "ERROR:" << std::endl << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  return n;
}

void Data::loadParameters(std::string& data_path) {
  NumpyArray fuel = loadNumpyArray(data_path + "parameters.npz", "fuel_capacity");
  NumpyArray nr_stars = loadNumpyArray(data_path + "parameters.npz", "number_stars");
  NumpyArray max_controls = loadNumpyArray(data_path + "parameters.npz", "max_controls");
  NumpyArray max_jump_range = loadNumpyArray(data_path + "parameters.npz", "max_jump_range");
  NumpyArray confusion_distance = loadNumpyArray(data_path + "parameters.npz", "confusion_distance");
  NumpyArray nr_states = loadNumpyArray(data_path + "parameters.npz", "NS");

  this->nr_states_ = nr_states.as_vec<int>()[0];
  this->nr_stars_ = nr_stars.as_vec<int>()[0];
  this->nr_actions_ = max_controls.as_vec<int>()[0];

  return;
}

void Data::loadMatrix(std::string& data_path) {
  NumpyArray indptr = loadNumpyArray(data_path + "P_indptr.npy");
  NumpyArray indices = loadNumpyArray(data_path + "P_indices.npy");
  NumpyArray data = loadNumpyArray(data_path + "P_data.npy");
  NumpyArray shape = loadNumpyArray(data_path + "P_shape.npy");

  this->indptr_ = indptr.as_vec<int>();
  this->indices_ = indices.as_vec<int>();
  this->values_ = data.as_vec<float>();
  this->shape_ = shape.as_vec<int64_t>();

  return;
}

Data::Data(std::string& dataset) {
  std::string data_path = "../data/data_" + dataset + "/";

  loadMatrix(data_path);
  loadParameters(data_path);
}

