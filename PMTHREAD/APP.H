// app.h
// application titles, etc
#define APP_TITLE               "Object Window Application"
#define APP_CLASS_CLIENT        "APPClient"
#define APP_CLASS_OBJECT        "APPObject"
// lengths
#define LEN_WORKSTRING              256
#define LEN_STACK                 18000

// structure to hold globals variables common to both threads
struct _globals {
  BOOL           fBusy;
  HAB            hab;
  HWND           hwndClient;
  HWND           hwndFrame;
  HWND           hwndTitlebar;
  HWND           hwndMenubar;
  HWND           hwndObject;
  char           szMsg[ LEN_WORKSTRING ];
  TID            tidObject;
};
typedef struct _globals GLOBALS, *PGLOBALS;

// user messages
#define WM_USER_ACK                      WM_USER+0
#define WM_USER_SLEEP                    WM_USER+1
#define WM_USER_ENABLE_CLIENT            WM_USER+2
#define WM_USER_DISABLE_CLIENT           WM_USER+3

// function prototypes -- _Optlink is a IBM C SET/2 modifier
void _Optlink threadmain( void *  );
PGLOBALS Create( HWND hwnd );
MRESULT EXPENTRY ObjectWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY ClientWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
