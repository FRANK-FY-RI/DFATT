#pragma once

#include "dfa_table.hpp"

int get_no_of_states(int optimization){
    int raw_no_of_states = sizeof(yy_base)/sizeof(int16_t);
    // Removing external ghost states
    int reduced_no_of_states = yy_def[1];
    if( optimization == 0 ) {
        return raw_no_of_states;
    }
    else {
        return reduced_no_of_states;
    }
}