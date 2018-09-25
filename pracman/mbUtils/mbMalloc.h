//----------------------------------------------------------------------------
// File:    mbMalloc.h
//----------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//----------------------------------------------------------------------------
// Date:    August 1, 2002
//----------------------------------------------------------------------------
// Description:
//----------------------------------------------------------------------------

#ifndef mbMalloc_h
#define mbMalloc_h

#include "mbLock.h"
#include "mbQueue.h"

#define MB_MALLOC_DEBUG     (1)

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

//----------------------------------------------------------------------------
// Global Valiables
//----------------------------------------------------------------------------

External Int32u_t   mbAllocatedMemoryChunks
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

#if MB_MALLOC_DEBUG

External Boolean_t  mbMallocInitFlag
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External Boolean_t  mbMallocListFlag
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External Int32u_t   mbObjectCount
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External qHead_t    mbMallocQueue;
External qHead_p    mbMalloc_q;

typedef struct  mbMemList_s
{
    qLinkage_t  linkage;
    Void_p      ptr;
    Ints_t      size;
    Int32u_t    line;
    Char_p      file_p;
} mbMemList_t, *mbMemList_p;

// Utility for keeping track of allocated object counts (debugging)
Int32u_t    mbObjectCountInc( void );
Int32u_t    mbObjectCountDec( void );
Int32u_t    mbObjectCountGet( void );

void            mbFreeFunc
(
    Void_p      ptr_p,
    Char_p      fileName_p,
    Int32u_t    line
);

Void_p          mbMallocFunc
(
    Void_p     *ptr_pp,
    Ints_t      size,
    Char_p      fileName_p,
    Int32u_t    line,
    Boolean_t   callocFlag
);

void            mbMallocTest
(
    Void_p     *ptr_pp,
    Ints_t      size
);

#else
Void_p          mbMallocFunc
(
    Void_p     *ptr_pp,
    Ints_t      size,
    Boolean_t   callocFlag
);

#endif // MB_MALLOC_DEBUG

Int32s_t        mbMallocInit
(
    Boolean_t   listFlag
);

void            mbMallocDump
(
    void
);

Int32u_t        mbMallocChunks
(
    void
);

#if MB_MALLOC_DEBUG

#define mbMalloc( _parm, _size ) mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), (Char_p)__FILE__, (Int32u_t)__LINE__, FALSE )
#define mbCalloc( _parm, _size ) mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), (Char_p)__FILE__, (Int32u_t)__LINE__, TRUE )


#define mbFree( _parm )\
{\
    mbFreeFunc( (void *)(_parm), (Char_p)__FILE__, (Int32u_t)__LINE__  );\
}

#define mbRemalloc( _parm, _size )\
{\
    mbFreeFunc( (void *)(_parm), (Char_p)__FILE__, (Int32u_t)__LINE__  );\
    mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), (Char_p)__FILE__, (Int32u_t)__LINE__, FALSE );\
}

#define mbRecalloc( _parm, _size )\
{\
    mbFreeFunc( (void *)(_parm), (Char_p)__FILE__, (Int32u_t)__LINE__  );\
    mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), (Char_p)__FILE__, (Int32u_t)__LINE__, FALSE );\
    memset( (Void_p)(_parm), 0, (Ints_t)(_size) );\
}

#else // MB_MALLOC_DEBUG

#define mbMalloc( _parm, _size ) mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), FALSE )
#define mbCalloc( _parm, _size ) mbMallocFunc( (void **)(&(_parm)), (Ints_t)(_size), TRUE )

#if 0
#define mbMalloc( _parm, _size )\
{\
    size_t _mysize = (_size);\
    if( _mysize == 0 ) _mysize = 8;\
    if( ( (void *)(_parm) = malloc( _mysize ) ) == NIL )\
    {\
        mbDlgDebug(( "Memory Allocation error (size:%d)", (Ints_t)_mysize ));\
    }\
    else\
    { \
        mbAllocatedMemoryChunks++;\
    }\
}


#define mbCalloc( _parm, _size )\
{\
    size_t _mysize = (_size);\
    if( _mysize == 0 ) (_mysize) = 8;\
    if( ( (void *)(_parm) = malloc( _mysize ) ) == NIL )\
    {\
        mbDlgDebug(( "Memory Allocation error (size:%d)", (Ints_t)_mysize ));\
    }\
    else\
    { \
        mbAllocatedMemoryChunks++;\
        memset( (void *)(_parm), 0 , _mysize );\
    }\
}

#endif // 0


#define mbRemalloc( _parm, _size )\
{\
    mbFree( _parm );\
    mbMalloc( (_parm), size );\
}

#define mbRecalloc( _parm, _size )\
{\
    mbFree( _parm );\
    mbCalloc( (_parm), size );\
}

#define mbFree( _parm )\
{\
    if( _parm != NIL )\
    {\
        free( _parm );\
        mbAllocatedMemoryChunks--;\
        _parm = NIL;\
    }\
}

#endif // MB_MALLOC_DEBUG

#define mbMallocStr( _parm_p, _str_p )\
{\
    size_t _len = 0;\
    if( (_str_p) != NIL ) _len = strlen( _str_p );\
    mbMalloc( (_parm_p), _len + 1 );\
    if( _str_p )\
    {\
        strcpy( (_parm_p), (_str_p) );\
    }\
    else\
    {\
        *(_parm_p) = 0;\
    }\
}

#define mbRemallocStr( _parm_p, _str_p )\
{\
    size_t _len = 0;\
    if( _parm_p ) mbFree( _parm_p );\
    if( _str_p ) _len = strlen( _str_p );\
    mbMalloc( (_parm_p), _len + 1 );\
    if( _str_p )\
    {\
        strcpy( (_parm_p), (_str_p) );\
    }\
    else\
    {\
        *(_parm_p) = 0;\
    }\
}

#endif // mbMalloc_h




