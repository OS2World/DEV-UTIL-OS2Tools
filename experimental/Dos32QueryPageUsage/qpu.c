#define INCL_DOS
#include <os2.h>
#include <stdio.h>


APIRET APIENTRY Dos32QueryPageUsage (ULONG ulCallLevel,
                                     ULONG ulParam1,
                                     ULONG ulParam2);

main()
{
  APIRET rc;
  CHAR szBuffer[512];
  
  rc = Dos32QueryPageUsage (0, /* register */
                            0,
                            0);
  
  printf("\n0 0 0 (register)   : rc=%u",rc);
  getch();
  
  
  
  rc = Dos32QueryPageUsage (0, /* register */
                            1,
                            0);
  
  printf("\n0 1 0 (register) : rc=%u",rc);
  getch();
  
  rc = Dos32QueryPageUsage (0, /* register */
                            2,
                            0);
  
  printf("\n0 2 0 (register) : rc=%u",rc);
  getch();
  
  rc = Dos32QueryPageUsage (0, /* register */
                            0,
                            (ULONG)szBuffer);
  
  printf("\n0 0 x (register)   : rc=%u",rc);
  getch();

  
  rc = Dos32QueryPageUsage (0, /* register */
                            1,
                            (ULONG)szBuffer);
  
  printf("\n0 1 x (register) : rc=%u",rc);
  getch();
  
  rc = Dos32QueryPageUsage (0, /* register */
                            2,
                            (ULONG)szBuffer);
  
  printf("\n0 2 x (register) : rc=%u",rc);
  getch();
  
#if 0
  rc = Dos32QueryPageUsage (1, /* register */
                            0,
                            (ULONG)szBuffer);
  
  printf("\n1 0 x (register) : rc=%u",rc);
  getch();
#endif
  
#if 0
  TRAP CS:EIP= 0160:fff5f95a
  rc = Dos32QueryPageUsage (1, /* register */
                            1,
                            (ULONG)szBuffer);
  
  printf("\n1 1 x (register) : rc=%u",rc);
  getch();
#endif
  
  rc = Dos32QueryPageUsage (3, /* register */
                            0,
                            0);
  
  printf("\n3 0 0 (unregister) : rc=%u",rc);
  getch();
  
  rc = Dos32QueryPageUsage (5, /* register */
                            0,
                            0);
  
  printf("\n5 0 0 (unregister) : rc=%u",rc);
  getch();
  
  rc = Dos32QueryPageUsage (4, /* register */
                            0,
                            0);
  
  printf("\n4 0 0 (unregister) : rc=%u",rc);
  getch();

}