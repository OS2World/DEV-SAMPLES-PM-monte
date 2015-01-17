/* client.c

This is the process which is on the client end of the named pipe.
This program is one of two closely-cooperating processes: client.exe
and host.exe.

Pipe clients use standard file I/O API's, so this program could be
a DOS program.

The client process opens the pipe, gets a string from the console,
then writes the string to the named pipe.  It then reads the pipe
and prints the contents of the read.

The client then closes the pipe and exits.

*/


#define INCL_BASE
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>


int main( int argc, char *argv[] )
{
  APIRET  rc;
  HFILE   hPipe;
  ULONG   bytes;
  ULONG   ulAction;
  char    szWork[ 256 ];

  // argument is the pipe name to host
  if( argc != 2 ) {
    printf( "supply pipe name on command line like \\PIPE\\FRED\n" );
    return 1;
  }

  // use file I/O APIs to open the pipe like a file
  rc = DosOpen( argv[ 1 ], &hPipe, &ulAction, 0, 0, FILE_OPEN,
      OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_FAIL_ON_ERROR, NULL );
  printf( "DosOpen rc %d\n", rc );
  assert( rc == 0 );

  // pull a string from the keyboard
  printf( "enter a string: " );
  fflush(NULL);
  gets( szWork );

  // write string to pipe as host process reads it
  rc = DosWrite( hPipe, szWork, strlen( szWork ), &bytes );
  assert( rc == 0 );

  // read from pipe as host process writes it
  rc = DosRead( hPipe, szWork, sizeof( szWork ), &bytes );
  assert( rc == 0 );

  // null-end the string and print it
  szWork[ bytes ] = 0;
  printf( "%s\n", szWork );

  // close the pipe like a file
  rc = DosClose( hPipe );
  assert( rc == 0 );

  return 0;
}
