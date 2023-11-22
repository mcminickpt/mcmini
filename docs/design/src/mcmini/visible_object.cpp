#include "mcmini/model/visible_object.hpp"

using namespace mcmini::model;

visible_object::visible_object(const visible_object &other) {
  *this = std::move(other.clone());
}

visible_object &visible_object::operator=(const visible_object &other) {
  return *this = std::move(other.clone());
}