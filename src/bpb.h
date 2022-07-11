#ifndef BPB_H
#define BPB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
* @brief Init BPB.
*
* @param Number of positions on BPB
*/
bool
bpb_init( unsigned int size );

/*
* @brief Get prediction to branch
*
* @param PC ( Program Counter ) of branch to get prediction
* @return true if prediction is taken or false
*/
bool
bpb_get_prediction( unsigned int pc );

/*
* @brief Update state of predidiction ot instruction
*
* @param pc of branch
* @param taken, if branch was or not taken
* @param hit, true (1) if prediction was correct or false (0)
*/
void
bpb_update_prediction( unsigned int pc, bool taken, bool hit );

/*
* @brief Return sucess rate BPB
*/
float
bpb_get_hit_rate( void );

/*
* @brief Free memory of BPB table
*/
void
bpb_free( void );

#ifdef __cplusplus
}
#endif

#endif // BPB_H
