/* term.c

Term.c is a simple, full-duplex asynchronous terminal program.

Thread 1 reads characters from the keyboard and writes them to the com port.
This thread will block most of the time in the keyboard read.

Thread 2 reads characters from the com port and writes them to the screen.
This thread will block most of the time in the read of the com port.

IBM C Set/2 does not support getch() for getting a single key from the
keyboard, so I used KbdCharIn().  KdbCharIn() is 16-bit, so the compiler
does a thunk at compile time.


*/


#define INCL_KBD
#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



//----------------------------------------------------------------------------------
// thread 2 code:
// read com port and write it to standard out (predefined file handle 1)

#define LEN_BUFFER   1

void _Optlink thread2main ( void * pv  )
{
  APIRET     rc;
  CHAR       achBuffer[ LEN_BUFFER + 1 ];
  HFILE      hcom;
  ULONG      ulByteCount;

  // com port handle passed as parameter to thread 2
  hcom = *((PHFILE)pv);

  // read the com port and write the data to standard output
  while( TRUE ) {
    rc = DosRead( hcom, achBuffer, LEN_BUFFER, &ulByteCount );
    assert( rc == 0 );

    achBuffer[ ulByteCount ] = 0;
    rc = DosWrite( 1, achBuffer, ulByteCount, &ulByteCount );
    assert( rc == 0 );
  }
}



//----------------------------------------------------------------------------------
// thread 1: reads characters from the keyboard and writes them to the com port

int main( int argc, char *argv[] )
{
  APIRET         rc;
  APIRET16       rc16;
  HFILE          hcom;
  KBDKEYINFO     keyinfo;
  ULONG          ulByteCount, ulAction, idThread;

  // com port name is argument to program
  if( argc != 2 ) {
    printf( "give com port name on command line\n" );
    return 1;
  }

  // open com port for exclusive read/write access
  rc = DosOpen( argv[ 1 ], &hcom, &ulAction, 0, 0, FILE_OPEN,
    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_FAIL_ON_ERROR, NULL );
  printf( "DosOpen rc %d\n", rc );
  assert( rc == 0 );

  // start thread that reads the com port; pass a pointer to the open comport handle
  idThread = _beginthread( thread2main, NULL, 2*4096, (void *)&hcom  );
  assert( idThread );

  // read a keyboard key and write it to the com port; no echo (full duplex)
  while( TRUE ) {
    rc16 = KbdCharIn( &keyinfo, IO_WAIT, 0 );
    assert( rc16 == 0 );
    rc = DosWrite( hcom, &keyinfo.chChar, 1, &ulByteCount );
    assert( rc == 0 );
  }
}
