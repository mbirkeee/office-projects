/******************************************************************************
 * Copyright (c) 2008, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: $
 *-----------------------------------------------------------------------------
 * $Log:$
 *
 *-----------------------------------------------------------------------------
 */

#ifndef mbCond_h
#define mbCond_h

/* System include files */
#include <pthread.h>
#include <errno.h>

/******************************************************************************
 * Defines and Typedefs
 *-----------------------------------------------------------------------------
 */

typedef struct MbCondition_s
{
    pthread_mutex_t         mutex;
    pthread_mutexattr_t     attr;
    pthread_cond_t          condition;
    struct timespec         timeout;
    Int32u_t                lockCount;
} MbCondition_t, *MbCondition_p;

#define SYS_CONDITION_INIT( _cond )\
{\
    _cond.lockCount = 0;\
    if( pthread_mutexattr_init( &_cond.attr ) )\
    {\
        printf( "\n%s:%d: pthread_mutexattr_init() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    if( pthread_mutex_init( &_cond.mutex, &_cond.attr ) )\
    {\
        printf( "\n%s:%d: pthread_mutex_init() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    if( pthread_cond_init( &_cond.condition, NIL ) )\
    {\
        printf( "\n%s:%d: pthread_cond_init() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
}

/* Wait automatically unlocks the mutex.  This we must enter the    */
/* WAIT in a locked state.                                          */

#define SYS_CONDITION_WAIT( _cond )\
{\
    if( _cond.lockCount == 0 )\
    {\
        printf( "\n%s:%d:  SYS_CONDITION_WAIT lockCount == 0 \n", __FUNCTION__, __LINE__ );\
    }\
    if( pthread_cond_wait( &_cond.condition, &_cond.mutex ) )\
    {\
        printf( "\n%s:%d: pthread_cond_wait() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
}

#define SYS_CONDITION_WAIT_SEC( _cond, _seconds )\
{\
    Ints_t _presult;\
    _cond.timeout.tv_sec = time( 0 );\
    _cond.timeout.tv_sec += _seconds;\
    if( _cond.lockCount == 0 )\
    {\
        printf( "\n%s:%d: SYS_CONDITION_WAIT_SEC lockCount == 0\n", __FUNCTION__, __LINE__ );\
    }\
    _presult = pthread_cond_timedwait( &_cond.condition, &_cond.mutex, &_cond.timeout );\
    if( _presult == ETIMEDOUT ) _seconds = 0;\
    if( _presult !=0 && _presult != ETIMEDOUT )\
    {\
        printf( "\n%s:%d: pthread_cond_timedwait() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
}

#define SYS_CONDITION_SET( _cond )\
{\
    clock_t  _startTime = times( 0 );\
    clock_t  _totalTime;\
    if( pthread_mutex_lock( &_cond.mutex ) )\
    {\
        printf( "\n%s:%d: pthread_mutex_lock() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    _cond.lockCount++;\
    if( pthread_cond_signal( &_cond.condition ) )\
    {\
        printf( "\n%s:%d: pthread_cond_wait() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    if( _cond.lockCount == 0 )\
    {\
        printf( "\n%s:%d: SYS_CONDITION_WAIT_UNLOCK lockCount == 0\n", __FUNCTION__, __LINE__ );\
    }\
    _cond.lockCount--;\
    if( pthread_mutex_unlock( &_cond.mutex ) )\
    {\
        printf( "\n%s:%d: pthread_mutex_lock() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    _totalTime = times( 0 ) - _startTime ;\
    if( _totalTime >= 10 )\
    {\
        SYS_LOG( "%s:%d Waited %ld ticks to set condition (%ld)\n", __FUNCTION__, __LINE__, _totalTime, _startTime );\
    }\
}

#define SYS_CONDITION_LOCK( _cond )\
{\
    if( pthread_mutex_lock( &_cond.mutex ) )\
    {\
        printf( "\n%s:%d: pthread_mutex_lock() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
    _cond.lockCount++;\
}

#define SYS_CONDITION_UNLOCK( _cond )\
{\
    if( _cond.lockCount == 0 )\
    {\
        printf( "\n%s:%d: SYS_CONDITION_WAIT_UNLOCK lockCount == 0\n", __FUNCTION__, __LINE__ );\
    }\
    _cond.lockCount--;\
    if( pthread_mutex_unlock( &_cond.mutex ) )\
    {\
        printf( "\n%s:%d: pthread_mutex_lock() returned an error\n", __FUNCTION__, __LINE__ );\
    }\
}


#endif /* mbCond_h */
