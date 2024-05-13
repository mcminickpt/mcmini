#include "mcmini/model_checking/algorithms/classic_dpor/clock_vector.hpp"

using namespace model_checking;

using namespace std;

clock_vector clock_vector::max(const clock_vector &cv1,
                               const clock_vector &cv2) {
  clock_vector maxCV;
  for (const auto &e : cv2.contents)
    maxCV[e.first] = std::max(cv1.value_for(e.first), e.second);
  for (const auto &e : cv1.contents)
    maxCV[e.first] = std::max(cv2.value_for(e.first), e.second);
  return maxCV;
}
