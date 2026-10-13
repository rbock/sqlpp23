include(CMakeFindDependencyMacro)
find_dependency(PostgreSQL)

include (${CMAKE_CURRENT_LIST_DIR}/Sqlpp23PostgreSQLTargets.cmake)
