/*
 *   Copyright 2026 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef TESTS_INSTALL_SRC_UTILS_TAG_HPP_
#define TESTS_INSTALL_SRC_UTILS_TAG_HPP_

// A user's own utils/tag.hpp, on the include path before arboris's headers: it must never be
// what an arboris header gets when it asks for its Tag.
namespace app {
struct Tag {};
}  // namespace app

#endif  // TESTS_INSTALL_SRC_UTILS_TAG_HPP_
