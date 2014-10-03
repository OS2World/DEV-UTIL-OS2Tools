/*****************************************************
 * Unix-like NetCat Tool.                            *
 *                                                   *
 * (c) 2002 Patrick Haller                           *
 *****************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifndef _IOSTREAM_H_
#define _IOSTREAM_H_


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/


/* Debug macro: squirt whatever message and sleep a bit so we can see it go
   by.  need to call like Debug ((stuff)) [with no ; ] so macro args match!
   Beware: writes to stdOUT... */
#ifdef DEBUG
#define Debug(x) printf x; printf ("\n"); fflush (stdout);
#else
#define Debug(x)	/* nil... */
#endif


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/


typedef struct
{
  BOOL fsOverwrite;
} GLOBALS, *PGLOBALS;

  

 /*
  
  IOStream
  +---IOStreamInput
  !   +--IOStreamInputStdIn   OK
  !   +--IOStreamInputTCP
  !   +--IOStreamInputFile
  !   +--IOStreamInputDASD
  !   +--IOStreamInputOne     OK
  !   +--IOStreamInputZero    OK
  !   +--IOStreamInputRandom  OK
  +---IOStreamOutput
      +--IOStreamOutputStdOut OK
      +--IOStreamOutputTCP
      +--IOStreamOutputFile
      +--IOStreamOutputGZIP
      +--IOStreamOutputUnGZIP
      +--IOStreamOutputBZIP2
      +--IOStreamOutputUnBZIP2
      +--IOStreamOutputDASD
      +--IOStreamOutputNull   OK
      */


class IOStream
{
  protected:
    PSZ  pszName;       /* the name of the stream                      */
    PSZ  pszType;       /* type description of the stream              */
    PSZ  pszDevice;     /* to where this stream connects               */
    BOOL flagConnected; /* indicates if the stream is connected or not */
  
  
  private:
    BOOL flagIsAvailable; /* indicates if this stream is available     */
  
  
  public:
    IOStream(PSZ _pszName,
             PSZ _pszType,
             PSZ _pszDevice)
    {
      pszName         = _pszName;
      pszType         = _pszType;
      pszDevice       = _pszDevice;
      flagConnected   = FALSE;
      setAvailable( TRUE );
    }
  
    /* terminate existing connection to 'device' */
    virtual APIRET disconnect()
    {
      // Note: this method NEEDS override anyway!
      
      setConnected( FALSE );
      
      return NO_ERROR;
    }
  
    ~IOStream()
    {
      if ( isConnected() )
      {
        Debug(("WARNING, stream object '%s' left connected for destructor",
               pszName))
        
        disconnect();
      }
    }
  
    /* return the stream's name */
    PSZ getStreamName() { return pszName; }
    
    /* return the stream's type */
    PSZ getStreamType() { return pszType; }
    
    /* return the stream's device/source/target/etc. */
    PSZ getStreamDevice() { return pszDevice; }

    /* set stream connection state */
    void setConnected( BOOL _isConnected ) { flagConnected = _isConnected; }

    /* query the stream connection state */
    BOOL isConnected( ) { return flagConnected; }

    /* query the availability flag */
    BOOL isAvailable( ) { return flagIsAvailable; }
    void setAvailable( BOOL flagNewAvailability ) { flagIsAvailable = flagNewAvailability; }
};


class IOBuffer
{
  protected:
    /* size of the buffer */
    unsigned long int ulSize;
    
    /* used/valid size of the buffer */
    unsigned long int ulValid;
     
    /* pointer to the storage */
    void *pData;
    
    /* usage indicator */
    /* may be used for multithreaded access later on */
    int iUsed;
  
  public:
    IOBuffer(unsigned long int _ulSize)
    {
      ulValid = 0;
      ulSize  = _ulSize;
      iUsed   = 0;
      
      pData   = (void*) malloc( _ulSize );
      
      if (NULL == pData)
        ulSize = 0;
    }
  
    ~IOBuffer()
    {
      if (NULL != pData)
        free( pData );
    }
  
    /* get direct access to the buffer and lock out others */
    virtual void* acquire( )
    {
      // only one owner at a time
      // @@@PH replace with mutex semaphore
      if (iUsed > 0)
        return NULL;
      
      iUsed++;
      
      return pData;
    }
  
    /* release buffer access */
    void release( )
    {
      // @@@PH replace with mutex semaphore
      if (iUsed > 0)
        iUsed--;
    }
  
    /* get usage status */
    int isUsed( ) { return iUsed; }
  
    /* set the valid range inside the buffer */
    void setValid(unsigned long int _ulValid)
    {
      ulValid = _ulValid;
    }
  
    /* get the valid range inside the buffer */
    unsigned long int getValid( )
    {
      return ulValid;
    }
  
    /* get the overall size of the buffer */
    unsigned long int getSize( )
    {
      return ulSize;
    }
};


class IOStreamInput : public IOStream
{
  public:
    IOStreamInput(PSZ pszName,
                  PSZ pszType,
                  PSZ pszDevice) 
      : IOStream(pszName,
                 pszType,
                 pszDevice)
    {
    }
  
    ~IOStreamInput()
    {
    }
    
    /* setup connection to 'device' */
    virtual APIRET connect( ) = 0;

    virtual APIRET read(IOBuffer *ioBuffer) = 0;
  
    virtual ULONG  estimatedStreamLength(PULONGLONG pull = NULL)
    {
      // by default we tell no stream length
      if (NULL != pull)
      {
        pull->ulLo = 0;
        pull->ulHi = 0;
      }
      
      return 0;
    }
};


class IOStreamOutput : public IOStream
{
  protected:
    IOStreamOutput *osAssociate;
  
  public:
    IOStreamOutput(PSZ pszName,
                   PSZ pszType,
                   PSZ pszDevice)
      : IOStream(pszName,
                 pszType,
                 pszDevice)
    {
      osAssociate = NULL;
    }
  
    ~IOStreamOutput()
    {
      if (NULL != osAssociate)
      {
        // delete the whole chain
        delete osAssociate;
        osAssociate = NULL;
      }
    }
  
    virtual APIRET connect( IOStreamInput* in ) = 0;
  
    virtual APIRET write(IOBuffer *ioBuffer) = 0;
  
    virtual void associate(IOStreamOutput *_osAssociate)
    {
      osAssociate = _osAssociate;
    }
};



class IOStreamOutputTCP : public IOStreamOutput
{
  protected:
    PSZ   pszURL;
    int   sockOutput;
  
  public:
    IOStreamOutputTCP( PSZ pszStreamToken );
    ~IOStreamOutputTCP( );
  
    virtual APIRET connect( IOStreamInput* in );
    virtual APIRET disconnect();
    virtual APIRET write(IOBuffer *ioBuffer);
};


class IOStreamInputTCP : public IOStreamInput
{
  protected:
    PSZ   pszURL;
    int   sockListen;
    int   sockInput;
  
  public:
    IOStreamInputTCP( PSZ pszStreamToken );
    ~IOStreamInputTCP( );
  
    virtual APIRET connect( );
    virtual APIRET disconnect();
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamOutputStdOut : public IOStreamOutput
{
  public:
    IOStreamOutputStdOut( );
    ~IOStreamOutputStdOut( );
    virtual APIRET connect( IOStreamInput* in );
    virtual APIRET disconnect();
    virtual APIRET write(IOBuffer *ioBuffer);
};


class IOStreamOutputFile : public IOStreamOutput
{
  protected:
    HFILE hFileOutput;
    PSZ   pszFilename;
  
  public:
    IOStreamOutputFile( PSZ pszStreamToken );
    ~IOStreamOutputFile( );
  
    virtual APIRET connect( IOStreamInput* in );
    virtual APIRET disconnect();
    virtual APIRET write(IOBuffer *ioBuffer);
};


class IOStreamOutputNull : public IOStreamOutput
{
  public:
    IOStreamOutputNull( );
    ~IOStreamOutputNull( );
  
    virtual APIRET connect( IOStreamInput *in );
    virtual APIRET disconnect();
    virtual APIRET write(IOBuffer *ioBuffer);
};


class IOStreamOutputDASDLogical : public IOStreamOutput
{
  protected:
    HFILE hDASDOutput;
    PSZ   pszPartitionName;
  
  public:
    IOStreamOutputDASDLogical( PSZ pszStreamToken );
    ~IOStreamOutputDASDLogical( );
      
    virtual APIRET connect( IOStreamInput* in );
    virtual APIRET disconnect();
    virtual APIRET write(IOBuffer *ioBuffer);
};




class IOStreamInputFile : public IOStreamInput
{
  protected:
    HFILE hFileInput;
    PSZ   pszFilename;
  
  public:
    IOStreamInputFile( PSZ pszStreamToken );
    ~IOStreamInputFile( );
  
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual ULONG estimatedStreamLength(PULONGLONG pull = NULL);
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamInputStdIn : public IOStreamInput
{
  public:
    IOStreamInputStdIn( );
    ~IOStreamInputStdIn( );
      
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamInputZero : public IOStreamInput
{
  public:
    IOStreamInputZero( );
    ~IOStreamInputZero( );
      
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamInputOne : public IOStreamInput
{
  public:
    IOStreamInputOne( );
    ~IOStreamInputOne( );
      
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamInputRandom : public IOStreamInput
{
  public:
    IOStreamInputRandom( );
    ~IOStreamInputRandom( );
      
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual APIRET read(IOBuffer *ioBuffer);
};


class IOStreamInputDASDLogical : public IOStreamInput
{
  protected:
    HFILE hDASDInput;
    PSZ   pszPartitionName;
  
  public:
    IOStreamInputDASDLogical( PSZ pszStreamToken );
    ~IOStreamInputDASDLogical( );
  
    virtual APIRET connect();
    virtual APIRET disconnect();
    virtual ULONG estimatedStreamLength(PULONGLONG pull = NULL);
    virtual APIRET read(IOBuffer *ioBuffer);
};





#endif /* _IOSTREAM_H_ */
