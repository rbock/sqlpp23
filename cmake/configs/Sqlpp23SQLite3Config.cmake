include(CMakeFindDependencyMacro)
find_dependency(SQLite3)

include (${CMAKE_CURRENT_LIST_DIR}/Sqlpp23SQLite3Targets.cmake)
