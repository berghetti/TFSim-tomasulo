#include "branch_predictor.hpp"

branch_predictor::branch_predictor(unsigned int t): n_bits(t)
{
    max = (1<<n_bits)-1;
    state = 1; // initial state 0 is not taken??
}

bool branch_predictor::predict()
{
  // TODO: logic BPB probably should be implemented here
  //       need pc original of insturctions
    return state&(1<<(n_bits-1));
}
void branch_predictor::update_state(bool taken)
{
    if(taken)
    {
        state = ++state>max?max:state;
    }
    else if(state)
    {
        state = --state<0?0:state;
    }
}
