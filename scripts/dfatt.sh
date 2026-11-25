#! /bin/bash

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <lex.yy.c> [ -O0 | -O1 | -O2 ]"
    exit 1
fi
INPUT_FILE="$(realpath "$1")"
cd "$ROOT_DIR"
mkdir -p build

if [[ $# > 2 ]]; then
    echo "Usage: $0 <lex.yy.c>"
    exit 1
fi

gcc src/table_lexer.c -o build/table_lexer
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    exit 1
fi
trap "rm -f build/table_lexer" EXIT

# $1 is the lex file (.l) for which a Transition Table is to be made
touch build/dfa_table.hpp
echo "#pragma once" >> build/dfa_table.hpp
trap "rm -f build/table_lexer build/dfa_table.hpp" EXIT
echo "#include <cstdint>" >> build/dfa_table.hpp
echo "using YY_CHAR = unsigned int;" >> build/dfa_table.hpp
echo "using flex_int16_t = int16_t;" >> build/dfa_table.hpp
build/table_lexer <"$INPUT_FILE" >>build/dfa_table.hpp

g++ -std=c++11 -Iinclude -Ibuild src/main.cpp -o build/main
status=$?
if [[ $status != 0 ]]; then
    echo "Compilation failed"
    exit 1
fi

trap "rm -f build/table_lexer build/dfa_table.hpp build/main" EXIT

if [[ $# == 1 ]]; then
    build/main
fi

if [[ $# == 2 ]]; then
    build/main "$2"
fi

trap "rm -f build/table_lexer build/dfa_table.hpp build/main build/tt.txt" EXIT

# save the plot of the DFA as an Image
python3 scripts/plot.py < build/tt.txt

echo "The Automaton generated for the given Regular Expression is saved as build/graph.png"
