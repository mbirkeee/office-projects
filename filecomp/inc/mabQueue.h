/******************************************************************************
 * File: mabQueue.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2001 Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: February 8, 2001
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Queue management macros.
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/filecomp/inc/mabQueue.h,v 1.2 2006/12/05 13:47:07 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mabQueue.h,v $
 * Revision 1.2  2006/12/05 13:47:07  mikeb
 * Update queue macros
 * Add totals to end of display
 * dos2unix files
 *
 * Revision 1.1.1.1  2006/05/23 01:51:24  mikeb
 * Initial Import
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef H_mabQueue
#define H_mabQueue

#include "mabGeneral.h"

#define MAB_Q_MAGIC     (0xD30F2A23)

/* Queue entry structure */
typedef struct _qEntry
{
    struct _qEntry  *flink;
    struct _qEntry  *blink;
} qEntry_t;

typedef qEntry_t *qEntry_p;

/* Queue head structure */
typedef struct _qHead
{
    qEntry_t            linkage;
    Int32u_t            size;
    Int32u_t            magic;
} qHead_t, *qHead_p;

#define qInitialize( _qHead_p ) \
(_qHead_p );\
{ \
    ( (_qHead_p)->linkage.flink = (_qHead_p)->linkage.blink = (qEntry_p)(_qHead_p) );\
    ( (_qHead_p)->size          = 0 );\
    ( (_qHead_p)->magic         = MAB_Q_MAGIC );\
}

#define qInitialized( _qHead_p )\
    ( ( (_qHead_p)->magic == MAB_Q_MAGIC ) ? TRUE : FALSE )

#define qEmpty( _qHead_p )\
    ( ( (_qHead_p)->linkage.flink == (qEntry_p)(_qHead_p) ) ? TRUE : FALSE )

#define qRemoveFirst( _qHead_p )\
     (_qHead_p)->linkage.flink;\
     {\
        qEntry_p _firstEntry;\
        _firstEntry = (_qHead_p)->linkage.flink;\
        _firstEntry->flink->blink = (qEntry_p)(_qHead_p);\
        (_qHead_p)->linkage.flink = _firstEntry->flink;\
        (_qHead_p)->size--; \
     }

#define qRemoveLast( _qHead_p ) \
     (_qHead_p)->linkage.blink;\
     {\
        qEntry_p _lastEntry;\
        _lastEntry = (_qHead_p)->linkage.blink;\
        _lastEntry->blink->flink = (qEntry_p)(_qHead_p);\
        (_qHead_p)->linkage.blink = _lastEntry->blink;\
        (_qHead_p)->size--; \
     }

#define qRemoveEntry( _qHead_p, _entry ) \
    {\
         qEntry_t _temp_entry;\
        _temp_entry = (_entry)->linkage;\
        _temp_entry.blink->flink = _temp_entry.flink;\
        _temp_entry.flink->blink = _temp_entry.blink;\
        (_qHead_p)->size--; \
    }

#define qInsertFirst( _qHead_p, _struct_p ) \
     (_struct_p)->linkage.flink       =   (_qHead_p)->linkage.flink;\
     (_struct_p)->linkage.blink       =   (qEntry_p)(_qHead_p);\
     (_qHead_p)->linkage.flink->blink = &((_struct_p)->linkage);\
     (_qHead_p)->linkage.flink        = &((_struct_p)->linkage);\
     (_qHead_p)->size++

#define qInsertLast( _qHead_p, _struct_p ) \
     (_struct_p)->linkage.flink = (qEntry_p)(_qHead_p);\
     (_struct_p)->linkage.blink = (_qHead_p)->linkage.blink;\
     (_qHead_p)->linkage.blink->flink = &((_struct_p)->linkage);\
     (_qHead_p)->linkage.blink = &((_struct_p)->linkage); \
     (_qHead_p)->size++

#define qInsertBefore( _qHead_p,  _structExist_p, _structNew_p ) \
    (_structNew_p)->linkage.flink           = &((_structExist_p)->linkage); \
    (_structNew_p)->linkage.blink           =   (_structExist_p)->linkage.blink;\
    (_structExist_p)->linkage.blink->flink  = &((_structNew_p)->linkage);\
    (_structExist_p)->linkage.blink         = &((_structNew_p)->linkage);\
    (_qHead_p)->size++

#define qInsertAfter( _qHead_p, _structExist_p, _structNew_p ) \
    if( (_structExist_p)->linkage.flink == (qEntry_p)(_qHead_p) ) \
    {\
        /* Exising struct was at end of queue */\
        (_structNew_p)->linkage.flink = (qEntry_p)(_qHead_p) ;\
        (_qHead_p)->linkage.blink = &((_structNew_p)->linkage); \
    }\
    else\
    {\
        /* Existing struct not at end of queue */\
        (_structNew_p)->linkage.flink          =   (_structExist_p)->linkage.flink;\
        (_structNew_p)->linkage.flink->blink   =   &((_structNew_p)->linkage);\
    }\
    (_structExist_p)->linkage.flink         = &((_structNew_p)->linkage);\
    (_structNew_p)->linkage.blink           = &((_structExist_p)->linkage);\
    (_qHead_p)->size++

#define qWalk( _entry_p, _qHead_p, _type )		  \
  for( (_entry_p)  = (_type)(_qHead_p)->linkage.flink ;   \
       (_entry_p) != (_type)(_qHead_p) ;                  \
       (_entry_p)  = (_type)(_entry_p)->linkage.flink )


#define qFirst( _head_p, _type ) (_type)((_head_p)->linkage.flink)

#define qLast(  _head_p, _type ) (_type)((_head_p)->linkage.blink)

#define qNext( _entry_p, _type ) (_type)((_entry_p)->linkage.flink)

#define qPrev( _entry_p, _type ) (_type)((_entry_p)->linkage.blink)

/* Calculate the address of the base of the structure given its type, */
/* and an address of a field within the structure. */

#define qContainingRecord( _address, _type, _field ) ((_type *)( \
                          (char *)(_address) - \
                          (char *)(&((_type *)0)->_field)))

#endif /* H_mabQueue */
