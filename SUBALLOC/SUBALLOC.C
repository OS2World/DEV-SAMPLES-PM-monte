// suballoc.c

// Shows the use of OS2 suballocation package for heap management.
// Especially good for the suballocation of shared memory,
// although this sample only shows it for process private space.

// It is preferable to have the suballocation package commit pages
// as needed (the DOSSUB_SPARSE_OBJ flag).  Since you can't grow
// memory objects, you can overestimate without a penalty because the
// suballocation package will commit pages as needed.



// os2 includes
#define INCL_DOS
#include <os2.h>

// c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 100K constant for memory object size
#define SIZE_100K        (100*1024)


// ----------------------------------------------------------------------


int main ( void )
{
  APIRET      rc;
  PVOID       pvBase;
  PVOID       apv[ 100 ];
  int         i;

  // allocate a 100K memory object; note: no PAG_COMMIT FLAG
  rc = DosAllocMem(  &pvBase, SIZE_100K, PAG_WRITE | PAG_READ );
  assert( rc == 0 );

  // initialize OS2 suballocation package for this process alone
  rc = DosSubSetMem( pvBase, DOSSUB_INIT | DOSSUB_SPARSE_OBJ, SIZE_100K );
  assert( rc == 0 );

  // suballocate memory 1K at a time.
  for( i = 0; i < 100; i++ ) {
    rc = DosSubAllocMem( pvBase, &apv[ i ], 1024 );
    if( rc != 0 ) {
      // 64 bytes are reserved by OS2 for suballocation management
      // this test failed when i == 99; rc == 311
      printf( "DosSubAllocMem rc %d;   i == %d\n", rc, i );
      break;
    }
  }

  // free the suballocs;  length of alloc required here
  do {
    rc = DosSubFreeMem(  pvBase, apv[ --i ], 1024 );
    assert( rc == 0 );
  } while( i );

  // free the memory object
  rc = DosFreeMem( pvBase );
  assert( rc == 0 );

  printf( "normal completion\n" );
  return 0;
}

