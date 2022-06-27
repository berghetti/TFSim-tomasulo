class branch_predictor
{
public:
    branch_predictor(unsigned int t);
    bool predict();
    void update_state(bool taken, bool hit);

private:
    int n_bits,max,state;
};
