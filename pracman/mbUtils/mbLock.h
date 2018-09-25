//---------------------------------------------------------------------------
// Function:    mbLock.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Feb. 9 2002
//---------------------------------------------------------------------------
// Description:
//
// Locking macros and typedefs.
//
// MAB:20021104 These functions only work in a single
// threaded environment.  I would have to investigate something like
// TCriticalSection (or its implementation) for a true multithreaded
// environment.  I have left that out for now because the TCriticalSection
// must be allocated using the new operator, and the queueing library
// currently allocates a lock but does not free it.  This would result in
// a memory leak.  I should look at updating the queuing stuff to be
// object oriented (i.e, C++).
//---------------------------------------------------------------------------

#ifndef mbLock_h
#define mbLock_h

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Int32u_t  mbAllocatedLocks
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

External Int32u_t  mbAllocatedLocksNew
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------


#define MB_DEBUG_LOCK   (1)

// 20050101: New lock object that uses actual mutex
typedef struct
{
    Void_p              handle;
    Int32u_t            count;
    Int64u_t            threadId;
#if MB_DEBUG_LOCK
    Char_t              location[MAX_PATH];
    Int32u_t            magic;
#endif MB_DEBUG_LOCK
} MB_LOCK_t, *MB_LOCK_p;

// 20050101: End of new code

typedef struct
{
    Int32u_t            count;
#if MB_DEBUG_LOCK
    Char_t              location[MAX_PATH];
    Int32u_t            magic;
#endif MB_DEBUG_LOCK
} mbLock_t, *mbLock_p;


#if MB_DEBUG_LOCK

#define MB_LOCK_MAGIC 0x5F48E320

#define mbLockAttempt( _parm ) mbLockAttemptFunc( &_parm, __FILE__, __LINE__ )

#define mbLockAcquire( _parm ) mbLockAcquireFunc( &_parm, __FILE__, __LINE__ )

#define mbLockInit( _parm )\
{\
    _parm.count = 0;\
    _parm.magic = MB_LOCK_MAGIC;\
    _parm.location[0] = 0;\
    mbAllocatedLocks++;\
}

#define mbLockFree( _parm )\
{\
    if( _parm.magic != MB_LOCK_MAGIC )\
    {\
        mbDlgDebug(( "Invalid magic number in Lock" ));\
    }\
    else\
    {\
        _parm.magic = 0;\
        mbAllocatedLocks--;\
    }\
}

#define mbLockRelease( _parm )\
{\
   if( _parm.magic != MB_LOCK_MAGIC )\
    {\
        mbDlgDebug(( "Invalid magic number in Lock" ));\
    }\
    else\
    {\
        if( --_parm.count != 0 )\
        {\
            mbLog( "Release Lock Error: Count: %d", _parm );\
        }\
        else\
        {\
            _parm.location[0] = 0;\
        }\
    }\
}

#else // MB_DEBUG_LOCK

#define mbLockAttempt( _parm ) mbAttemptLockFunc( &_parm, __FILE__, __LINE__ )

#define mbLockAcquire( _parm )\
{\
    if( ++_parm.count != 1 ) { mbDlgDebug(( "Acquire Lock Error: Count: %d", _parm )); }\
}

#define mbLockInit( _parm )\
{\
    _parm = 0;\
    mbAllocatedLocks++;\
}

#define mbLockFree( _parm )\
{\
    mbAllocatedLocks--;\
}

#define mbLockRelease( _parm )\
{\
   if( --_parm != 0 ){ mbDlgDebug(( "Release Lock Error: Count: %d", _parm )); }\
}

#endif // MB_DEBUG_LOCK

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t        mbLockAttemptFunc
(
    mbLock_p    lock_p,
    Char_p      file_p,
    Int32s_t    line
);

Void_t          mbLockAcquireFunc
(
    mbLock_p    lock_p,
    Char_p      file_p,
    Int32s_t    line
);

MB_LOCK_p       MB_LOCK_INIT
(
    MB_LOCK_p   lock_p
);

Void_t          MB_LOCK_ACQUIRE
(
    MB_LOCK_p   lock_p
);

Void_t          MB_LOCK_RELEASE
(
    MB_LOCK_p   lock_p
);

Void_t          MB_LOCK_DESTROY
(
    MB_LOCK_p lock_p
);
#endif // mbLock_h

