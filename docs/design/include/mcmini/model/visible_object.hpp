#pragma once

#include <stdint.h>

#include <memory>

namespace mcmini::model {

/**
 * A placeholder which represents something that multiple threads interact with
 * to communicate with each other in a program.
 *
 * A _visible object_, from the perspective of a model checker, are those
 * portions of a program which are semantically interesting with respect to
 * verification. Threads in a program interact with one another by communicating
 * somehow through something which is accessible to each thread communicating
 * (hence the idea of "visible"). Data used locally by threads in a program is
 *
 * This class presents a model
 *
 */
class visible_object {
 public:
  /**
   * The identifier type used to specify and differentiate objects between one
   * another in a given simulation.
   *
   * Model-checking involves specifying objects of a different
   *
   */
  using id = uint32_t;

  /**
   *
   *
   */
  visible_object::id get_identifier_in_modelchecking_model() const;

  /**
   * Create a duplicate of the given object with the same id.
   *
   * @return a pointer to a new object with the same contents as this one. The
   * returned object is assigned the same identifier as the given one, and can
   * hence be used to represent this object in the same context.
   */
  virtual std::unique_ptr<visible_object> clone() = 0;
};

}  // namespace mcmini::model