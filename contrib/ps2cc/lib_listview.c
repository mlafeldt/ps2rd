/****************************************************************************
ListView API Library

This contains my homemade functions to make manipulating ListView controls
easier and cleaner.

NO GLOBALS
*****************************************************************************/
#include "ps2cc.h"

/**********************************************
ListViewAddRow - Add a row to a listview
***********************************************/
int ListViewAddRow(HWND hListView, int count, ...)
{
    va_list arglist;
    int i;
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.cchTextMax = 1024;
    int iCount = SendMessage(hListView, LVM_GETITEMCOUNT, 0, 0);
    va_start(arglist, count);
    LvItem.iItem=iCount; LvItem.iSubItem=0;
    LvItem.pszText=va_arg(arglist, char *);
    SendMessage(hListView, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
    for (i = 1; i < count; i++) {
        LvItem.iSubItem=i;
        LvItem.pszText=va_arg(arglist, char *);
        SendMessage(hListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
    }
    va_end(arglist);
    return iCount;
}

/**********************************************
ListViewSetRow - Set item/subitem(s) in a listview
***********************************************/
int ListViewSetRow(HWND hListView, int item, int subitem, int count, ...)
{
    va_list arglist;
    int i;
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.mask=LVIF_TEXT;
    LvItem.cchTextMax = 1024;
    va_start(arglist, count);
    LvItem.iItem=item; LvItem.iSubItem=subitem;
    i = 0;
    do {
        LvItem.iSubItem += i;
        LvItem.pszText = va_arg(arglist, char *);
        SendMessage(hListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
        i++;
    } while (i < count);
    va_end(arglist);
    return 0;
}


/**********************************************
ListViewAddCol - Add a column to a listview
***********************************************/
int ListViewAddCol(HWND hListView, const char* colName, int colNum, int colWidth)
{
    LVCOLUMN LvCol; memset(&LvCol,0,sizeof(LvCol));
    LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
    LvCol.cx=colWidth;
    LvCol.pszText=(char*)colName;
    LvCol.cchTextMax = lstrlen( LvCol.pszText );
    SendMessage(hListView,LVM_INSERTCOLUMN,colNum,(LPARAM)&LvCol);
    return 0;
}

/**********************************************
ListViewGetText - Grab the text from a listview item/subitem
***********************************************/
int ListViewGetText(HWND hListView, int iNum, int iSub, char* iText, int txtLen)
{
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.iItem = iNum;
    LvItem.iSubItem = iSub;
    LvItem.mask = LVIF_TEXT;
    LvItem.cchTextMax = txtLen;
    LvItem.pszText = iText;
    SendMessage(hListView, LVM_GETITEMTEXT, iNum, (LPARAM)&LvItem);
    return 0;
}

/**********************************************
ListViewGetHex - Grab the hex value from a listview item/subitem
***********************************************/
u64 ListViewGetHex(HWND hListView, int iNum, int iSub)
{
    char iText[20];
    u64 tvalue;
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.iItem = iNum;
    LvItem.iSubItem = iSub;
    LvItem.mask = LVIF_TEXT;
    LvItem.cchTextMax = 20;
    LvItem.pszText = iText;
    SendMessage(hListView, LVM_GETITEMTEXT, iNum, (LPARAM)&LvItem);
    if (!isHex(iText)) { return 0; }
    sscanf(iText,"%I64x",&tvalue);
    return tvalue;
}

/**********************************************
ListViewGetDec - Grab the dec value from a listview item/subitem
***********************************************/
u64 ListViewGetDec(HWND hListView, int iNum, int iSub)
{
    char iText[20];
    u64 tvalue;
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.iItem = iNum;
    LvItem.iSubItem = iSub;
    LvItem.mask = LVIF_TEXT;
    LvItem.cchTextMax = 20;
    LvItem.pszText = iText;
    SendMessage(hListView, LVM_GETITEMTEXT, iNum, (LPARAM)&LvItem);
    if (!isDec(iText)) { return 0; }
    sscanf(iText,"%I64u",&tvalue);
    return tvalue;
}

/**********************************************
ListViewGetFloat - Grab the float value from a listview item/subitem
***********************************************/
u64 ListViewGetFloat(HWND hListView, int iNum, int iSub, int fsize)
{
    char iText[20];
//    u64 tvalue;
    LVITEM LvItem;  memset(&LvItem,0,sizeof(LvItem));
    LvItem.iItem = iNum;
    LvItem.iSubItem = iSub;
    LvItem.mask = LVIF_TEXT;
    LvItem.cchTextMax = 20;
    LvItem.pszText = iText;
    SendMessage(hListView, LVM_GETITEMTEXT, iNum, (LPARAM)&LvItem);
    float tmpFloat;
    double tmpDouble;
    if (!isFloat(iText)) { return 0; }
    if (fsize == 4 ) {
        sscanf(iText, "%f", &tmpFloat);
        return Float2Hex(tmpFloat);
    }
    else {
        sscanf(iText, "%Lf", &tmpDouble);
        return Double2Hex(tmpDouble);
    }
}

/**********************************************
ListViewHitTst - hit test/ subitem hit test
***********************************************/
int ListViewHitTst(HWND hListView, DWORD dwPos, int iItem)
{
    POINT pLoc = { GET_X_LPARAM( dwPos ), GET_Y_LPARAM( dwPos ) };
    ScreenToClient( hListView, &pLoc );
    LVHITTESTINFO lvHit; memset(&lvHit,0,sizeof(lvHit));
    lvHit.pt.x = pLoc.x;
    lvHit.pt.y = pLoc.y;
    if (iItem == -1) { return SendMessage(hListView, LVM_HITTEST, 0, (LPARAM)&lvHit); }
    lvHit.iItem = iItem;
    SendMessage(hListView, LVM_SUBITEMHITTEST, 0, (LPARAM)&lvHit);
    return lvHit.iSubItem;
}

