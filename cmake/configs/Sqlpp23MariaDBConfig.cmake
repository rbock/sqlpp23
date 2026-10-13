include(CMakeFindDependencyMacro)
find_dependency(MariaDB)

include (${CMAKE_CURRENT_LIST_DIR}/Sqlpp23MariaDBTargets.cmake)
