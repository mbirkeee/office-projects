//---------------------------------------------------------------------------
// File:    pmcQueue.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 21, 2001
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbQueue.h"

#if MB_Q_FUNCTION

/******************************************************************************
 * Function: qInitialize()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

qHead_p qInitialize( qHead_p qp )
{
    qp->linkage.flink = qp->linkage.blink = (qLinkage_p)qp;
    qp->size          = 0;
    qp->magic         = MB_Q_MAGIC;
    mbLockInit( qp->lock );
    return qp;
}

/******************************************************************************
 * Function: qRemoveFirst()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

qLinkage_p qRemoveFirst( qHead_p qp )
{
    qLinkage_p  return_p;
    qLinkage_p  firstEntry_p;

    return_p = (qLinkage_p)qp->linkage.flink;
    firstEntry_p = (qLinkage_p)qp->linkage.flink;
    firstEntry_p->flink->blink = (qLinkage_p)qp;
    qp->linkage.flink = firstEntry_p->flink;
    qp->size--;

    return return_p;
}

/******************************************************************************
 * Function: qRemoveLast()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

qLinkage_p qRemoveLast( qHead_p qp )
{
    qLinkage_p  return_p;
    qLinkage_p  lastEntry_p;

    return_p = (qLinkage_p)qp->linkage.blink;
    lastEntry_p = qp->linkage.blink;

    lastEntry_p->blink->flink  = (qLinkage_p)qp;
    qp->linkage.blink = lastEntry_p->blink;
    qp->size--;

    return return_p;
}

/******************************************************************************
 * Function: qRemoveEntry()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Void_t  qRemoveEntryCast( qHead_p qp, qLinkage_p entry_p )
{
    qLinkage_t temp_entry;

    temp_entry.flink        = entry_p->flink;
    temp_entry.blink        = entry_p->blink;
    temp_entry.blink->flink = temp_entry.flink;
    temp_entry.flink->blink = temp_entry.blink;
    qp->size--;
}

/******************************************************************************
 * Function: qInsertFirst()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Void_t qInsertFirstCast( qHead_p qp, qLinkage_p entry_p )
{
    entry_p->flink              = qp->linkage.flink;
    entry_p->blink              = (qLinkage_p)qp;
    qp->linkage.flink->blink    = entry_p;
    qp->linkage.flink           = entry_p;
    qp->size++;
}

/******************************************************************************
 * Function: qInsertLast()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Void_t qInsertLastCast( qHead_p qp, qLinkage_p entry_p )
{
     entry_p->flink             = (qLinkage_p)qp;
     entry_p->blink             = qp->linkage.blink;
     qp->linkage.blink->flink   = entry_p;
     qp->linkage.blink          = entry_p;
     qp->size++;
}

/******************************************************************************
 * Function: qInsertBefore()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Void_t qInsertBeforeCast( qHead_p qp, qLinkage_p entryExist_p, qLinkage_p entryNew_p )
{
    entryNew_p->flink           =  entryExist_p;
    entryNew_p->blink           =  entryExist_p->blink;
    entryExist_p->blink->flink  =  entryNew_p;
    entryExist_p->blink         =  entryNew_p;
    qp->size++;
}

/******************************************************************************
 * Function: qInsertAfter()
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Void_t qInsertAfterCast( qHead_p qp, qLinkage_p entryExist_p, qLinkage_p entryNew_p )
{
    if( entryExist_p->flink == (qLinkage_p)qp )
    {
        /* Existing entry was at end of queue */
        entryNew_p->flink   = (qLinkage_p)qp;
        qp->linkage.blink   = entryNew_p;
    }
    else
    {
        /* Existing entry not at end of queue */
        entryNew_p->flink          = entryExist_p->flink;
        entryNew_p->flink->blink   = entryNew_p;
    }
    entryExist_p->flink = entryNew_p;
    entryNew_p->blink   = entryExist_p;
    qp->size++;
}

#endif /* MB_Q_FUNCTION */



