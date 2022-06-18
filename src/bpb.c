/* Branch Prediction Buffer */

#include <stdbool.h>
#include <stdlib.h>

#define DEFAULT_LEN_BPB 10
#define DEFAULT_NBITS 2

// initial predictions is not taken ( false )
static int *bpb;
static unsigned int size_bpb;
static unsigned int qtd_bits = DEFAULT_NBITS;
static unsigned int max = ( 1 << DEFAULT_NBITS ) -1;

bool
bpb_init( unsigned int size )
{
  size_bpb = size ? size : DEFAULT_LEN_BPB;

  bpb = calloc( size, sizeof *bpb );

  return !!bpb;
}

bool
bpb_get_prediction( unsigned int pc )
{
  return bpb[ pc % size_bpb ] & ( 1 << ( qtd_bits - 1 ) );
}

void
bpb_update_prediction( unsigned int pc, bool pred )
{
  int i = pc % size_bpb;
  int state = bpb[i];

  if ( pred )
    state = ( (unsigned) ++state ) > max ? max : state;
  else
    state = --state < 0 ? 0 : state;

  bpb[i] = state;
}

void
bpb_free( void )
{
  free( bpb );
}
