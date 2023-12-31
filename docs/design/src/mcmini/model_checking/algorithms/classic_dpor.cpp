#include "mcmini/model_checking/algorithms/classic_dpor.hpp"

#include <iostream>

using namespace mcmini::model_checking;

void classic_dpor::verify_using(coordinator &coordinator,
                                const callbacks &callbacks) {
  std::cout << "DPOR is running here! Yay!!" << std::endl;
}