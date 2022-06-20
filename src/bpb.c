/*
Branch Prediction Buffer (BPB)

Considerations:
  BPB can be configured to usage desired amount bits to change prediction,
  changing N_BITS define and recompiling simulator.

  BPB start with state 0 ( not taken ), to change this initialize memory of bpb
  on function bpb_init to value desired.

How this work:
  on first bpb_get_prediction of branch 'x' is return false ( not taken ),
  after processing of branch 'x' bpb_update_prediction must be call
  passing if the branch was or not taken,

  case yes, state of branch is incremented by 1 if current state is less than 'max',
  case no, state of branch is decremented by 1 if current state is greater than 0

  return is, state & MSB (max),
  then if prediction was 2 bits (default) return ( state & 1 << ( 2 - 1 ) ),
  examples:
    state == 0 - return Not Taken
    state == 1 - return Not Taken
    state == 2 - return Taken
    state == 3 - return Taken
*/

#include <stdbool.h>
#include <stdlib.h>

#define DEFAULT_SIZE_BPB 10
#define N_BITS 2

#define MAX ( ( 1 << N_BITS ) -1 )
#define MSB ( 1 << ( N_BITS - 1 ) ) /* most significant bit */

static int *bpb;
static unsigned int size_bpb;

bool
bpb_init( unsigned int size )
{
  size_bpb = size ? size : DEFAULT_SIZE_BPB;

  bpb = calloc( size, sizeof *bpb );

  return !!bpb;
}

bool
bpb_get_prediction( unsigned int pc )
{
  return ( bpb[ pc % size_bpb ] & MSB );
}

void
bpb_update_prediction( unsigned int pc, bool pred )
{
  unsigned int i = pc % size_bpb;
  int state = bpb[i];

  if ( pred )
    state += ( state < MAX );
  else
    state -=  ( state > 0 );

  bpb[i] = state;
}

void
bpb_free( void )
{
  free( bpb );
}
