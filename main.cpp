#include <vector>
#include <fstream>
#include <iostream>
#include <cstdint>
#include "dfa_table.hpp"


int nextstate(int curr_state, uint8_t input) {
    uint8_t yy_c = yy_ec[input]; 
    while(yy_chk[yy_base[curr_state] + yy_c] != curr_state) {
        curr_state = yy_def[curr_state];
        if(curr_state >= 10) yy_c = yy_meta[yy_c];
    }
    return yy_nxt[yy_base[curr_state] + yy_c];
}

int main() {

    std::vector<int> symbols;
    for(int i = 0; i<256; i++) {
        if(yy_ec[i] > 1) symbols.emplace_back(i);
    }

    const int no_of_states = sizeof(yy_base)/sizeof(int16_t);

    std::vector<std::vector<int>> dfa(no_of_states+1, std::vector<int>(symbols.size()));

    for(int curr_state = 1; curr_state<no_of_states; curr_state++) {
        for(size_t ind = 0; ind<symbols.size(); ind++) {
            dfa[curr_state][ind] = nextstate(curr_state, symbols[ind]);
        }
    }

    std::ofstream fout("tt.txt");
    if(!fout) {
        std::cout<<"Unable to open/create file tt.txt\n";
        return 1;
    }
    std::cout<<"State\t"; 
    fout<<no_of_states-1<<'\n'<<symbols.size()<<'\n';
    for(const char a:symbols) {
        fout<<a<<'\t';
        std::cout<<a<<'\t';
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
     
    return 0;
}

