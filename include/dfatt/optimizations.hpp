#pragma once

#include <dfa_table.hpp>
#include <queue>

int get_no_of_states(int optimization){
    int raw_no_of_states = sizeof(yy_base)/sizeof(int16_t);
    // Removing external ghost states
    int reduced_no_of_states = yy_def[1]+1;
    if( optimization == 0 ) {
        return raw_no_of_states;
    }
    else {
        return reduced_no_of_states;
    }
}

int nextstate(int curr_state, uint8_t input) {
    uint8_t yy_c = yy_ec[input];
    while(yy_chk[yy_base[curr_state] + yy_c] != curr_state) {
        curr_state = yy_def[curr_state];
        if(curr_state >= yy_def[1]) yy_c = yy_meta[yy_c];
    }
    return yy_nxt[yy_base[curr_state] + yy_c];
}

std::vector<std::vector<int>> getDFA(int& no_of_states, std::vector<int>& symbols, int optimization) {

    std::vector<std::vector<int>> dfa(no_of_states + 1, std::vector<int>(symbols.size()));

    for(int curr_state = 1; curr_state < no_of_states; curr_state++) {
        for(size_t ind = 0; ind < symbols.size(); ind++) {
            dfa[curr_state][ind] = nextstate(curr_state, symbols[ind]);
        }
    }

    if(optimization == 0 || optimization == 1) return dfa;

    std::vector<bool> vis(no_of_states + 1, false);
    std::queue<int> q;

    q.push(1);
    vis[1] = true;

    while(!q.empty()) {
        int curr = q.front();
        q.pop();

        for(size_t ind = 0; ind < symbols.size(); ind++) {
            int nxt = dfa[curr][ind];

            if(nxt != 0 && !vis[nxt]) {
                vis[nxt] = true;
                q.push(nxt);
            }
        }
    }

    std::vector<int> mp(no_of_states + 1, 0);
    int new_states = 1;

    for(int i = 1; i <= no_of_states; i++) {
        if(vis[i]) {
            mp[i] = new_states++;
        }
    }

    std::vector<std::vector<int>> optimizedDFA(
        new_states,
        std::vector<int>(symbols.size(), 0)
    );

    for(int i = 1; i <= no_of_states; i++) {
        if(vis[i]) {
            for(size_t ind = 0; ind < symbols.size(); ind++) {
                int nxt = dfa[i][ind];

                optimizedDFA[mp[i]][ind] =  mp[nxt];
            }
        }
    }


    no_of_states = new_states;

    return optimizedDFA;
}