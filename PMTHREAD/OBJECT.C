/* object.c: the object window procedure on thread 2  */
// os2 includes
#define INCL_DOSPROCESS
#define INCL_WIN
#include <os2.h>
// crt includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// app includes
#include "def.h"
#include "app.h"
#include "pmassert.h"

//----------------------------------------------------------------------
// thread 2 entry point: gets and dispatches object window messages
// _Optlink is an IBM C Set/2 function modifier
void _Optlink threadmain( void * pv  )
{
  BOOL       fSuccess;
  HAB        hab;
  HMQ        hmq;
  PGLOBALS   pg;
  QMSG       qmsg;

  // cast and set pointer to globals
  pg = (PGLOBALS) pv;

  // thread initialization
  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );

  // prevent system from posting object window a WM_QUIT
  // I'll post WM_QUIT when it's time.
  fSuccess = WinCancelShutdown( hmq, TRUE );
  pmassert( hab, fSuccess );

  fSuccess = WinRegisterClass( hab, APP_CLASS_OBJECT,
                  (PFNWP)ObjectWinProc, 0, sizeof( PGLOBALS ));
  pmassert( hab, fSuccess );

  pg->hwndObject = WinCreateWindow( HWND_OBJECT, APP_CLASS_OBJECT, "",
             0, 0, 0, 0, 0, HWND_OBJECT, HWND_BOTTOM, 0, (PVOID)pg, NULL );
  pmassert( hab, pg->hwndObject );

  // created OK, ack client
  WinPostMsg( pg->hwndClient, WM_USER_ACK, 0, 0 );

  // get/dispatch messages; user messages, for the most part
  while( WinGetMsg ( hab, &qmsg, 0, 0, 0 ))
  {
    WinDispatchMsg ( hab, &qmsg );
  }

  // tell client window to quit
  WinPostMsg( pg->hwndClient, WM_QUIT, 0, 0 );

  // clean up
  WinDestroyWindow( pg->hwndObject );
  WinDestroyMsgQueue( hmq );
  WinTerminate( hab );
  return;
}

// --------------------------------------------------------------------------
// object window procedure; mp1 is the window to acknowledge upon completion
MRESULT EXPENTRY ObjectWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PGLOBALS      pg;
  HWND          hwndToAck;

  // store the handle of the window to ack upon task completion;
  hwndToAck = (HWND)mp1;

  switch( msg ) {
  case WM_CREATE:
    // for the create case, mp1 is pointer to globals;
    // save it in object window words
    WinSetWindowULong( hwnd, QWL_USER, (ULONG) mp1 );
    return (MRESULT) 0;

  case WM_USER_SLEEP:
    // sleep as though this were a time-consuming task
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    DosSleep( 20000 );
    DosBeep( 500, 150 );
    WinPostMsg( hwndToAck, WM_USER_ACK, (MPARAM) msg, 0 );
    return (MRESULT) 0;
  }

  // default:
  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}
