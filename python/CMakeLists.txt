# The Python extension module, built by scikit-build-core (see pyproject.toml) or by hand
# with -DARBORIS_PYTHON=ON. nanobind comes from the Python environment: `pip install
# nanobind`, then -Dnanobind_DIR=$(python -m nanobind --cmake_dir) when building by hand.
find_package(Python 3.10 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind CONFIG REQUIRED)

# The static library goes into a shared module, so its objects have to be PIC. Set on the
# target: CMAKE_POSITION_INDEPENDENT_CODE applies only to targets created after it, and
# arboris already exists by the time python/ is added.
set_target_properties(arboris PROPERTIES POSITION_INDEPENDENT_CODE ON)

nanobind_add_module(_arboris NB_STATIC src/arboris_module.cc)
target_link_libraries(_arboris PRIVATE arboris)
target_include_directories(_arboris PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(_arboris PRIVATE ARBORIS_VERSION="${PROJECT_VERSION}")

install(TARGETS _arboris LIBRARY DESTINATION arboris COMPONENT python)
