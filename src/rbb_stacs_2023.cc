/* Code for producing the experiments in Section 6 of "Tight Bounds for Repeated Balls-Into-Bins" */
#include <iostream>
#include <random>

/* Runs the Repeated Balls-into-Bins (RBB) process. This process was introduced in
*     "Self-Stabilizing Repeated Balls-into-Bins", 
         by Becchetti, Clementi, Natale, Pasquale and Posta (2015)
         [https://arxiv.org/abs/1501.04822].
   
   It starts from an arbitrary load vector with n bins and m balls. In each round:
     1. Removes one ball from each non-empty bin.
     2. Allocates these balls uniformly at random to the n bins.
   
   This class keeps track of the load-vector, the maximum load and the number of 
   empty bins.
   */
class RepeatedBallsIntoBins {
public:

  /* Initializes the RBB process with the given load vector. */
  RepeatedBallsIntoBins(const std::vector<size_t>& load_vector) 
    : load_vector_(load_vector), max_load_(0), num_empty_bins_(0), bin_uar_(0, load_vector.size() - 1) { 
    for (const auto val : load_vector_) {
      max_load_ = std::max(max_load_, val);
      num_empty_bins_ += (val == 0);
    }
  }
  
  /* Performs one round of the RBB process. */
  template<typename T>
  void nextRound(T& generator) {
    // Phase 1: Remove one ball from each bin.
    size_t n = load_vector_.size();
    size_t balls_to_allocate = n - num_empty_bins_;
    max_load_ = 0;
    num_empty_bins_ = 0;
    for (size_t i = 0; i < n; ++i) {
      if (load_vector_[i] > 0) {
        --load_vector_[i];
      }
      max_load_ = std::max(max_load_, load_vector_[i]);
      num_empty_bins_ += (load_vector_[i] == 0);
    }

    // Phase 2: Allocate these balls randomly among the bins.
    for (int j = 0; j < balls_to_allocate; ++j) {
      size_t i = bin_uar_(generator);
      ++load_vector_[i];
      max_load_ = std::max(max_load_, load_vector_[i]);
      num_empty_bins_ -= (load_vector_[i] == 1);
    }
  }

  /* Returns the current maximum load. */
  size_t getMaxLoad() const {
    return max_load_;
  }

  /* Returns the current number of empty bins. */
  size_t getNumEmptyBins() const {
    return num_empty_bins_;
  }

  /* Returns the current load vector. */
  std::vector<size_t> getLoadVector() const {
    return load_vector_;
  }

private:

  /* Current load vector of the process. */
  std::vector<size_t> load_vector_;

  /* Number of currently empty bins in the load vector. */
  size_t num_empty_bins_;

  /* Current maximum load in the load vector. */
  size_t max_load_;

  /* The uniform distribution that samples one of the n bins. */
  std::uniform_int_distribution<size_t> bin_uar_;
};

/* Generates a uniform load vector for the given number of bins and balls. */
std::vector<size_t> generate_uniform_vector(size_t num_bins, size_t num_balls) {
  std::vector<size_t> load_vector(num_bins, num_balls/num_bins);
  size_t rem = num_balls % num_bins;
  for (size_t i = 0; i < rem; ++i) {
    ++load_vector[i];
  }
  return load_vector;
}

/* Auxiliary function for running RBB experiments for a given number of bins
   and a given number of balls. */
std::pair<double, double> run_experiments_for_n_and_m(size_t num_bins, size_t num_balls, size_t num_rounds, size_t num_repetitions) {
  std::mt19937_64 generator;
  size_t aggregate_max_loads = 0;
  size_t aggregate_num_empty_bins = 0;
  for (size_t rep = 0; rep < num_repetitions; ++rep) {
    auto load_vector = generate_uniform_vector(num_bins, num_balls);
    RepeatedBallsIntoBins rbb(load_vector);
    for (size_t round = 0; round < num_rounds; ++round) {
      rbb.nextRound(generator);
      aggregate_max_loads += rbb.getMaxLoad();
      aggregate_num_empty_bins += rbb.getNumEmptyBins();
    }
  }
  double avg_max_load = aggregate_max_loads / double(num_rounds * num_repetitions);
  double avg_num_empty_bins = aggregate_num_empty_bins / double(num_rounds * num_repetitions);
  return { avg_max_load, avg_num_empty_bins };
}

/* Runs the RBB process for n in {10^2, 10^3, 10^4} for 10^6 rounds starting with the
   uniform load vector with number of balls m in {n, 4n, ... , 52n}. */
void run_experiments(size_t num_repetitions) {
  std::vector<size_t> num_bins_all({ 100, 1'000, 10'000 });
  size_t num_rounds = 1'000'000;
  for (const auto num_bins : num_bins_all) {
    std::cout << "Results for " << num_bins << " bins:" << std::endl;
    std::vector<std::pair<size_t, double>> avg_max_load_collection, avg_num_empty_bin_collection;
    for (size_t scale_factor = 1; scale_factor <= 52; scale_factor += 3) {
      size_t num_balls = scale_factor * num_bins;
      auto [avg_max_load, avg_num_empty_bins] = run_experiments_for_n_and_m(num_bins, num_balls, num_rounds, num_repetitions);
      avg_max_load_collection.push_back({ scale_factor, avg_max_load });
      avg_num_empty_bin_collection.push_back({ scale_factor, avg_num_empty_bins });
    }
    // Output the (average) maximum loads.
    for (const auto [scale_factor, avg_max_load] : avg_max_load_collection) {
      std::cout << "(" << scale_factor << ", " << avg_max_load << ")" << std::endl;
    }
    // Output the (average) number of empty bins. 
    for (const auto [scale_factor, avg_num_empty_bins] : avg_num_empty_bin_collection) {
      std::cout << "(" << scale_factor << ", " << avg_num_empty_bins/double(num_bins) << ")" << std::endl;
    }
  }
}


int main() {
  run_experiments(25);
  return 0;
}
