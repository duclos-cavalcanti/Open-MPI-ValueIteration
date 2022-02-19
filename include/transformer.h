#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "Eigen/Sparse"
#include "Eigen/Dense"

#include <vector>
#include <string>

#include "data.h"

typedef Eigen::SparseMatrix<float, Eigen::RowMajor> SparseMatrix_t;

/**
 * Struct that holds the prepared data to pass onto the
 * value iteration classes.
 */
struct Transformer {
  std::vector< std::vector<std::vector< std::pair<int, int> > > > coord_map; /**< Magical map that holds coordinates to nnz values */

  unsigned int nr_states_; /**< Number of States */
  unsigned int nr_stars_; /**< Number of stars */
  unsigned int nr_actions_; /**< Number of actions */

  Eigen::VectorXf J_; /**< Eigen::Vector that holds the current J of the class */
  Eigen::VectorXi Pi_; /**< Eigen::Vector thath holds the current Pi of the class */
  const Eigen::Map<SparseMatrix_t> P_; /**< Eigen::SparseMatrix thath holds the Probability matrix */

public:
  /** Transformer Constructor
  *
  * Takes in the raw data struct needed to construct J, P, Pi and the
  * coordinate map.
  */
  Transformer(struct Data& data):
    J_(data.shape_[1]),
    Pi_(data.shape_[1]),
    P_(data.shape_[0], data.shape_[1], 
       data.values_.size(), data.indptr_.data(), 
       data.indices_.data(), data.values_.data()) {

      J_.fill(0);
      Pi_.fill(0);

      nr_stars_ = data.nr_stars_;
      nr_states_ = data.nr_states_;
      nr_actions_ = data.nr_actions_;

      this->createCoordinateMap(data);
    }

private:
  void createCoordinateMap(struct Data& data) {
    coord_map.resize(data.nr_states_, std::vector<std::vector< std::pair<int, int>>>(data.nr_actions_));
    for (int r = 0; r < P_.outerSize(); ++r) { // iterates rows
      for(Eigen::InnerIterator<typeof(P_)> it(P_, r); it; ++it) { // iterate cols
        int row = r;
        int col = it.col();
        int action = row % data.nr_actions_;
        int state = row / data.nr_actions_;

        coord_map.at(state).at(action).push_back(std::make_pair(row, col));
      }
    }
  }
};

/** @}*/
#endif
