
```
# pre-compile the module
clang++ -std=c++23 module/sqlpp23_postgresql.cppm -fmodule-file=sqlpp23=sqlpp23.pcm --precompile -o sqlpp23_postgresql.pcm -I$PWD/include -I/usr/include/postgresql

# compile the test program
clang++ -std=c++23 tests/postgresql/modules/test.cpp -fmodule-file=sqlpp23=sqlpp23.pcm -fmodule-file=sqlpp23_postgresql=sqlpp23_postgresql.pcm -o sqlpp23_postgresql.out -I$PWD/tests/include -I$PWD/include -lpq
```
