
```
# pre-compile the module
clang++ -std=c++23 module/sqlpp23.cppm --precompile -o sqlpp23.pcm -I$PWD/include

# compile the test program
clang++ -std=c++23 tests/core/modules/test.cpp -fmodule-file=sqlpp23=sqlpp23.pcm -o sqlpp23.out -I$PWD/tests/include -I$PWD/include
```
