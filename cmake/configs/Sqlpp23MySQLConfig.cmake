include(CMakeFindDependencyMacro)
find_dependency(MySQL)

include (${CMAKE_CURRENT_LIST_DIR}/Sqlpp23MySQLTargets.cmake)
