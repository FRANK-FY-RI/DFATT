if [[ $# != 1 ]]; then
    echo "Usage: $0 <lex.yy.c>"
    exit 1
fi

gcc table_lexer.c -o table_lexer
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    exit 1
fi

# $1 is the lex file (.l) for which a Transition Table is to be made
touch dfa_table.hpp
echo "#include <cstdint>" >> dfa_table.hpp
echo "using YY_CHAR = unsigned int;" >> dfa_table.hpp
echo "using flex_int16_t = int16_t;" >> dfa_table.hpp
./table_lexer <$1 >>dfa_table.hpp

g++ main.cpp -o main
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    rm dfa_table.hpp
    exit 1
fi

./main

rm dfa_table.hpp
