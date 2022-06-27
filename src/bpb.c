/*
Branch Prediction Buffer (BPB)

Considerations:
  BPB can be configured to usage desired amount bits to change prediction,
  changing N_BITS define and recompiling simulator.

  BPB start with state 1 ( not taken weak ), to change this initialize memory of bpb
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
#include <stdlib.h>   // malloc / free
#include <stdint.h>   // uint8_t
#include <string.h>   // memset

#define DEFAULT_SIZE_BPB 10
#define N_BITS 2 // set one or two

#define MAX ( ( 1 << N_BITS ) -1 )
#define MSB ( 1 << ( N_BITS - 1 ) ) /* most significant bit */

static uint8_t *bpb;
static unsigned int size_bpb;

static unsigned int tot_predictions;
static unsigned int tot_hit;

bool
bpb_init( unsigned int size )
{
  size_bpb = size ? size : DEFAULT_SIZE_BPB;
  size_t len = size_bpb * sizeof *bpb;

  bpb = malloc( len );

  if ( bpb )
    memset( bpb, N_BITS - 1, len );

  return bpb;
}

bool
bpb_get_prediction( unsigned int pc )
{
  tot_predictions++;
  return ( bpb[ pc % size_bpb ] & MSB );
}

void
bpb_update_prediction( unsigned int pc, bool taken, bool hit )
{
  unsigned int i = pc % size_bpb;
  uint8_t state = bpb[i];

  tot_hit += hit;

  if ( taken )
    state += ( state < MAX );
  else
    state -=  ( state > 0 );

  bpb[i] = state;
}

float
bpb_get_hit_rate( void )
{
  return ( float ) tot_hit / ( float ) tot_predictions * 100.0;
}

void
bpb_free( void )
{
  free( bpb );
}
