/* ipc.c

This sample shows the use of a mutual exclusion semaphore to protect multiple
thread access to a resource.  The resource here is the printf() function.
printf does not need semaphore protection when using multithread C libraries,
but nevermind that for the sake of example.

All created threads block on the request of the mutex sem until thread 1
releases it.

Note that the threads obtain the mutex sem in the order they requested it.

The threads are created by thread 1, then thread 1 waits on a compound
(muxwait, wait-all) semaphore which contains one semaphore for each thread
started.  When each thread is about to exit, it posts its respective
semaphore.  When all the semaphores in the muxwait semaphore are posted,
thread 1 wakes up and terminates the process.

An exercise would be to raise the priority of a given thread to see if that
thread is the first to obtain the mutex sem when thread 1 releases it.

*/

#define INCL_DOS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SEMNAME_ACK       "\\SEM32\\ACK"
#define SEMNAME_SERIAL    "\\SEM32\\SERIAL"
#define SEMNAME_MUX       "\\SEM32\\MUX"
#define COLORNAMES        "amber blue cyan green mauve teal"

struct _threadparms {
  PSZ     pszColorname;
  HEV     hev;
};
typedef struct _threadparms THREADPARMS, *PTHREADPARMS;

// global variables
HEV      hevAck;
HMTX     hmtxSerial;




// ----------------------------------------------------------------------
void _Optlink threadcode( void* pv )
{
  APIRET      rc;
  char        szColor[ 32 ];
  HEV         hev;

  // copy parameters to local storage; the post the acknowledge semaphore
  strcpy( szColor, ((PTHREADPARMS)pv)->pszColorname );
  hev = ((PTHREADPARMS)pv)->hev;
  rc = DosPostEventSem( hevAck );
  assert( rc == 0 );

  // this request will block until thread 1 does a release
  rc = DosRequestMutexSem( hmtxSerial, -1 );
  assert( rc == 0 );

  printf( "%s\n", szColor );

  rc = DosReleaseMutexSem( hmtxSerial );
  assert( rc == 0 );
  rc = DosPostEventSem( hev );
  assert( rc == 0 );

  // return to the C runtime for thread exiting
  return;
}



//-------------------------------------------------------------------------
int main( void )
{
  int          i;
  char         *pch,  szWork[ 100 ];
  APIRET       rc;
  HEV          hev;
  HMUX         hmux;
  SEMRECORD    aSemrecord[ 32 ];
  ULONG        ulThreadCount, ulPostCount, ulUser;
  THREADPARMS  threadparms;
  TID          tid;


  // handles to sems are stored in global static variables accessible from both threads

  // sem used by thread 2 to ack thread 1
  rc = DosCreateEventSem( SEMNAME_ACK, &hevAck, 0, TRUE );
  assert( rc == 0 );

  // mutual-exclusion semaphore to serialize access to a serially-reusable resource
  // create "set" so all other thread block on it until released
  rc = DosCreateMutexSem( SEMNAME_SERIAL, &hmtxSerial, 0, TRUE );
  assert( rc == 0 );

  // count the number of threads created; same as number of colors in COLORNAMES string
  ulThreadCount = 0;

  // use strtok to parse the colors from the COLORNAMES string
  pch = strtok( COLORNAMES, " " );
  while( pch ) {

    // reset this event semaphore; created thread will post it as an acknowledgement
    rc = DosResetEventSem(  hevAck, &ulPostCount );
    assert( rc == 0 );

    // name and create an event semaphore using color name
    sprintf( szWork, "\\SEM32\\%s", pch );
    rc = DosCreateEventSem( szWork, &hev, 0, FALSE );
    assert( rc == 0 );

    // prepare this element of the SEMRECORD array; required below for compound sem wait
    aSemrecord[ ulThreadCount ].hsemCur   = (HSEM)hev;
    aSemrecord[ ulThreadCount ].ulUser    = ulThreadCount;

    // argument to thread is a pointer to a structure of arguments
    threadparms.pszColorname   = pch;
    threadparms.hev            = hev;

    // use beginthread to ensure C runtime support is properly initialized
    i = _beginthread( threadcode, NULL, 2*4096, (void *)&threadparms );
    assert( i );

    ulThreadCount++;

    // wait on acknowledgement from thread; params copied
    rc = DosWaitEventSem( hevAck, 5000 );
    assert( rc == 0 );
    pch = strtok( NULL, " " );
  }

  // create the compound (mux) semaphore;
  // a wait on this sem will wait until all sem in the array post
  rc = DosCreateMuxWaitSem( SEMNAME_MUX, &hmux, ulThreadCount, aSemrecord, DCMW_WAIT_ALL );
  assert( rc == 0 );

  // release this semaphore and some thread with soon obtain it via DosRequestMutexSem
  rc = DosReleaseMutexSem( hmtxSerial );
  assert( rc == 0 );

  // this wait ends when all threads have requested, then released the mutex sem
  rc = DosWaitMuxWaitSem( hmux, -1, &ulUser );
  assert( rc == 0 );

  // ensure threads are gone
  while( ulThreadCount ) {
    // add one because I created threads 2...n
    tid = ulThreadCount+1;
    rc = DosWaitThread( &tid, DCWW_WAIT );
    // rc 309 is invalid thread id, meaning thread has already exited
    assert( 309 == rc || 0 == rc );
    ulThreadCount--;
  }

  // threads are gone; return to the C runtime for exiting thread 1
  return 0;
}
