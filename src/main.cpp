
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <array>

#include "dfatt/optimizations.hpp"

std::array<std::string, 256> load_symbol_names() {
    std::array<std::string, 256> names;
    std::ifstream file("data/symbol_names.tsv");
    int code;
    std::string name;
    while (file >> code >> name) {
        if (code >= 0 && code < static_cast<int>(names.size())) names[code] = name;
    }
    return names;
}

int main(int argc, char* argv[]) {
    const std::array<std::string, 256> symbol_names = load_symbol_names();

    int optimization = 0;

    if(argc == 1) {
        optimization = 0;
    }
    else if(argc == 2) {
        std::string flag = argv[1];

        if(flag == "-O1") {
            optimization = 1;
        }
        else if(flag == "-O2") optimization = 2;
        else if(flag == "-O0") optimization = 0;
        else if(flag == "-O3") optimization = 3;
        else {
            std::cerr << "Invalid flag\n";
            return 1;
        }
    }
    else {
        std::cerr << "Too many arguments\n";
        return 1;
    }

    std::vector<int> symbols;
    for(int i = 0; i<256; i++) {
        if(yy_ec[i] > 1) symbols.emplace_back(i);
    }

    int no_of_states = get_no_of_states(optimization);

    auto result = getDFA(no_of_states,symbols,optimization);

    std::vector<std::vector<int>> dfa = result.first;
    std::vector<int> finals = result.second;

    std::ofstream fout("build/tt.txt");
    if(!fout) {
        std::cout<<"Unable to open/create file tt.txt\n";
        return 1;
    }
    std::cout<<"State\t";
    fout<<no_of_states-1<<'\n'<<symbols.size()<<'\n';
    for(const int a:symbols) {
        fout<<a<<'\t';
        std::cout<<symbol_names[a]<<'\t';
    }
    fout<<'\n';
    std::cout<<'\n';

    for(int i = 1; i<no_of_states; i++) {
        fout<<i<<'\t';
        std::cout<<i<<'\t';
        for(size_t j = 0; j<symbols.size(); j++) {
            fout<<dfa[i][j]<<'\t';
            std::cout<<dfa[i][j]<<'\t';
        }
        fout<<'\n';
        std::cout<<'\n';
    }

    fout<<1<<"\n";
    for(auto&i:finals)
    {
        fout<<i<<" ";
    }
    fout<<"\n";

    return 0;
}
