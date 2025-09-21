/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTILS_ASSERTION_HPP_
#define SRC_UTILS_ASSERTION_HPP_

#if defined(DEBUG)
#include <iostream>
#endif

#if defined(DEBUG)
#define ARBORIS_ASSERT(cond, msg)                            \
  do {                                                       \
    if (!(cond)) {                                           \
      std::cerr << "Assertion failed: " << std::endl         \
                << " - condition : " << #cond << std::endl \
                << " - message   : " << msg << std::endl   \
                << " - at        : " << __FILE__ << ":" << __LINE__ << " in " << __func__ << std::endl; \
      std::abort(); \
    } \
  } \
  while (0)
#else
#define ARBORIS_ASSERT(cond, msg)
#endif

#endif  // SRC_UTILS_ASSERTION_HPP_
