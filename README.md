# waveTest
using boost/wave library to mimic gcc/cpp preprocessor

This is based on boost/wave/ quickstart example by adding gcc/cpplib with including path and all defined macro.

The list of macro is acquired by

g++ -std=c++11 -dM -E -x c++ - < /dev/null

And the search path is by copying directory list after "#include <...> search starts here:"

g++ -std=c++11 -E -v -x c++ - < /dev/null

The wave library has a builtin limit that it only allow pre-defined macro "__cplusplus" being defined as c++11 value which is "201103L". And this macro is considered as one of five "special macro" in boost/wave/util/cpp_macromap_utils.hpp:is_special_macroname. So, even you want to call "remove_macro_definition" and "add_macro_definition", library will generate error to prevent you from doing so. There seems to be no workaround. In general, this macro is not significent, but for some gcc c++XXX specific feature, it is fatal. One example is gcc/plugin which is very sensitive with versions. other example is like c++ modules etc.




