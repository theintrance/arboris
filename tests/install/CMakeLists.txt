# A program built against an installed arboris, the way a user's project would. Run by CI
# after `cmake --install`; see .github/workflows/build-and-test.yml.
cmake_minimum_required(VERSION 3.14)
project(arboris_install_check CXX)

find_package(arboris REQUIRED)

add_executable(install_check main.cc)
# The project's own src/ comes before the package's -isystem include directory: its
# utils/tag.hpp must not be mistaken for arboris's.
target_include_directories(install_check PRIVATE src)
target_link_libraries(install_check PRIVATE arboris::arboris)
