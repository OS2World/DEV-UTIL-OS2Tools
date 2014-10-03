/**
 * PMStart
 *
 * This is a replacement for PMSHELL.EXE. It's main purpose is to
 * reduce load time, memory footprint and avoid loading the whole PMWP.DLL.
 *
 * Note:
 * This is virtually a do nothing, stay invisible application.
 * It could be modified to ensure at least one shell process is running.
 *
 */

#define INCL_DOS
#define INCL_WIN
#include <os2.h>

int main (int argc, 
          char *argv[])
{
  HAB  hab;
  HMQ  hmq;
  QMSG qmsg;

  hab = WinInitialize( NULLHANDLE );
  if ( hab )
  {
    hmq = WinCreateMsgQueue( hab, 100 );
    if ( hmq )
    {
      for (;;)
      {
        while(WinGetMsg(hab,
                        (PQMSG)&qmsg,
                        NULLHANDLE,
                        0,
                        0))
        {
          WinDispatchMsg(hab,
                         (PQMSG)&qmsg);
        }

        if(qmsg.msg == WM_QUIT &&
           WinIsWindow( hab, 
                       (HWND)qmsg.mp2))
        {
          /* forward appropriate CLOSE command */
          WinSendMsg( (HWND) qmsg.mp2,
                     WM_SYSCOMMAND,
                     MPFROMSHORT( SC_CLOSE ),
                     MPFROM2SHORT( CMDSRC_MENU, FALSE ) );
        }
      }

    }
  }

  return( 0 );
}
