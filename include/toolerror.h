/*****************************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Modul os2errors
 * Funktion  : Datentypen
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 *****************************************************************************/

#ifndef MODULE_OS2TOOLS_ERRORS
#define MODULE_OS2TOOLS_ERRORS

#ifdef __cplusplus
      extern "C" {
#endif
        
#pragma pack(1)


/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/
        
                             /* OS/2 system error codes used in this library */
#ifndef NO_ERROR
  #define NO_ERROR                       0
#endif
        
#define ERROR_TOOLS_INVALID_ARGUMENT   1003                       /* SYS1003 */
        
                                                          /* own error codes */
#define ERROR_TOOLS                      0xC0000000
#define ERROR_TOOLS_INVALID_PARAMETER    ERROR_TOOLS + 0
#define ERROR_TOOLS_NOT_ENOUGH_MEMORY    ERROR_TOOLS + 1
#define ERROR_TOOLS_LIBRARY_BACKLEVEL    ERROR_TOOLS + 2
#define ERROR_TOOLS_LIBRARY_INCOMPATIBLE ERROR_TOOLS + 3
        
        
#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_ERRORS */

