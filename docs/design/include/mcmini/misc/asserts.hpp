#pragma once

#include <stdexcept>
#include <string>

struct asserts final {
  /* Prevent construction */
  asserts() = delete;
  asserts(asserts &) = delete;
  asserts(asserts &&) = delete;

  /**
   * @brief Asserts that condition _cond_ is enforced, or else raises an
   * exception.
   *
   * @param cond        the condition that should be satisfied.
   * @param why_enforced a human-readable explanation for why _cond_ is enforced
   * by the caller.
   * @throws std::invalid_argument if cond is unsatisfied
   */
  static void assert_condition(bool cond, std::string why_enforced) {
    if (!cond) {
      throw std::invalid_argument(why_enforced);
    }
  }

  /**
   * @brief Asserts that condition _cond_ is enforced, or else raises an
   * exception.
   *
   * @param cond        the condition that should be satisfied.
   * @param why_enforced a human-readable explanation for why _cond_ is enforced
   * by the caller.
   * @throws std::invalid_argument if cond is unsatisfied
   */
  static void assert_condition(bool cond, const char *why_enforced) {
    assert_condition(cond, std::string(why_enforced));
  }

  /**
   * @brief Asserts that condition _cond_ is always satisfied due to program
   * invariants.
   *
   * @param cond         the condition that should be satisfied.
   * @param why_invariant a human-readable explanation for why _cond_ is never
   * _false_
   * @throws std::logic_error if cond is unsatisfied or if whyEnforced is
   * null
   */
  static void assert_invariant(bool cond, std::string why_invariant) {
    if (!cond) {
      throw std::logic_error(why_invariant);
    }
  }

  /**
   * @brief Asserts that condition _cond_ is always satisfied due to program
   * invariants.
   *
   * @param cond         the condition that should be satisfied.
   * @param why_invariant a human-readable explanation for why _cond_ is never
   * _false_
   * @throws std::logic_error if cond is unsatisfied or if whyEnforced is
   * null
   */
  static void assert_invariant(bool cond, const char *why_invariant) {
    assert_invariant(cond, std::string(why_invariant));
  }
};
