#ifndef VALUE_ITERATION_H
#define VALUE_ITERATION_H

#include <utility>
#include <vector>
#include <string>
#include <sstream>

#include "Eigen/Sparse"
#include "Eigen/Dense"

#include <mpi.h>
#include <chrono>

#include "cnpy.h"
#include "data.h"
#include "transformer.h"
#include "timer.h"

/**
 * helping typedef for SparseMatrix, float, RowMajor
 */
typedef Eigen::SparseMatrix<float, Eigen::RowMajor> SparseMatrix_t;

/**
 * Base class for all implementations
 */
class ValueIterator {
protected:
  const unsigned int max_iter_ = 1e6; /**< maximum number of iterations */
  const float alpha_ = 0.99;          /**< alpha parameter */
  const float tolerance_ = 1e-8;      /**< minimum error tolerance */ 

  unsigned int nr_states_; /**< number of states */
  unsigned int nr_stars_; /**< number of stars */
  unsigned int nr_actions_; /**< number of actions */
  unsigned int nr_runs_; /**< number of runs the class performed */
  int freq_; /**< frequency in which this class performs updates during the value iteration */

  float comm_time_; /**< Period which the class is performing any communication */

  std::vector<float> total_times_; /**< Vector that holds the execution times of all runs */
  std::vector<float> comm_times_; /**< Vector that holds the communication times of all runs */
  std::vector< float > Js_; /**< Vector that holds the final J's of all runs */
  std::vector< int > Pis_; /**< Vector that holds the final Pi's of all runs */

  Eigen::VectorXf J_; /**< Eigen::Vector that holds the current J of the class */
  Eigen::VectorXi Pi_; /**< Eigen::Vector thath holds the current Pi of the class */
  const Eigen::Map<SparseMatrix_t> P_; /**< Eigen::SparseMatrix thath holds the Probability matrix */
  std::vector< std::vector<std::vector< std::pair<int, int> > > > coord_map; /**< Magical map that holds coordinates to nnz values */
public:

  /** ValueIterator Constructor
  *
  * Receives the Data struct which is needed to initialize all the dimensions
  * and values necessary to the Class. Here is where J, Pi and the probability
  * matrix are created. Same goes to other internal variables. Also sets the
  * frequeny of updates through the overloaded frequency variable.
  */
  ValueIterator(struct Transformer& data, int frequency):
    J_(data.J_),
    Pi_(data.Pi_),
    P_(data.P_),
    coord_map(data.coord_map){

    nr_stars_ = data.nr_stars_;
    nr_states_ = data.nr_states_;
    nr_actions_ = data.nr_actions_;

    nr_runs_ = 0;
    comm_time_ = 0;
    freq_ = frequency;
  }

  /**
   * Public function that stores all the results associated to this Class.
   * @return: void
   */
  void storeValues(std::string& dataset) {
    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &nproc); // Number of processes

    if (rank != 0) // only master should store values
      return;

    std::vector<int> nr_states(1, this->nr_states_);
    std::vector<int> nr_actions(1, this->nr_actions_);
    std::vector<int> nr_runs(1, this->nr_runs_);
    std::vector<int> freq(1, this->freq_);
    std::vector<int> nr_proc(1, nproc);
    std::string filename = "../results/" + dataset + "/" + this->getName() + "_" + std::to_string(nproc) + "-" + std::to_string(freq_) + ".npz";

    cnpy::npz_save(filename, 
                   "num_runs", 
                   nr_runs.data(), 
                   {1}, 
                   "w");

    cnpy::npz_save(filename, 
                   "num_procs", 
                   nr_proc.data(), 
                   {1}, 
                   "a");

    cnpy::npz_save(filename, 
                   "frequency", 
                   freq.data(), 
                   {1}, 
                   "a");

    cnpy::npz_save(filename, 
                   "nr_states", 
                   nr_states.data(), 
                   {1}, 
                   "a");

    cnpy::npz_save(filename, 
                   "nr_actions", 
                   nr_actions.data(), 
                   {1}, 
                   "a");

    cnpy::npz_save(filename,
                   "execution_times", 
                   total_times_.data(), 
                   {total_times_.size()}, 
                   "a");

    cnpy::npz_save(filename, 
                   "comm_times", 
                   comm_times_.data(), 
                   {comm_times_.size()}, 
                   "a");

    cnpy::npz_save(filename, 
                   "trajectory", 
                   Js_.data(), 
                   {nr_runs_, this->getTrajectory().size()}, 
                   "a");

    cnpy::npz_save(filename, 
                   "policy", 
                   Pis_.data(), 
                   {nr_runs_, this->getPolicy().size()}, 
                   "a");

  }

  /**
   * Public function that initiates the Class value iteration with it's corresponding MPI scheme.
   * @return: void
   */
  void iterateValues(void) {
    int rank;
    const int master = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    Timer timer;
    
    timer.start();

    this->implementation(); // actual scheme implementation

    const double diff = timer.diff();

    if(rank == master) {
      total_times_.push_back(diff);
      comm_times_.push_back(this->comm_time_);
      this->appendTrajectory();
      this->appendPolicy();

      ++this->nr_runs_;

      this->comm_time_ = 0;
      this->J_.fill(0);
      this->Pi_.fill(0);
    }
  };

protected:

  /**
   * This function is called from within iterateValues() and contains the actual scheme implementation.
   * This is a virtual function and has to be overwritten by every child iterator class.
   * @return: void
   * @see void iterateValues(void)
   */
  virtual void implementation(void) = 0;

  /**
   * This function is called from within implementation() and contains the code that iterates through
   * value associated to the states in the begin to end range. Also contains the updating of said values.
   * This is a virtual function and has to be overwritten by every child iterator class.
   * @return: void
   * @see void implementation(void)
   */
  virtual void partialValueIteration(const int begin, const int end) = 0;

  /**
   * This function returns the error associated to the overloaded range of states.
   * @param begin: unsigned int of the specific begin state
   * @param end: unsigned int of the specific end state
   * @return error: float is the maximum error over all errors of each state
   */
  float step(const unsigned int begin, const unsigned int end) {
    float error = 0, c;
    unsigned int fuel, goal_star, current_star;

    for (unsigned int state = begin; state < end; ++state) {
      const auto& slice = P_.middleRows(state * nr_actions_, nr_actions_);

      fuel = state / (nr_stars_ * nr_stars_);
      goal_star = state % (nr_stars_ * nr_stars_) / nr_stars_;
      current_star = state % (nr_stars_ * nr_stars_) % nr_stars_;

      int min_action = 0;
      float min_cost = std::numeric_limits<float>::max();

      for (unsigned int a = 0; a < nr_actions_; ++a) { // iterate through actions
        float cost_action = 0;
        if (coord_map[state][a].size() > 0) {
          for(auto& coord : coord_map[state][a]) {
            float prob = P_.coeff(coord.first, coord.second);
            if (goal_star == current_star && a == 0) {// reached the goal star
                c = -100;
            } else if (fuel == 0) { // out of fuel
                c = 100;
            } else if (a > 0) { // avoid unnecessary jumps
                c = 5;
            } else { // else no cost
                c = 0;
            }
            cost_action += prob * (c + alpha_ * J_[coord.second]);
          }
          if(cost_action < min_cost) {
              min_cost = cost_action;
              min_action = a;
          }
        }
      }
      error = std::max(error, std::abs(min_cost - J_[state]));
      J_[state] = min_cost;
      Pi_[state] = min_action;
    }

    return error;
  }
      
  /**
   * This function returns the string with the classes name
   * @return name: string
   */
  std::string getName(void) {
      return typeid(*this).name();
  };

  /**
   * This function returns the J float vector which is the trajectory
   * @return J: float vector 
   */
  std::vector<float> getTrajectory(void) {
    std::vector<float> J;
    J.assign(J_.data(), J_.data() + J_.size());
    return J;
  }

  /**
   * Appends current calculated J/Trajectory into the Js_ vector.
   * @return: void
   */
  void appendTrajectory(void) {
    std::vector<float> J = this->getTrajectory();
    Js_.insert(Js_.end(), J.begin(), J.end());
  }

  /**
   * This function returns the Pi int vector which is the policy
   * @return Pi: int vector 
   */
  std::vector<int> getPolicy(void) {
    std::vector<int> Pi;
    Pi.assign(Pi_.data(), Pi_.data() + Pi_.size());
    return Pi;
  }

  /**
   * Appends current calculated Pi/Policy into the Pis_ vector.
   * @return: void
   */
  void appendPolicy(void) {
    std::vector<int> P = this->getPolicy();
    Pis_.insert(Pis_.end(), P.begin(), P.end());
  }



private:

};

#endif
