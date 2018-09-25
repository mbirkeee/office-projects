//---------------------------------------------------------------------------
// Function:    mbOpup.h
//---------------------------------------------------------------------------
// Author:      Michael Bree (c) 2005, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        January 2, 2005
//---------------------------------------------------------------------------
// Description:
//
// Popup menu utilities
//---------------------------------------------------------------------------

#ifndef mbPopup_h
#define mbPopup_h

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Boolean_t       mbPopupEnableAll
(
    TPopupMenu *menu_p,
    Boolean_t   enable
);


Boolean_t       mbPopupEnableItem
(
    TPopupMenu *menu_p,
    Char_p      name_p,
    Boolean_t   enable
);

Boolean_t       mbPopupEnableItemRecurse
(
    TMenuItem  *item_p,
    Char_p      name_p,
    Boolean_t   enable,
    Boolean_p   found_p
);

#endif // mbPopup_h

