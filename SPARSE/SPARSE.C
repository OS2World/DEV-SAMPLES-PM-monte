/* sparse.c

Allocates a 1MB memory object but commits no pages in that memory object.

The program then proceeds to write on that memory which is invalid. This
causes an exception, or trap.

Traps are handled by an exception handler provided by this program.  At
exception time, the handler will commit the invalid page(s).

*/


// os2 includes
#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

// c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// a typedef for a function that handles an exception
typedef ULONG _System FNEH( PEXCEPTIONREPORTRECORD,
                            PEXCEPTIONREGISTRATIONRECORD,
                            PCONTEXTRECORD,
                            PVOID );
typedef FNEH *PFNEH;

// a typedef for an exception handler registration record; a link in a list
struct _syseregrec {
  PEXCEPTIONREGISTRATIONRECORD pnext;
  PFNEH                        pfneh;
};
typedef struct _syseregrec SYSEREGREC;


// ----------------------------------------------------------------------
// the exception handler function

ULONG _System Handler( PEXCEPTIONREPORTRECORD p1,
                       PEXCEPTIONREGISTRATIONRECORD p2,
                       PCONTEXTRECORD p3,
                       PVOID pv )
{
   PVOID  pvBase;

   // exception number of interest is access violation
   if( p1->ExceptionNum == XCPT_ACCESS_VIOLATION  ) {

      // violation occurred on a write access
      if( p1->ExceptionInfo[ 0 ] == XCPT_WRITE_ACCESS ) {

         // try to commit the referenced page; first make a pointer to it
         pvBase = (PVOID)(p1->ExceptionInfo[ 1 ] & 0xFFFFF000);

         // set the attribute of the page to be commit and writable
         if( NO_ERROR == DosSetMem( pvBase, 1, PAG_COMMIT | PAG_WRITE )) {
           // successful commit; this exception has been handled.
           return XCPT_CONTINUE_EXECUTION;
         }
      }
   }
   // not handled, let other handlers in the chain have the exception
   return XCPT_CONTINUE_SEARCH;
}


// ----------------------------------------------------------------------
int main ( void )
{
  APIRET      rc;
  PCHAR       pchar;
  PSZ         psz;
  PVOID       pBase;
  SYSEREGREC  regrec;

  // insert my exception handler into the chain of handlers for this thread
  regrec.pnext = NULL;
  regrec.pfneh = Handler;
  rc = DosSetExceptionHandler( (PEXCEPTIONREGISTRATIONRECORD) &regrec );
  assert( rc == 0 );

  // allocate a 1 megabyte memory object without committing any of it;
  // note no PAG_COMMIT flag
  rc = DosAllocMem(  &pBase, (1024*1024), PAG_WRITE );
  assert( rc == 0 );

  // this will cause an exception since the page is not committed
  pchar = (PCHAR)pBase;
  *pchar = 'a';

  // this string copy will cause two exceptions
  psz = (PSZ)pBase + 0x06fffa;
  strcpy( psz, "THIS IS A VERY LONG STRING THAT WILL CROSS A 4K BOUND." );

  // reference the memory
  printf( "%c\n", *pchar );
  printf( "%s\n", psz );

  // free memory object
  rc = DosFreeMem( pBase );
  assert( rc == 0 );

  // unlink my handler before exiting;
  // important because the link record is automatic to this function
  rc = DosUnsetExceptionHandler( (PEXCEPTIONREGISTRATIONRECORD) &regrec );
  assert( rc == 0 );

  return 0;
}
