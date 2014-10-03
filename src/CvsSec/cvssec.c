/***
 * (c) 2000 Patrick Haller
 *
 * This tool is an additional access control for the cvs concurrent versions system.
 * It's purpose is to read $CVSROOT/writeinfo file and check if the specified
 * user has access to the specified directory and subsequent directories.
 * If access shall be granted, the program returns 0.
 * For denied access, a non-zero value will be returned.
 *
 * Remarks: 
 * - the project directory names may currently NOT contain whitespaces!
 */


/**********************************************************************
 * includes                                                           *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/**********************************************************************
 * defines & type definitions                                         *
 **********************************************************************/

/*#define DEBUG*/

#ifdef DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

#define ACL_FILENAME "CVSROOT/writeinfo"

#define CMD_UNKNOWN     0
#define CMD_CHECKCOMMIT 1
#define CMD_CHECKTAG    2

#define RC_ACCESS_DENIED  1
#define RC_ACCESS_ALLOWED 0


#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0L
#endif

#ifndef min
#define min(a,b) ( ( a < b ) ? a : b )
#endif

typedef char* PSZ;                          /* zero terminated string */
typedef PSZ*  PPSZ;              /* pointer to zero terminated string */
typedef int   BOOL;                                  /* boolean value */


#ifdef OS2
#define strcasecmp stricmp
#endif


/**********************************************************************
 * prototypes                                                         *
 **********************************************************************/

BOOL name_remove_trailing_slash(PSZ pszName);

BOOL dir_match(PSZ pszCvsRoot,
               PSZ pszDirectory,
               PSZ pszLine);

BOOL user_match(PSZ pszUsername,
                PSZ pszAclLine);

int acl_querydirectory(FILE *fileACL, 
                       PSZ  pszDirectory, 
		       PPSZ ppszReturn);

int acl_open(FILE **pfileACL, 
             PSZ pszDirectorySpec, 
	     PSZ pszAclFile);

int acl_close(FILE *fileACL);

int help_and_exit(void);

int cmd_checkcommit(PSZ pszDirectory);

int main (int argc, PSZ argv[]);


/**********************************************************************
 * implementation                                                     *
 **********************************************************************/

/**********************************************************************
 * Name:       BOOL name_remove_trailing_slash
 * Purpose:    check if a trailing slash or backslash is present and 
 *             remove it
 * Parameters: PSZ  pszName - the directory name to check
 * Returns:    TRUE - slash removed
 *             FALSE - no slash present
 * History:    2000-02-03 22:00 Patrick Haller Created 
 **********************************************************************/

BOOL name_remove_trailing_slash(PSZ pszName)
{
  int iNameLength = strlen(pszName);
  
  if (iNameLength < 1)                /* for sure nothing to remove ! */
    return FALSE; 

  if ( (pszName[iNameLength - 1] == '/') ||
       (pszName[iNameLength - 1] == '\\') )
  {
    pszName[iNameLength - 1] = 0;
    return TRUE;    
  }
  else
    return FALSE;
}


/**********************************************************************
 * Name:       BOOL dir_match
 * Purpose:    check if the specified directory and the acl entry match
 * Parameters: PSZ  pszDirectory - the directory name to scan for
 *             PSZ  pszLine      - the acl entry from the acl file
 * Returns:    TRUE - names match
 *             FALSE - mismatch
 * History:    2000-02-03 22:00 Patrick Haller Created 
 **********************************************************************/

BOOL dir_match(PSZ pszCvsRoot,
               PSZ pszDirectory,
               PSZ pszLine)
{
  char szDir[260]; 
  int  iSpace;         /* position of the 1st space within the string */
  PSZ  pszTemp;
  int  iCvsRootLength;

  DPRINTF(("cvssec: Debug: dir_match(%s,%s,%s)\n",
           pszCvsRoot,
           pszDirectory,
	   pszLine));

  if ( (NULL == pszCvsRoot) ||
       (NULL == pszDirectory) ||                  /* check parameters */
       (NULL == pszLine) ) 
    return FALSE;                         /* no match in this case :) */

  iCvsRootLength = strlen(pszCvsRoot);
  if (0 == iCvsRootLength)
    return FALSE;

                 /* extract the 1st subword from the pszLine argument */
  pszTemp = strchr(pszLine, ' ');
  if (NULL == pszTemp)                              /* wrong syntax ? */
    return FALSE;

  iSpace = min(pszTemp - pszLine, sizeof(szDir));
  iSpace += iCvsRootLength;
  if (iSpace > 0)                
  {
    strcpy(szDir,
           pszCvsRoot);
	   
    strncat(szDir,                                /* copy the subword */
            pszLine,
            iSpace);
    szDir[iSpace] = 0;                        /* terminate the string */
    

    DPRINTF(("cvssec: DPRINTF szDir=[%s] iSpace=%d\n",
            szDir, iSpace));


    if (szDir[iSpace - 1] == '*')           /* ends with a wildcard? */
    { 
      /* if the directory acl entry ends with a *, then all          */
      /* directories from this path below are mathed!                */
      if (strncmp(szDir, 
                  pszDirectory,
                  iSpace - 1) == 0)
        return TRUE;
    }                        
    else                      
      if (strcmp(szDir,    /* here, it requires a full name match ! */
                 pszDirectory) == 0)
        return TRUE;
  }
 
  return FALSE; /* no match ! */
}


/**********************************************************************
 * Name:       BOOL user_match
 * Purpose:    check if the specified username and the acl entry match
 * Parameters: PSZ  pszUsername - the username to scan for
 *             PSZ  pszAclLine  - the acl entry from the acl file
 * Returns:    TRUE - names match
 *             FALSE - mismatch
 * History:    2000-02-03 22:00 Patrick Haller Created 
 **********************************************************************/

BOOL user_match(PSZ pszUsername,
                PSZ pszAclLine)
{
  PSZ pszTemp;

  DPRINTF(("cvssec - DPRINTF user=[%s], acl=[%s]\n",
         pszUsername,
         pszAclLine));

  /* @@@PH pretty cheap scan here! */
  
  pszAclLine = strchr(pszAclLine, ' ');
  if (NULL != pszAclLine)                    /* correct line syntax ? */
  {
    while (*pszAclLine == ' ') 
      pszAclLine++;                             /* skip whitespaces ! */

    while (*pszAclLine)                    /* until end of the string */
    {
      pszTemp = strchr(pszAclLine, ' ');               /* get subword */
      if (NULL == pszTemp)
        pszTemp = pszAclLine + strlen(pszAclLine);

      if (strncmp(pszUsername,                       /* compare words */
                  pszAclLine,
                  pszTemp - pszAclLine) == 0)
        return TRUE;                                    /* OK, match! */

      pszAclLine = pszTemp;                      /* skip to next word */
    }
  }


  return FALSE; /* default is "no match" */
}


/**********************************************************************
 * Name:       int acl_querydirectory
 * Purpose:    scan the specified file for a line starting with the
 *             specified directory name.
 * Parameters: FILE *fileACL     - the file to scan
 *             PSZ  pszDirectory - the directory name to scan for
 *             PPSZ ppszReturn   - pointer to result string address
 * Returns:    0  - no error
 *             !0 - return code from the i/o calls 
 *             ppszReturn will either point to the the matched line
 *             or set the address to NULL if no match was found. 
 * History:    2000-02-03 20:08 Patrick Haller Created 
 **********************************************************************/

int acl_querydirectory(FILE *fileACL,
                       PSZ  pszDirectory,
                       PPSZ ppszReturn)
{
  int  rc;                          /* return code from i/o operation */
  char szScanBuffer[256];      /* line buffer for the fgets operation */
  PSZ  pszLine;        /* pointer to the line that has just been read */
  int  iDirectoryLength;            /* length of the directory string */
  PSZ  pszCvsRoot;       /* entry of the CVSROOT environment variable */

  pszCvsRoot = getenv("CVSROOT");            /* query the environment */
  if (NULL != pszCvsRoot)
  {
    int iCvsRootLast = strlen(pszCvsRoot) - 1;
    
    /* check for trailing slashes */
    name_remove_trailing_slash(pszCvsRoot);
  }

  if ( (NULL == fileACL) ||                       /* check parameters */
       (NULL == pszDirectory) ||
       (NULL == ppszReturn) ||
       (NULL == pszCvsRoot) )
    return EINVAL;                                    /* signal error */

  iDirectoryLength = strlen(pszDirectory);    /* get length of string */     
  if (0 == iDirectoryLength)                   /* invalid parameter ? */
    return EINVAL;

  rc = fseek(fileACL, 0, SEEK_SET);      /* jump to start of the file */
  if (-1 == rc)                                   /* check for errors */
    return EACCES;                          /* signal error condition */

  while (!feof(fileACL))
  {
    pszLine = fgets(szScanBuffer,          /* read line from the file */
                    sizeof(szScanBuffer),
                    fileACL);
    if (NULL == pszLine)                               /* error check */
      return ENOENT;                                  /* signal error */

    if (pszLine[0] != '#')                          /* comment line ? */
      if (dir_match(pszCvsRoot,
                    pszDirectory,
                    pszLine) == TRUE)
        {
          *ppszReturn = strdup(pszLine);             /* copy the line */
          return 0;                                 /* OK, we're done */
        }
  }

  return ENOENT;                        /* return result of operation */  
}


/**********************************************************************
 * Name:       int acl_open
 * Purpose:    We've got to scan for ./CVSROOT/(ACL_FILENAME) walking upwards
 *             from the specified path. If we've not been successful,
 *             we can at last repeat a single scan for $CVSROOT/(ACL_FILENAME).
 *             If no acl file can be opened, abort operation and deny access.
 * Parameters: 
 *             FILE **pfileACL   - where to return the file handle
 *             PSZ  pszDirectory - the directory to scan
 *             PSZ  pszAclFile   - name of the acl file
 * Returns:    0  - no error
 *             !0 - return code from the i/o calls 
 *             pfileACL will contain the file descriptor if scan was
 *             successful
 * History:    2000-02-03 20:08 Patrick Haller Created 
 **********************************************************************/

int acl_open(FILE **pfileACL,
             PSZ  pszDirectorySpec,	     
             PSZ  pszAclFile)
{
  char szBuffer[260];                      /* string operation buffer */
  int  iDirectoryLength;              /* length of the directory name */
  PSZ  pszDirectory;              /* our copy of the directory string */
  PSZ  pszTemp;                           /* temporary string pointer */

  if ( (NULL == pfileACL) ||                      /* check parameters */
       (NULL == pszDirectorySpec) ||
       (NULL == pszAclFile) )
    return EINVAL;                                    /* signal error */
    
  iDirectoryLength = strlen(pszDirectorySpec);
  if (0 == iDirectoryLength)                      /* check parameters */
    return EINVAL;

                                         /* copy the directory string */
  pszDirectory = (PSZ)malloc(iDirectoryLength + 2);     /* more room! */
  if (NULL == pszDirectory)                /* check proper allocation */
    return ENOMEM;                                    /* signal error */

  strcpy(pszDirectory,                             /* copy the string */
         pszDirectorySpec);
  pszDirectory[iDirectoryLength] = 0;    /* ensure string termination */
  

  /****************************************
   * walk upwards the specified directory *
   ****************************************/

  // check for missing trailing slash/backslash
  name_remove_trailing_slash(pszDirectory);

  /* now try to open the file */
  *pfileACL = NULL;

  while (NULL == *pfileACL)
  {
    /* build filename to test */
    strncpy (szBuffer,
             pszDirectory,
             sizeof(szBuffer) - strlen(pszAclFile) - 1);
    strcat (szBuffer,
            pszAclFile);

    DPRINTF(("DPRINTF: testing [%s]\n",                  /* DPRINTF message */
           szBuffer));    

    *pfileACL = fopen(szBuffer, "r");         /* try to open the file */
    if (NULL != *pfileACL)
    {
      free(pszDirectory);         /* free previously allocated memory */
      return 0;                                                 /* OK */  
    }

           /* as we didn't succeed to open the file, walk up the tree */
    DPRINTF(("cvssec: DPRINTF - pszDirectory = [%s]\n",
           pszDirectory));

    name_remove_trailing_slash(pszDirectory);
    if (*pszDirectory == 0)                           /* we're done ! */
      break;

    DPRINTF(("cvssec: DPRINTF - pszDirectory = [%s]\n",
           pszDirectory));

    pszTemp = strrchr(pszDirectory, '/');
    if (NULL == pszTemp)                             /* did it exist? */
      pszTemp = strrchr(pszDirectory,'\\');          /* try backslash */

    if (NULL == pszTemp)                     /* OK, we're at the end! */
      break;                                        /* leave the loop */

    *++pszTemp = 0;           /* terminate string after the character */   
  }


  /****************************************
   * try the environment variable         *
   ****************************************/

  pszTemp = getenv("CVSROOT");               /* query the environment */

  DPRINTF(("cvssec: DPRINTF testing $CVSROOT=[%s]\n",
           (pszTemp ? pszTemp : "null")));

  if (NULL != pszTemp)
  {
    /* extract directory */
    pszDirectory = strrchr(pszTemp, ':');      /* scan for last colon */
    if (NULL != pszDirectory)           /* if we've found a colon ... */
      pszDirectory++;                   /* skip to the next character */
    else
      pszDirectory = pszTemp;                    /* path only CVSROOT */
                    
    strncpy(szBuffer,         /* now we've got the directory string ! */ 
            pszDirectory,
            sizeof(szBuffer) - strlen(pszAclFile) - 1);

                          /* check for the required terminating slash */
    if ( (szBuffer[strlen(szBuffer) - 1] != '/') &&
         (szBuffer[strlen(szBuffer) - 1] != '\\') )
      strcat(szBuffer,
             "/");

    strcat(szBuffer,
           pszAclFile);

    DPRINTF(("DPRINTF: testing2 [%s]\n",                 /* DPRINTF message */
           szBuffer));

    *pfileACL = fopen(szBuffer, "r");         /* try to open the file */
    if (NULL != *pfileACL)
    {
      free(pszDirectory);         /* free previously allocated memory */
      return 0;                                                 /* OK */  
    }   
  }

  free(pszDirectory);             /* free previously allocated memory */
  return ENOENT;                                                /* OK */
}


/**********************************************************************
 * Name:       int acl_close
 * Purpose:    Close the acl file descriptor if it's open.
 * Parameters: FILE *fileACL   - the acl file descriptor
 * Returns:    0  - no error
 *             !0 - return code from the i/o calls 
 * History:    2000-02-03 20:08 Patrick Haller Created 
 **********************************************************************/

int acl_close(FILE *fileACL)
{
  if(NULL != fileACL)
    return(fclose(fileACL));                        /* close the file */
  else
   return 0;                                                    /* OK */
}


/**********************************************************************
 * Name:       int help_and_exit()
 * Purpose:    print usage description
 * Parameters: -
 * Returns:    does not return
 *             
 * History:    2000-02-03 20:24 Patrick Haller Created 
 **********************************************************************/

int help_and_exit(void)
{
                                               /* yield usage message */
  printf ("cvssec (c) 2000 Patrick Haller <phaller@gmx.net>\n"
          "Usage: cvssec [checkcommit, checktag] [path] [file(s)]\n");
  exit(-1); /* deny access */

  return 0;                                    /* keep compiler happy */
}  


/**********************************************************************
 * Name:       int cmd_checkcommit
 * Purpose:    perform checkcommit operation
 * Parameters: PSZ pszDirectory - the directory to look for
 * Returns:    0 - access allowed
 *             1 - access denied
 *             
 * History:    2000-02-03 20:24 Patrick Haller Created 
 **********************************************************************/

int cmd_checkcommit(PSZ pszDirectory)
{
  FILE* fileACL;                       /* file handle to the acl file */
  PSZ   pszUsername;                  /* the name of the current user */
  PSZ   pszAclLine;                            /* directory acl entry */
  int   rc;                                   /* function return code */
  int   rcAccess = RC_ACCESS_DENIED;                   /* return code */

  if (NULL == pszDirectory)                       /* check parameters */
    return RC_ACCESS_DENIED;                           /* deny access */

  /* do a CMD_CHECKCOMMIT */
  rc = acl_open(&fileACL,                 /* try to open the acl file */
                pszDirectory,
                ACL_FILENAME);
  if (rc != 0)                                   /* check for success */
  {
    printf ("cvssec: error - cannot open acl file '%s'\n",
            ACL_FILENAME);
    return (rcAccess);                             /* abort operation */
  }
  
  DPRINTF(("cvssec: DPRINTF: fileACL=%08xh\n",
           (void*)fileACL));

  /* do the scan */
  rc = acl_querydirectory(fileACL,
                          pszDirectory,
                          &pszAclLine);
  if (rc == 0)                                   /* check for success */
  {
            /* we have a valid directory entry, now scan for username */
                              /* scan for username set by cvs pserver */
    pszUsername = getenv("USER");
    if (NULL == pszUsername)                      /* check for errors */
      printf("cvssec: error - no USER environment variable set by cvs pserver!\n");
    else
    {
           /* ok, we've got the username and the directory entry line */
      if (user_match(pszUsername,
                     pszAclLine) == TRUE)
        rcAccess = RC_ACCESS_ALLOWED;
    }
  }

  acl_close(fileACL);                               /* close the file */
                      /* no matching directory entry has been found ! */
  return (rcAccess);
}


/**********************************************************************
 * Name:       int main
 * Purpose:    main entry point
 * Parameters: int argc   - number of parameters specified
 *             PSZ argv[] - pointer to string array of parameters
 *
 *             The command line arguments look like this:
 *             0: cvssec  - name of this process
 *             1: command - e. g. "checkcommit", "checktag"
 *             2: path    - e. g. "/mnt/export0/cvs-repository/projects/tools/cvssec"
 *             3: file    - e. g. "test"
 * 
 * Returns:    0  - permit access to cvs repository
 *             !0 - deny access to cvs repository
 *             
 * History:    2000-02-03 20:24 Patrick Haller Created 
 **********************************************************************/

int main (int argc, PSZ argv[])
{
  int  i;
  int  iCommand;                              /* numeric command code */
  PSZ  pszDirectory;                 /* points to specified directory */
  PSZ  pszFiles;                       /* points to the list of files */
  FILE fileAcl;                                    /* file descriptor */
  int  rc;                                             /* return code */
               
  if (argc < 3)         /* check for appropriate number of parameters */
    help_and_exit();

                                       /* query command and directory */
  if (strcasecmp(argv[1], "checkcommit") == 0) 
    iCommand = CMD_CHECKCOMMIT;
  else
    if (strcasecmp(argv[1], "checktag") == 0) 
      iCommand = CMD_CHECKTAG;
    else 
      help_and_exit();                             /* unknown command */

  pszDirectory = argv[2];
  name_remove_trailing_slash(pszDirectory);


  /* Debug output */
  for (i = 3;
       i < argc;
       i++)
  {
    printf ("[%3d] [%s]\n",
            i,
            argv[i]);

    
  }


  if (CMD_CHECKTAG == iCommand)                /* perform the command */
  {
    printf("cvssec: sorry, 'checktag' is not yet implemented.\n");
    rc = RC_ACCESS_DENIED;
  }
  else
  if (CMD_CHECKCOMMIT == iCommand)
  {
    rc = cmd_checkcommit(pszDirectory);
  }
  else
  {
    printf("cvssec: error - unknown command.\n");
    rc = RC_ACCESS_DENIED;
  }    

  DPRINTF(("cvssec: DEBUG: returning %d\n",
           rc));

  return rc;
}

