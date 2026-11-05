#! /bin/bash

set -e

if [[ $# > 2 ]]; then
    echo "Usage: $0 <lex.yy.c>"
    exit 1
fi

gcc table_lexer.c -o table_lexer
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    exit 1
fi
trap "rm table_lexer" EXIT

# $1 is the lex file (.l) for which a Transition Table is to be made
touch dfa_table.hpp
echo "#pragma once" >> dfa_table.hpp
trap "rm table_lexer; rm dfa_table.hpp" EXIT
echo "#include <cstdint>" >> dfa_table.hpp
echo "using YY_CHAR = unsigned int;" >> dfa_table.hpp
echo "using flex_int16_t = int16_t;" >> dfa_table.hpp
./table_lexer <$1 >>dfa_table.hpp

g++ -std=c++11 main.cpp -o main
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    exit 1
fi

if [[ $# == 1 ]]; then
    ./main
fi

if [[ $# == 2 ]]; then
    ./main $2
fi

trap "rm table_lexer; rm dfa_table.hpp; rm main" EXIT
./main
trap "rm table_lexer; rm dfa_table.hpp; rm main; rm tt.txt" EXIT

# save the plot of the DFA as an Image
python3 plot.py < tt.txt

echo "The Automaton generated for the given Regular Expression is saved as graph.png"
