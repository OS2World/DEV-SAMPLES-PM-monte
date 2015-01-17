// pmassert.h
#ifndef NDEBUG
#define pmassert(hab,exp)\
{\
if(!(exp)) {\
  char ebuff[ 64 ]; unsigned long errorid; unsigned short shortrc;\
  errorid = WinGetLastError( hab ); \
  sprintf( ebuff, "Line %d\nFile %s\nLast Error %p\nExpression %s\n",\
                 __LINE__, __FILE__, errorid, #exp );\
  shortrc = WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, ebuff,\
                 "Assertion failed. Continue?", 0, MB_YESNO  );\
  if( shortrc == MBID_NO ) exit( 1 );\
}\
}
#else
  #define pmassert(hab,exp)
#endif
