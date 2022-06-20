#ifndef BPB_H
#define BPB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
* @brief Init BPB and set to first stage note taken.
*
* @param Number of positions on BPB
*/
bool
bpb_init( unsigned int size );

/*
* @brief Get prediction of instruction
*
* @param PC ( Program Counter ) of instruction to get prediction
* @return true if prediction is taken or false
*/
bool
bpb_get_prediction( unsigned int pc );

/*
* @brief Update state of predidiction ot instruction
*
* @param pc of instruction
* @parm pred, if instruction was or not actually taken
*/
void
bpb_update_prediction( unsigned int pc, bool pred );

/*
* @brief Free memory of BPB
*/
void
bpb_free( void );

#ifdef __cplusplus
}
#endif

#endif // BPB_H
