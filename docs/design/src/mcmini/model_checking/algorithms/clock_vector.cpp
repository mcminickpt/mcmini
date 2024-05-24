#include "mcmini/model_checking/algorithms/classic_dpor/clock_vector.hpp"

#include <algorithm>

using namespace model_checking;

using namespace std;

clock_vector clock_vector::max(const clock_vector &cv1,
                               const clock_vector &cv2) {
  clock_vector max_cv;
  for (const auto &e : cv2.contents)
    max_cv[e.first] = std::max(cv1.value_for(e.first), e.second);
  for (const auto &e : cv1.contents)
    max_cv[e.first] = std::max(cv2.value_for(e.first), e.second);
  return max_cv;
}
