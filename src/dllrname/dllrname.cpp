/* $Id: dllrname.cpp,v 1.1 2002/01/10 16:24:48 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */

/****************************************************************************
 * To Do
 ****************************************************************************
 
 - support LE format (might be virtually identical to LX)
 - support PE format
 - support ELF format
 - missing feature from original: change DLL internal name if
   DLL is also in the change list
 
 */


/****************************************************************************
 * Includes
 ****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <conio.h>

#include "ccollection.h"
#include "binaryfile.h"
#include "moduleimage.h"
#include "moduleimagelx.h"
#include "moduleimagene.h"


/****************************************************************************
 * Global Variables
 ****************************************************************************/

static BOOL flagQuiet                = FALSE;
static BOOL flagReport               = TRUE;
static BOOL flagNoInternalNameChange = FALSE;
static BOOL flagReplaceModule        = FALSE;
static BOOL flagBackup               = FALSE;
static BOOL flagHelp                 = FALSE;
static BOOL flagOverwrite            = FALSE;
static BOOL flagDisplay              = FALSE;


/****************************************************************************
 * Macros
 ****************************************************************************/

#ifdef DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif


/****************************************************************************
 * Implementation
 ****************************************************************************/


/***********************************************************************
 * Name      :
 * Purpose   :
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    :
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/


/*
  DOS32REPLACEMODULE 417

      APIRET APIENTRY DosReplaceModule( PSZ pszOldModule,
                                  PSZ pszNewModule,
                                  PSZ pszBackupModule );

When a DLL or EXE file is in use by the system, the file is locked. It
can not therefore be replaced on the harddisk by a newer copy. The API
DosReplaceModule is to allow the replacement on the disk of the new
module whilst the system continues to run the old module. The contents
of the file pszOldModule are cached by the system, and the file is
closed. A backup copy of the file is created as pszBackupModule for
recovery purposes should the install program fail. The new module
pszModule takes the place of the original module on the disk.

Note - the system will continue to use the cached old module until all
references to it are released. The next reference to the module will
cause a reload from the new module on disk.

Note - only protect mode executable files can be replaced by this API.
This API can not be used for DOS/Windows programs, or for data files.

Note - pszNewModule and pszBackupModule may be NULL pointers.

Note - this API uses the swap file to cache the old module. This API
is expensive in terms of disk space usage.
*/

/*** IMPORTIERTE PROTOTYPEN ***/
// APIRET APIENTRY DosReplaceModule( PSZ pszOldModule,
//                                  PSZ pszNewModule,
//                                  PSZ pszBackupModule );


int changeImportModuleNames(ModuleImage *mi, 
                            CLinearList *listNames)
{
  int iChanges = 0;
  char szBuffer[256];
  
  // iterate over the list
  PLINEARLISTENTRY pLLE = listNames->getFirst();
  while (NULL != pLLE)
  {
    // OK, process the current name pair
    char* pszNamePair = (char*)pLLE->pObject;
    
    // separate the name pair
    strncpy(szBuffer,
            pszNamePair,
            sizeof( szBuffer ) );
    
    char* pszEqual = strchr(szBuffer, '=');
    if (NULL != pszEqual)
    {
      *pszEqual = 0;
      PSZ pszNewName = pszEqual + 1;
      PSZ pszOldName = szBuffer;
      
      // verify the names are not identical
      if (strcmp(pszOldName,
                 pszNewName) != 0)
      {
        // change the names
        int iChange = mi->changeImportModuleName(pszOldName,
                                                 pszNewName);
        // report ?
        if (flagReport && iChange)
        {
          printf("\n  changed [%s] to [%s] (%d times)",
                 pszOldName,
                 pszNewName,
                 iChange);
        }
        
        // keep track of total changes
        iChanges += iChange;
      }
    }
    
    // skip to the next name pair
    pLLE = listNames->getNext( pLLE );
  }
  
  // formatting required?
  if (flagReport && iChanges)
    printf("\n  --> ");
  
  // Done
  return iChanges;
}


/***********************************************************************
 * Name      : processFile
 * Purpose   : process the specified file
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    : 
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

APIRET processFile( char* pszFilename,
                    CLinearList* listNames)
{
  int    iBytesLoaded;
  APIRET rc = NO_ERROR;
  char*  pszError;
  char   szErrorBuffer[48];
  int    iChanges = 0;
  
  printf("%-14s ",
         pszFilename);
  
  // create binary file object
  BinaryFile *bf = new BinaryFile( pszFilename );
  ModuleImage *mi = NULL;
  
  // read the file into memory
  rc = bf->load( &iBytesLoaded );
  if (NO_ERROR == rc)
  {
    mi = new ModuleImage(bf->getBuffer(),
                         bf->getBufferSize());
    
    // verify we've got an LX module
    switch ( mi->getModuleType() )
    {
      case MODULE_LX:
        printf("LX : ");
      
        // convert the object
        delete mi;
        mi = new ModuleImageLX(bf->getBuffer(),
                               bf->getBufferSize());
        break;
      
      case MODULE_NE:
        printf("NE : ");
      
        // convert the object
        delete mi;
        mi = new ModuleImageNE(bf->getBuffer(),
                               bf->getBufferSize());
        break;
      
      case MODULE_MZ:
        printf("MZ : ");
        rc = ERROR_INVALID_MODULETYPE;
        break;
    
      case MODULE_LE:
        printf("LE : ");
        rc = ERROR_INVALID_MODULETYPE;
        break;
      
      case MODULE_PE:
        printf("PE : ");
        rc = ERROR_INVALID_MODULETYPE;
        break;
      
      default:
        printf("?? : ");
        rc = ERROR_INVALID_MODULETYPE;
        break;
    }
  }
  
  if (NO_ERROR == rc)
  {
    
    // apply changes
    iChanges = changeImportModuleNames(mi, listNames);
    if (iChanges > 0)
    {
      // create backup of original
      if (flagBackup)
      {
        rc = bf->backup(flagOverwrite);
        if (NO_ERROR == rc)
          printf("(backup) ");
      }
      
      // verify if module is already loaded
      // and eventually issue a DosReplaceModule
      if (NO_ERROR == rc)
      {
        rc = bf->canWrite();
        if (ERROR_SHARING_VIOLATION == rc)
        {
          if (flagReplaceModule)
          {
            printf("(replaced) ");
            
            PSZ pszModuleOld    = bf->getFilename();
            PSZ pszModuleNew    = NULL;
            PSZ pszModuleBackup = NULL;
            
            rc = DosReplaceModule  (pszModuleOld, 
                                    pszModuleNew,
                                    pszModuleBackup);
          }
        }
        else
          rc = NO_ERROR;
  
        // save file
        if (NO_ERROR == rc)
          rc = bf->save();
      }
    }
    else
    {
      printf("(unchanged) ");
    }
  }
  
  // build processing status message
  switch (rc)
  {
    case NO_ERROR:             pszError = "OK"; break;
    case ERROR_FILE_NOT_FOUND: pszError = "file not found"; break;
    case ERROR_PATH_NOT_FOUND: pszError = "path not found"; break;
    case ERROR_ACCESS_DENIED:  pszError = "access denied"; break;
    case ERROR_INVALID_MODULETYPE:
                               pszError = "unsupported module format"; break;
    default:
      pszError = szErrorBuffer;
      sprintf( szErrorBuffer, "error #%d", rc);
      break;
  }
  
  // print the processing status message
  printf ("%s, %d changes\n", 
          pszError,
          iChanges);

  // release the binary file object and the image object
  if (NULL != mi)
    delete mi;
  
  delete bf;
  
  return rc;
}


/***********************************************************************
 * Name      : displayFile
 * Purpose   : display the imports of the specified file
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    : 
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

APIRET displayFile( char* pszFilename)
{
  int    iBytesLoaded;
  APIRET rc = NO_ERROR;
  char*  pszError;
  char   szErrorBuffer[48];
  int    iChanges = 0;
  
  printf("%-14s ",
         pszFilename);
  
  // create binary file object
  BinaryFile *bf = new BinaryFile( pszFilename );
  ModuleImage *mi = NULL;
  
  // read the file into memory
  rc = bf->load( &iBytesLoaded );
  if (NO_ERROR == rc)
  {
    mi = new ModuleImage(bf->getBuffer(),
                         bf->getBufferSize());
    
    // verify we've got an LX module
    switch ( mi->getModuleType() )
    {
      case MODULE_LX:
        printf("LX : \n");
      
        // convert the object
        delete mi;
        mi = new ModuleImageLX(bf->getBuffer(),
                               bf->getBufferSize());
        break;
      
      case MODULE_NE:
        printf("NE : \n");
      
        // convert the object
        delete mi;
        mi = new ModuleImageNE(bf->getBuffer(),
                               bf->getBufferSize());
        break;
      
      case MODULE_MZ:
        printf("MZ : \n");
        rc = ERROR_INVALID_MODULETYPE;
        break;
    
      case MODULE_LE:
        printf("LE : \n");
        rc = ERROR_INVALID_MODULETYPE;
        break;
      
      case MODULE_PE:
        printf("PE : \n");
        rc = ERROR_INVALID_MODULETYPE;
        break;
      
      default:
        printf("?? : \n");
        rc = ERROR_INVALID_MODULETYPE;
        break;
    }
  }
  
  if (NO_ERROR == rc)
  {
    // display the imports
    mi->displayImportModuleNames();
  }
  
  // build processing status message
  switch (rc)
  {
    case NO_ERROR:             pszError = NULL; break;
    case ERROR_FILE_NOT_FOUND: pszError = "file not found"; break;
    case ERROR_PATH_NOT_FOUND: pszError = "path not found"; break;
    case ERROR_ACCESS_DENIED:  pszError = "access denied"; break;
    case ERROR_INVALID_MODULETYPE:
                               pszError = "unsupported module format"; break;
    default:
      pszError = szErrorBuffer;
      sprintf( szErrorBuffer, "error #%d", rc);
      break;
  }
  
  // print the processing status message
  if (NULL != pszError)
    printf ("%s\n",
            pszError);

  // release the binary file object and the image object
  if (NULL != mi)
    delete mi;
  
  delete bf;
  
  return rc;
}


/***********************************************************************
 * Name      : displayHelp
 * Purpose   : display short help text on usage of this utility
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    : must simulate original IBM DLLRNAME tool
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

void displayHelp(void)
{
  // display help
  printf("Usage:\n"
         "  DLLRNAME filename1..filenameN [oldname1=newname1..oldnameN=newnameN] [flags]"
         "\n"
         "\nThe flags are:"
         "\n/H or /? - Display help"
         "\n/N       - do not rename a DLL if its internal name changes"
         "\n           (currently never changes internal name)"
         "\n/Q       - do not display logo"
         "\n/R       - do not generate name change report"
         "\n/S       - do automatically replace module if loaded"
         "\n/B       - do automatically create a backup if changes apply"
         "\n/O       - do overwrite already existing backup files"
         "\n/D       - only display the imported modules name table"
         "\n");
}

  
/***********************************************************************
 * Name      : parseArguments
 * Purpose   : parse the command line for arguments and parameters
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    : must simulate original IBM DLLRNAME tool
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

APIRET parseArguments(int argc, 
                      char *argv[], 
                      CLinearList* listFiles, 
                      CLinearList *listNames)
{
  // get parameters from command line
  // build list of files
  // build list of name replacements
  
  if (argc == 0)
  {
    flagHelp = TRUE;
    return NO_ERROR;
  }
  
  for (int i = 1;
       i < argc;
       i++)
  {
    char* pszParam = argv[i];
    
    // check what kind of parameter we have
    if (*pszParam == '/')
    {
      // OK, it's a flag
      switch(*(pszParam + 1))
      {
        case 'h':
        case 'H':
        case '?':
          flagHelp = TRUE;
          break;
          
        case 'q':
        case 'Q':
          flagQuiet = TRUE;
          break;
          
        case 'r':
        case 'R':
          flagReport = FALSE;
          break;
          
        case 'n':
        case 'N':
          flagNoInternalNameChange = TRUE;
          break;
        
        case 's':
        case 'S':
          flagReplaceModule = TRUE;
          break;
        
        case 'b':
        case 'B':
          flagBackup = TRUE;
          break;
        
        case 'o':
        case 'O':
          flagOverwrite = TRUE;
          break;
      
        case 'd':
        case 'D':
          flagDisplay = TRUE;
          break;
        
        default:
          fprintf(stderr,
                  "ERROR: unknown flag '%s' (ignored)\n",
                  pszParam);
          break;
      }
    }
    else
      if (strchr(pszParam, '=') != NULL)
      {
        // OK, it's a name pair
        listNames->addLast( strdup( pszParam ) );
      }
      else
      {
        // treat this as a file then
        listFiles->addLast( strdup( pszParam ) );
      }
  }
  
  return NO_ERROR;
}
  

/***********************************************************************
 * Name      : 
 * Purpose   :
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    :
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

int main(int argc, char *argv[])
{
  int rc = NO_ERROR;
  int iErrors = 0;
  
  CLinearList* listFiles = new CLinearList();
  CLinearList* listNames = new CLinearList();
  
  
  static const char* pszTitle =
    "DLL Rename Utility ("__TIMESTAMP__")\n"
    "(c) 2001-2002 InnoTek Systemberatung GmbH\n"
    "\n";
  
  // get parameters from command line
  // build list of files
  // build list of name replacements
  rc = parseArguments(argc, argv, listFiles, listNames);
  
  // print the title (despite any errors while parsing arguments
  if (!flagQuiet)
    printf(pszTitle);

  if (NO_ERROR != rc)
  {
    fprintf(stderr,
            "ERROR: error #%d while parsing arguments\n",
            rc);
    iErrors++;
  }
  else
  {
    // display help?
    if (flagHelp)
      displayHelp();
    
    // iterate over the list
    PLINEARLISTENTRY pLLE = listFiles->getFirst();
    while (pLLE != NULL)
    {
      // process this file
      char* pszFilename = (char*) pLLE->pObject;
      
      if (flagDisplay)
        rc = displayFile( pszFilename );
      else
        rc = processFile( pszFilename, listNames );
      
      if (NO_ERROR != rc)
      {
        fprintf(stderr,
                "ERROR: error #%d while processing file '%s'\n",
                rc,
                pszFilename);
        iErrors++;
      }
      
      // get the next file
      pLLE = listFiles->getNext( pLLE );
    }
  }
  
  // print report
  printf("Completed, %d errors.\n",
         iErrors);
  
  // done
  delete listFiles;
  delete listNames;
  
  return rc;
}



/*
 * $Log: dllrname.cpp,v $
 * Revision 1.1  2002/01/10 16:24:48  phaller
 * Added DLLRNAME tool
 *
 * Revision 1.7  2002/01/10 16:20:07  patrick
 * Added display feature
 *
 * Revision 1.6  2002/01/10 16:05:29  patrick
 * Some minor bugfixes
 *
 * Revision 1.5  2002/01/10 15:50:51  patrick
 * Updated todo files
 *
 * Revision 1.4  2002/01/10 15:41:52  patrick
 * Finalized first version
 *
 * Revision 1.3  2002/01/10 15:33:48  patrick
 * Tool is almost feature complete. Renaming internal DLL names pending.
 *
 * Revision 1.2  2002/01/10 15:03:44  patrick
 * Added support for NE executables
 *
 * Revision 1.1  2002/01/10 14:44:51  patrick
 * Added DLLRNAME Tool
 *
 *
 */