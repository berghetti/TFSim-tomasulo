#include "branch_predictor.hpp"

static unsigned int tot_predictions = 0;
static unsigned int tot_hit = 0;

branch_predictor::branch_predictor(unsigned int t): n_bits(t)
{
    max = (1<<n_bits)-1;
    state = 1; // initial state not taken

}

bool branch_predictor::predict()
{
    tot_predictions++;
    return state&(1<<(n_bits-1));
}
void branch_predictor::update_state(bool taken, bool hit)
{
    tot_hit += hit;

    if(taken)
    {
        state = ++state>max?max:state;
    }
    else if(state)
    {
        state = --state<0?0:state;
    }
}

float
predictor_get_hit_rate( void )
{
  return ( float ) tot_hit / ( float ) tot_predictions * 100.0;
}
