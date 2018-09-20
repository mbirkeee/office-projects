/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbLock.h,v 1.1.1.1 2007/07/10 05:06:02 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbLock.h,v $
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef mbLock_h
#define mbLock_h

/* System include files */
#include <pthread.h>
#include <errno.h>

/******************************************************************************
 * Defines and Typedefs
 *-----------------------------------------------------------------------------
 */

typedef struct MbLock_s
{
    pthread_mutex_t         mutex;
    pthread_mutexattr_t     attr;
    pthread_mutexattr_t    *attr_p;
    pthread_mutex_t        *mutex_p;
    Int32u_t                count;
    Boolean_t               createdFlag;
} MbLock_t, *MbLock_p;

/******************************************************************************
 * Function Prototypes
 *-----------------------------------------------------------------------------
 */

Int32s_t    mbLockFileGet( Char_p lockFile_p );

/******************************************************************************
 * Macros
 *-----------------------------------------------------------------------------
 */

#define MB_LOCK_CREATE( _lk_p )                             \
{                                                           \
    _lk_p = malloc( sizeof( Lock_t ) );                     \
    _lk_p->attr_p  = &_lk_p->attr;                          \
    _lk_p->mutex_p = &_lk_p->mutex;                         \
    pthread_mutexattr_init( _lk_p->attr_p );                \
    pthread_mutex_init( _lk_p->mutex_p, _lk_p->attr_p );    \
    _lk_p->count = 0;                                       \
    _lk_p->createdFlag = TRUE;                              \
}

#define MB_LOCK_INIT( _lk_ )                                \
&_lk_;                                                      \
{                                                           \
    _lk_.attr_p  = &_lk_.attr;                              \
    _lk_.mutex_p = &_lk_.mutex;                             \
    pthread_mutexattr_init( _lk_.attr_p );                  \
    pthread_mutex_init( _lk_.mutex_p, _lk_.attr_p );        \
    _lk_.count = 0;                                         \
    _lk_.createdFlag = FALSE;                               \
}

#define MB_LOCK_ACQUIRE( _lk_p )                            \
{                                                           \
    if( pthread_mutex_lock( _lk_p->mutex_p ) )              \
    {                                                       \
        printf( "\n\nMB_LOCK_ACQUIRE() error: pthread_mutex_lock() returned an error\n" );\
    }                                                       \
    else                                                    \
    {                                                       \
        if( ++_lk_p->count != 1 )                           \
        {                                                   \
            printf( "\n\nMB_LOCK_ACQUIRE() error: lock.count %ld != 1\n", _lk_p->count );\
        }                                                   \
    }                                                       \
}

#define MB_LOCK_RELEASE( _lk_p )                            \
{                                                           \
    if( _lk_p->count != 1 )                                 \
    {                                                       \
        printf( "\n\nMB_LOCK_RELEASE() error: lock.count %ld != 1\n", _lk_p->count );\
    }                                                       \
    else                                                    \
    {                                                       \
        _lk_p->count--;                                     \
        if( pthread_mutex_unlock( _lk_p->mutex_p ) )        \
        {                                                   \
            printf( "\n\nMB_LOCK_RELEASE() error: pthread_mutex_unlock() returned an error\n" );\
        }                                                   \
    }                                                       \
}

/* Note: it is not really safe to just free the lock object... eventually we */
/* should have a tracking system to keep track of threads waiting on locks, etc */
#define MB_LOCK_FREE( _lk_p )                               \
{                                                           \
    if( _lk_p != NIL )                                      \
    {                                                       \
        if( _lk_p->createdFlag == TRUE ) free( _lk_p );     \
        _lk_p = NIL;                                        \
    }                                                       \
}

#endif /* mbLock_h */
