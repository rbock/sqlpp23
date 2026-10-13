include(CMakeFindDependencyMacro)
find_dependency(SQLCipher)

include (${CMAKE_CURRENT_LIST_DIR}/Sqlpp23SQLCipherTargets.cmake)
