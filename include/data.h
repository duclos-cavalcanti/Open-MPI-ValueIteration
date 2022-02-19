#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>

/**
 * @defgroup data Data
 *
 * @brief Data parsing and reading code.
 *
 * Data Struct and the functions needed to read and prepare the data
 * needed for the chosen dataset.
 *
 * @{
 */


/**
 * Struct that holds the bare needed data to initiate out
 * asynchronous value iteration methods.
 */
struct Data {
  unsigned int nr_states_; /**< Number of States */
  unsigned int nr_stars_; /**< Number of stars */
  unsigned int nr_actions_; /**< Number of actions */

  std::vector<int> indptr_; /**< indptr */
  std::vector<int> indices_; /**< indices */
  std::vector<float> values_; /**< The values for the Probability Matrix */
  std::vector<int64_t> shape_; /**< Shape vector */

public:
  /** Data Constructor
  *
  * Takes in the name as a string of the path to the needed dataset, then
  * loads all parameters into the struct based on that path.
  */
  Data(std::string& data);

private:
  /**
   * Loads parameters from parameters.npz into the Data struct.
   * Is in charge of loading the nr_states, nr_stars and nr_actions.
   * @param data_path: path to data set folder [String]
   * @return void
   */
  void loadParameters(std::string& data_path);

  /**
   * Loads parameters from parameters.npz into the Data struct.
   * Is in charge of loading the indptr, indices, values, and shape.
   * @param data_path: path to data set folder [String]
   * @return void
   */
  void loadMatrix(std::string& data_path);
};


/** @}*/
#endif
