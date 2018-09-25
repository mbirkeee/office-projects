//---------------------------------------------------------------------------
// Function:    mbQueue.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Feb. 9 2002
//---------------------------------------------------------------------------
// Description:
//
// Queue management functions and macros.
//---------------------------------------------------------------------------

#ifndef mbQueue_h
#define mbQueue_h

// Setting the following option to 1 replaces some of the queue management
// macros with functions.
#define MB_Q_FUNCTION       (1)

#define MB_Q_MAGIC          (0xD30F2A23)

/* Queue entry structure */
typedef struct _qLinkage
{
    struct _qLinkage  *flink;
    struct _qLinkage  *blink;
} qLinkage_t;

typedef qLinkage_t *qLinkage_p;

/* Queue head structure */
typedef struct _qHead
{
    qLinkage_t          linkage;
    Int32u_t            size;
    Int32u_t            magic;
    mbLock_t            lock;
} qHead_t, *qHead_p;

/******************************************************************************
 * Function: qInitialize()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
qHead_p  qInitialize( qHead_p );
#else
#define qInitialize( _qHead_p )\
(_qHead_p );\
{ \
    ( (_qHead_p)->linkage.flink = (_qHead_p)->linkage.blink = (qLinkage_p)(_qHead_p) );\
    ( (_qHead_p)->size          = 0 );\
    ( (_qHead_p)->magic         = MAB_Q_MAGIC );\
    PMC_LOCK_INITITALIZE( (_qHead_p->lock );\
}
#endif /* MB_Q_FUNCTION */

/******************************************************************************
 * Function: qInitialized()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#define qInitialized( _qHead_p )\
    ( ( (_qHead_p)->magic == MAB_Q_MAGIC ) ? TRUE : FALSE )

/******************************************************************************
 * Function: qEmpty()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#define qEmpty( _qHead_p )\
    ( ( (_qHead_p)->linkage.flink == (qLinkage_p)(_qHead_p) ) ? TRUE : FALSE )

/******************************************************************************
 * Function: qRemoveFirst()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
qLinkage_p qRemoveFirst( qHead_p );
#else
#define qRemoveFirst( _qHead_p )\
     (qLinkage_p)((_qHead_p)->linkage.flink);\
     {\
        qLinkage_p _firstEntry;\
        _firstEntry               = (qLinkage_p)(_qHead_p)->linkage.flink;\
        _firstEntry->flink->blink = (qLinkage_p)(_qHead_p);\
        (_qHead_p)->linkage.flink = _firstEntry->flink;\
        (_qHead_p)->size--; \
     }
#endif /* MB_Q_FUNCTION */

/******************************************************************************
 * Function: qRemoveLast()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
qLinkage_p qRemoveLast( qHead_p );
#else
#define qRemoveLast( _qHead_p ) \
     (qLinkage_p)(_qHead_p)->linkage.blink;\
     {\
        qLinkage_p _lastEntry;\
        _lastEntry                = (_qHead_p)->linkage.blink;\
        _lastEntry->blink->flink  = (qLinkage_p)(_qHead_p);\
        (_qHead_p)->linkage.blink = _lastEntry->blink;\
        (_qHead_p)->size--; \
     }
#endif /* MB_Q_FUNCTION */

/******************************************************************************
 * Function: qRemoveEntry()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
Void_t  qRemoveEntryCast( qHead_p, qLinkage_p );
#define qRemoveEntry( _qHead_p, _entry_p ) qRemoveEntryCast( (qHead_p)(_qHead_p), (qLinkage_p)(_entry_p ));
#else
#define qRemoveEntry( _qHead_p, _entry_p )\
{\
     qLinkage_t _temp_entry;\
    _temp_entry.flink        = (_entry_p)->linkage.flink;\
    _temp_entry.blink        = (_entry_p)->linkage.blink;\
    _temp_entry.blink->flink = _temp_entry.flink;\
    _temp_entry.flink->blink = _temp_entry.blink;\
    (_qHead_p)->size--;\
}
#endif /* MB_Q_FUNCTION */

/******************************************************************************
 * Function: qInsertFirst()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
Void_t  qInsertFirstCast( qHead_p, qLinkage_p );
#define qInsertFirst( _qHead_p, _entry_p ) qInsertFirstCast( (qHead_p)(_qHead_p), (qLinkage_p)(_entry_p ));
#else
#define qInsertFirst( _qHead_p, _entry_p )\
{\
    (_entry_p)->linkage.flink        =   (_qHead_p)->linkage.flink;\
    (_entry_p)->linkage.blink        =   (qLinkage_p)(_qHead_p);\
    (_qHead_p)->linkage.flink->blink = &((_entry_p)->linkage);\
    (_qHead_p)->linkage.flink        = &((_entry_p)->linkage);\
    (_qHead_p)->size++;\
}
#endif

/******************************************************************************
 * Function: qInsertLast()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
Void_t  qInsertLastCast( qHead_p, qLinkage_p );
#define qInsertLast( _qHead_p, _entry_p ) qInsertLastCast( (qHead_p)(_qHead_p), (qLinkage_p)(_entry_p ));
#else
#define qInsertLast( _qHead_p, _entry_p )\
{\
     (_entry_p)->linkage.flink        =   (qLinkage_p)(_qHead_p);\
     (_entry_p)->linkage.blink        =   (_qHead_p)->linkage.blink;\
     (_qHead_p)->linkage.blink->flink = &((_entry_p)->linkage);\
     (_qHead_p)->linkage.blink        = &((_entry_p)->linkage); \
     (_qHead_p)->size++;\
}
#endif /* MB_Q_FUNCTION */

/******************************************************************************
 * Function: qInsertBefore()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
Void_t  qInsertBeforeCast( qHead_p, qLinkage_p, qLinkage_p );
#define qInsertBefore( _qHead_p, _entryExist_p, _entryNew_p ) qInsertBeforeCast( (qHead_p)(_qHead_p), (qLinkage_p)(_entryExist_p ), (qLinkage_p)(_entryNew_p ) );
#else
#define qInsertBefore( _qHead_p, _entryExist_p, _entryNew_p )\
{\
    (_entryNew_p)->linkage.flink           = &((_entryExist_p)->linkage);\
    (_entryNew_p)->linkage.blink           =   (_entryExist_p)->linkage.blink;\
    (_entryExist_p)->linkage.blink->flink  = &((_entryNew_p)->linkage);\
    (_entryExist_p)->linkage.blink         = &((_entryNew_p)->linkage);\
    (_qHead_p)->size++;\
}
#endif

/******************************************************************************
 * Function: qInsertAfter()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#if MB_Q_FUNCTION
Void_t  qInsertAfterCast( qHead_p, qLinkage_p, qLinkage_p );
#define qInsertAfter( _qHead_p, _entryExist_p, _entryNew_p ) qInsertAfterCast( (qHead_p)(_qHead_p), (qLinkage_p)(_entryExist_p ), (qLinkage_p)(_entryNew_p ) );
#else
#define qInsertAfter( _qHead_p, _entryExist_p, _entryNew_p )\
{\
    if( (_entryExist_p)->linkage.flink == (qLinkage_p)(_qHead_p) ) \
    {\
        /* Existing entry was at end of queue */\
        (_entryNew_p)->linkage.flink = (qLinkage_p)(_qHead_p) ;\
        (_qHead_p)->linkage.blink = &((_entryNew_p)->linkage); \
    }\
    else\
    {\
        /* Existing entry not at end of queue */\
        (_entryNew_p)->linkage.flink          =   (_entryExist_p)->linkage.flink;\
        (_entryNew_p)->linkage.flink->blink   =   &((_entryNew_p)->linkage);\
    }\
    (_entryExist_p)->linkage.flink = &((_entryNew_p)->linkage);\
    (_entryNew_p)->linkage.blink   = &((_entryExist_p)->linkage);\
    (_qHead_p)->size++;\
}
#endif

/******************************************************************************
 * Function: qWalk()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */
#define qWalk( _entry_p, _qHead_p, _type )              \
for( (_entry_p)  = (_type)(_qHead_p)->linkage.flink ;   \
     (_entry_p) != (_type)(_qHead_p) ;                  \
     (_entry_p)  = (_type)(_entry_p)->linkage.flink )

/******************************************************************************
 * Function: qInit()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#define qFirst( _head_p, _type ) (_type)((_head_p)->linkage.flink)

#define qLast(  _head_p, _type ) (_type)((_head_p)->linkage.blink)

#define qNext( _entry_p, _type ) (_type)((_entry_p)->linkage.flink)

#define qPrev( _entry_p, _type ) (_type)((_entry_p)->linkage.blink)

/******************************************************************************
 * Function: qContainingRecord()
 *-----------------------------------------------------------------------------
 * Description:
 *
 *  Calculate the address of the base of the structure given its type,
 * and an address of a field within the structure.
 *-----------------------------------------------------------------------------
 */

#define qContainingRecord( _address, _type, _field ) ((_type *)( \
                          (char *)(_address) - \
                          (char *)(&((_type *)0)->_field)))

#endif /* H_mbQueue */
