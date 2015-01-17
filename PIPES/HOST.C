/* host.c

This process hosts a named pipe.  It is one of two closely-cooperating
processes: host.exe and client.exe.

The program creates a named pipe, then blocks in DosConnectNPipe until the
client process does an open.

Upon connect, the host process reads the pipe.  The data is assumed to be
string data.  The host reverses the characters in the string and write the
result back down the pipe.

The host then does a DosDisconnectNPipe, loops, and blocks again in the
connect.

Control-break to terminate.

*/

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>

#define LEN_PIPE                      512
#define PIPEMODE_UNIQUE            0x0001
#define OPENMODE_DUPLEX            0X0002

int main( int argc, char **argv )
{
  APIRET         rc;
  HFILE          hPipe;
  ULONG          bytes;
  ULONG          bytesread;
  char           achBuffer[ LEN_PIPE ];

  // argument is the pipe name to host
  if( argc != 2 ) {
    printf( "supply pipe name on command line like \\PIPE\\FRED\n" );
    return 1;
  }

  // create a named pipe in the disconnected state
  rc = DosCreateNPipe( argv[1], &hPipe, OPENMODE_DUPLEX, PIPEMODE_UNIQUE,
    LEN_PIPE, LEN_PIPE, 1000L );
  printf( "DosCreateNPipe rc %d\n", rc );
  assert( rc == 0 );

  while( TRUE ) {

    // pipe connection occurs when client process does a file open on this pipe
    printf( "waiting for connection\n" );
    rc = DosConnectNPipe( hPipe );
    assert( rc == 0 );
    printf( "connected\n" );

    // read data from duplex pipe as client process writes it
    rc = DosRead( hPipe, achBuffer, LEN_PIPE, &bytesread );
    assert( rc == 0 );

    // reverse string and write back into pipe as client process reads it
    achBuffer[ bytesread ] = 0;
    strrev( achBuffer );
    rc = DosWrite( hPipe, achBuffer, bytesread, &bytes );
    assert( rc == 0 );

    // disconnect long pipe and loop
    DosDisConnectNPipe( hPipe );
    printf( "disconnected\n" );
  }
  return 0;
}
