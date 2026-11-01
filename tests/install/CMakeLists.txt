# A program built against an installed arboris, the way a user's project would. Run by CI
# after `cmake --install`; see .github/workflows/build-and-test.yml.
cmake_minimum_required(VERSION 3.14)
project(arboris_install_check CXX)

find_package(arboris REQUIRED)

add_executable(install_check main.cc)
target_link_libraries(install_check PRIVATE arboris::arboris)
