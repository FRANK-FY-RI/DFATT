
#pragma once

#include <dfa_table.hpp>
#include <algorithm>
#include <cstdint>
#include <map>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

using DFAResult = std::pair<std::vector<std::vector<int>>, std::vector<int>>;

int get_no_of_states(int optimization) {
    int raw = sizeof(yy_base) / sizeof(yy_base[0]);
    int reduced = yy_def[1] + 1;

    return optimization == 0 ? raw : reduced;
}

int nextstate(int state, uint8_t input) {
    uint8_t c = yy_ec[input];

    while (yy_chk[yy_base[state] + c] != state) {
        state = yy_def[state];

        if (state >= yy_def[1])
            c = yy_meta[c];
    }

    return yy_nxt[yy_base[state] + c];
}

int accept_of(int state) {
    int n = sizeof(yy_accept) / sizeof(yy_accept[0]);

    if (state < 0 || state >= n)
        return 0;

    return yy_accept[state]==1;
}

DFAResult minimizeDFA(const std::vector<std::vector<int>>& dfa,
                      const std::vector<int>& accept) {
    int n = dfa.size();
    int m = n ? dfa[0].size() : 0;

    std::vector<int> group(n, 0);
    int groups = 1;

    while (true) {
        std::map<std::vector<int>, int> mp;
        std::vector<int> newGroup(n);

        for (int i = 0; i < n; i++) {
            std::vector<int> key;

            key.push_back(group[i]);
            key.push_back(accept[i]);

            for (int j = 0; j < m; j++) {
                int nxt = dfa[i][j];

                if (nxt < 0 || nxt >= n)
                    nxt = 0;

                key.push_back(group[nxt]);
            }

            if (mp.find(key) == mp.end()) {
                int id = mp.size();
                mp[key] = id;
                newGroup[i] = id;
            } else {
                newGroup[i] = mp[key];
            }
        }

        int cnt = mp.size();
        group = newGroup;

        if (cnt == groups)
            break;

        groups = cnt;
    }

    int start = group[1];
    int dead = group[0];

    std::vector<int> id(groups, -1);

    id[dead] = 0;
    int nxt = 1;

    if (start != dead)
        id[start] = nxt++;

    for (int i = 0; i < groups; i++) {
        if (id[i] == -1)
            id[i] = nxt++;
    }

    int total = nxt;

    std::vector<int> rep(groups, -1);

    for (int i = 0; i < n; i++) {
        if (rep[group[i]] == -1)
            rep[group[i]] = i;
    }

    std::vector<std::vector<int>> ans(total, std::vector<int>(m, 0));
    std::vector<int> finals;

    for (int i = 0; i < groups; i++) {
        int s = rep[i];

        for (int j = 0; j < m; j++) {
            int to = dfa[s][j];

            if (to < 0 || to >= n)
                to = 0;

            ans[id[i]][j] = id[group[to]];
        }

        if (id[i] != 0 && accept[s] != 0)
            finals.push_back(id[i]);
    }

    sort(finals.begin(), finals.end());

    return {ans, finals};
}

DFAResult getDFA(int& no_of_states,
                        const std::vector<int>& symbols,
                        int optimization) {

    std::vector<std::vector<int>> dfa(
        no_of_states + 1, std::vector<int>(symbols.size(), 0));

    for (int i = 1; i < no_of_states; i++) {
        for (int j = 0; j < symbols.size(); j++) {
            dfa[i][j] = nextstate(i, symbols[j]);
        }
    }

    if (optimization == 0 || optimization == 1) {
        dfa.resize(no_of_states);

        std::vector<int> finals;

        for (int i = 1; i < no_of_states; i++) {
            if (accept_of(i))
                finals.push_back(i);
        }

        return {dfa, finals};
    }

    std::vector<bool> vis(no_of_states + 1, false);
    std::queue<int> q;

    q.push(1);
    vis[1] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int j = 0; j < symbols.size(); j++) {
            int v = dfa[u][j];

            if (v != 0 && !vis[v]) {
                vis[v] = true;
                q.push(v);
            }
        }
    }

    std::vector<int> mp(no_of_states + 1, 0);
    int cnt = 1;

    for (int i = 1; i <= no_of_states; i++) {
        if (vis[i])
            mp[i] = cnt++;
    }

    std::vector<std::vector<int>> ans(
        cnt, std::vector<int>(symbols.size(), 0));

    std::vector<int> accept(cnt, 0);

    for (int i = 1; i <= no_of_states; i++) {
        if (!vis[i])
            continue;

        for (int j = 0; j < symbols.size(); j++) {
            ans[mp[i]][j] = mp[dfa[i][j]];
        }

        accept[mp[i]] = accept_of(i);
    }

    if (optimization == 3) {
        DFAResult res = minimizeDFA(ans, accept);

        no_of_states = res.first.size();

        return res;
    }

    std::vector<int> finals;

    for (int i = 1; i < cnt; i++) {
        if (accept[i])
            finals.push_back(i);
    }

    no_of_states = cnt;

    return {ans, finals};
}
