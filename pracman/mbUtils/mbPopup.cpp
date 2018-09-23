//---------------------------------------------------------------------------
// File:    mbPopup.cpp
//---------------------------------------------------------------------------
// Author:  Michael Bree (c) 2005, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 2, 2005
//---------------------------------------------------------------------------
// Description:
//
// Popup menu utilities
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbDlg.h"
#include "mbPopup.h"
#include "mbStrUtils.h"

//---------------------------------------------------------------------------
// Function: mbPopupEnableAll
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t  mbPopupEnableAll( TPopupMenu *menu_p, Boolean_t enable )
{
    return mbPopupEnableItem( menu_p, NIL, enable );
}

//---------------------------------------------------------------------------
// Function: mbPopupEnableItem
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Boolean_t  mbPopupEnableItem( TPopupMenu *menu_p, Char_p name_p, Boolean_t enable )
{
    Boolean_t   returnCode = FALSE;
    Boolean_t   found = FALSE;

    // Sanity check
    if( menu_p == NIL )
    {
        mbDlgError( "invalid popup menu pointer" );
        goto exit;
    }

    if( name_p == NIL )
    {
        found = TRUE;
    }
    else
    {
        if( strlen( name_p ) == 0 )
        {
            mbDlgError( "invalid popup menu item" );
            goto exit;
        }
    }

    // OK, try to carry out request
    mbPopupEnableItemRecurse( menu_p->Items, name_p, enable, &found );

    if( !found )
    {
        mbDlgError( "Menu item '%s' not found", name_p );
        goto exit;

    }
    returnCode = TRUE;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: mbPopupEnableItemRecurse
//---------------------------------------------------------------------------
// Description:
//
// Loop through all items and sub items.  Return code indicates parent
// should set state.  This happens only when enabling a specific item.
// name_p == NIL indicates all items should have state set
//---------------------------------------------------------------------------

Boolean_t       mbPopupEnableItemRecurse
(
    TMenuItem  *item_p,
    Char_p      name_p,
    Boolean_t   enable,
    Boolean_p   found_p
)
{
    Int32s_t    returnCode = FALSE;
    Ints_t      i, count;
    Boolean_t   set;
    Boolean_t   recurse = TRUE;

    if( item_p == NIL ) goto exit;

    count = item_p->Count;
    //mbDlgInfo( "Count: %d\n", item_p->Count );

    // Loop through all items at this level
    for( i = 0 ; i < count ; i++ )
    {
        set = FALSE;
        if( name_p == NIL )
        {
            // If name_p == NIL, we set the state no matter what
            set = TRUE;
        }
        else
        {
            if( mbStrPos( item_p->Items[i]->Name.c_str(), name_p ) == 0 )
            {
                *found_p = TRUE;
                set = TRUE;
                recurse = FALSE;
            }
        }

        //mbDlgInfo( "Name: %s\n", item_p->Items[i]->Name.c_str( ) );

        if( recurse )
        {
            // Check if setting child should cause this (parent) to be set
            if( mbPopupEnableItemRecurse( item_p->Items[i], name_p, enable, found_p ) == TRUE )
            {
                set = TRUE;
            }
        }

        if( set )
        {
            item_p->Items[i]->Enabled = enable;
            if( enable == TRUE && name_p != NIL )
            {
                // Signal that parent should also set state
                returnCode = TRUE;
                break;
            }
        }
    }

exit:
    return returnCode;
}
