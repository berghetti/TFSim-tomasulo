#ifndef BPB_H
#define BPB_H

#ifdef __cplusplus
extern "C" {
#endif

bool
bpb_init( unsigned int size );

bool
bpb_get_prediction( unsigned int pc );

void
bpb_update_prediction( unsigned int pc, bool pred );

void
bpb_free( void );

#ifdef __cplusplus
}
#endif

#endif // BPB_H
