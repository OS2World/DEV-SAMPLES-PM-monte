/* app.c:
A sample PM application showing use of an object window operated by
thread 2 for time-consuming tasks.
*/
// os2 includes
#define INCL_DOSPROCESS
#define INCL_WIN
#include <os2.h>
// c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
// app includes
#include "def.h"
#include "app.h"
#include "pmassert.h"

// ----------------------------------------------------------------------
// main entry point for thread 1
int main ( void )
{
  APIRET   rc;
  BOOL     fSuccess;
  HAB      hab;
  HMQ      hmq;
  HWND     hwndClient;
  HWND     hwndFrame;
  QMSG     qmsg;
  ULONG    flCreate;
  PGLOBALS pg;

  // PM application init
  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );
  assert( hmq );

  // register client window class
  // with 4 bytes of window words to hold a pointer to globals
  fSuccess = WinRegisterClass( hab, APP_CLASS_CLIENT, (PFNWP)ClientWinProc,
                      CS_SIZEREDRAW | CS_CLIPCHILDREN, sizeof( PGLOBALS ) );
  pmassert( hab, fSuccess );

  flCreate = FCF_SYSMENU | FCF_SIZEBORDER    | FCF_TITLEBAR |
             FCF_MINMAX  | FCF_SHELLPOSITION | FCF_TASKLIST |
             FCF_MENU    | FCF_ICON;

  // standard window create; returns after WM_CREATE processing finishes
  hwndFrame = WinCreateStdWindow( HWND_DESKTOP, WS_VISIBLE, &flCreate,
               APP_CLASS_CLIENT, APP_TITLE, 0, 0, ID_APP, &hwndClient );
  pmassert( hab, hwndFrame );
  pmassert( hab, hwndClient );
  pg = (PGLOBALS) WinQueryWindowULong( hwndClient, QWL_USER );

  // dispatch user input messages
  while( WinGetMsg( hab, &qmsg, 0, 0, 0 ))
  {
    WinDispatchMsg( hab, &qmsg );
  }

  // wrap up
  WinDestroyWindow ( hwndFrame );
  WinDestroyMsgQueue ( hmq );
  WinTerminate ( hab );

  rc = DosWaitThread( &pg->tidObject, DCWW_WAIT );
  assert( rc == 0 );

  // exit the process
  return 0;
}

// ----------------------------------------------------------------------
// client window procedure
MRESULT EXPENTRY ClientWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  HAB           hab;
  HPS           hps;
  PGLOBALS      pg;
  RECTL         rectl;
  ULONG         ulWork;

  switch( msg ) {
  case WM_CLOSE:
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    WinPostMsg( pg->hwndObject, WM_QUIT, 0, 0 );
    return (MRESULT) 0;

  case WM_COMMAND:
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    switch( SHORT1FROMMP( mp1 )) {
    case IDM_SLEEP:
      // disable client for this lengthy task
      WinSendMsg( hwnd, WM_USER_DISABLE_CLIENT, 0L, 0L );
      // tell object window to perform lengthy task
      WinPostMsg( pg->hwndObject, WM_USER_SLEEP, (MPARAM)hwnd, 0L );
      // wait for ack
      break;
    }
    return (MRESULT) 0;

  case WM_CREATE:
    hab = WinQueryAnchorBlock( hwnd );
    // allocate global memory; see GLOBALS struct in app.h
    pg = (PGLOBALS) malloc( sizeof( GLOBALS ));
    pmassert( hab, pg );
    // initialize globals to zero
    memset( pg, 0, sizeof( GLOBALS ));
    // store globals pointer into client window words
    WinSetWindowULong( hwnd, QWL_USER, (ULONG) pg );
    // disable client until object window initializes
    WinSendMsg( hwnd, WM_USER_DISABLE_CLIENT, 0, 0 );
    // initialize globals with important data
    pg->hab          = hab;
    pg->hwndClient   = hwnd;
    pg->hwndFrame    = WinQueryWindow( hwnd, QW_PARENT );
    pg->hwndTitlebar = WinWindowFromID( pg->hwndFrame, FID_TITLEBAR );
    pg->hwndMenubar  = WinWindowFromID( pg->hwndFrame, FID_MENU );
    // create thread 2 for object window; pass pointer to globals
    pg->tidObject = _beginthread( threadmain, NULL, LEN_STACK, (void *)pg );
    pmassert( hab, pg->tidObject );
    return (MRESULT) 0;

  case WM_MOUSEMOVE:
    // if busy, display the wait pointer, else the arrow pointer
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    ulWork = pg->fBusy ? SPTR_WAIT : SPTR_ARROW;
    WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP,ulWork,FALSE));
    return (MRESULT) TRUE;

  case WM_PAINT:
    hps = WinBeginPaint( hwnd, 0, &rectl );
    WinFillRect( hps, &rectl, SYSCLR_WINDOW );
    WinEndPaint( hps );
    return (MRESULT) 0;

  case WM_USER_ACK:
    // object window has completed which task?
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    switch( (ULONG) mp1 ) {
    case WM_USER_SLEEP:
      WinMessageBox( HWND_DESKTOP, pg->hwndFrame, "Done sleeping.",
                       APP_TITLE, 0, MB_CANCEL);
      break;
    }
    WinSendMsg( hwnd, WM_USER_ENABLE_CLIENT, 0, 0 );
    return (MRESULT) 0;

  case WM_USER_DISABLE_CLIENT:
    // this message sent by client
    // disable all but the frame window
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    pg->fBusy = TRUE;
    WinEnableWindow( pg->hwndClient, FALSE );
    WinEnableWindow( pg->hwndMenubar, FALSE );
    return (MRESULT) 0;

  case WM_USER_ENABLE_CLIENT:
    // this message sent by client
    // enable client and menu
    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
    WinEnableWindow( pg->hwndClient, TRUE );
    WinEnableWindow( pg->hwndMenubar, TRUE );
    pg->fBusy = FALSE;
    return (MRESULT) 0;
  }
  // default
  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}
