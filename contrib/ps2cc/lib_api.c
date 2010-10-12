/****************************************************************************
API Library

This is mostly a collection of API related functions/wrappers to make my life
easier--most of which were written for RenegadeEx with generic portability in
mind.

NO GLOBALS
*****************************************************************************/

#include "ps2cc.h"

/****************************************************************************
Combobox Add Item
*****************************************************************************/
int ComboAddItem(HWND hCombo, const char* combostring, DWORD value)
{
    int iCount = SendMessage(hCombo,CB_GETCOUNT,0,0);
    SendMessage(hCombo,CB_ADDSTRING,iCount,(LPARAM)combostring);
    return SendMessage(hCombo,CB_SETITEMDATA,iCount,value);
}

/****************************************************************************
ComboSelFromData - Set selected combo box item based on item data
*****************************************************************************/
int ComboSelFromData (HWND hCombo, u32 DataValue)
{
    int i;
    int NumItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
    for (i = 0; i < NumItems; i++) {
        if (SendMessage(hCombo, CB_GETITEMDATA, i, 0) == DataValue) {
            SendMessage(hCombo,CB_SETCURSEL,i,0); return i;
        }
    }
    return -1;
}

/****************************************************************************
Hex window functions
*****************************************************************************/
int isHexWindow(HWND txtbox)
{
    char txtInput[17];
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    return isHex(txtInput);
}

u64 GetHexWindow(HWND txtbox)
{
    char txtInput[17];
    u64 tvalue=0;
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    if (isHex(txtInput)) { sscanf(txtInput,"%I64x",&tvalue); }
    return tvalue;
}

int SetHexWindow(HWND txtbox, u64 value)
{
    char txtValue[20], fmtString[20];
    sprintf(fmtString, "%%0%uI64X", SendMessage(txtbox, EM_GETLIMITTEXT, 0, 0));
    sprintf(txtValue, fmtString, value);
    SetWindowText(txtbox, txtValue);
    return 0;
}

/****************************************************************************
Decimal window functions
*****************************************************************************/
u64 isDecWindow(HWND txtbox)
{
    char txtInput[31];
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    return isDec(txtInput);
}
u64 GetDecWindow(HWND txtbox)
{
    char txtInput[31];
    u64 tvalue=0;
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    if (isDec(txtInput)) { sscanf(txtInput,"%I64u",&tvalue); }
    return tvalue;
}
u64 GetDecWindowS(HWND txtbox)
{
    char txtInput[31];
    u64 tvalue=0;
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    if (isDec(txtInput)) { sscanf(txtInput,"%I64d",&tvalue); }
    return tvalue;
}
int SetDecWindowU(HWND txtbox, u64 value)
{
    char txtValue[31];
    sprintf(txtValue, "%I64u", value);
    SetWindowText(txtbox, txtValue);
    return 0;
}

int SetDecWindowS(HWND txtbox, u64 value)
{
    char txtValue[31];
    sprintf(txtValue, "%I64d", value);
    SetWindowText(txtbox, txtValue);
    return 0;
}

/****************************************************************************
Float window functions
*****************************************************************************/
u64 IsFloatWindow(HWND txtbox)
{
    char txtInput[30];
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    return isFloat(txtInput);
}

u64 GetFloatWindow(HWND txtbox, int fsize)
{
    char txtInput[30];
    float tmpFloat;
    double tmpDouble;
    GetWindowText(txtbox, txtInput, sizeof(txtInput));
    if (!isFloat(txtInput)) { return 0; }
    if (fsize == 4 ) {
        sscanf(txtInput, "%f", &tmpFloat);
        return Float2Hex(tmpFloat);
    }
    else {
        sscanf(txtInput, "%Lf", &tmpDouble);
        return Double2Hex(tmpDouble);
    }
}

int SetFloatWindow(HWND txtbox, u64 value, int fsize)
{
    char txtValue[31];
    float tmpFloat=0;
    u32 *CastFloat=(u32*)(&tmpFloat);
    double tmpDouble=0;
    u64 *CastDouble=(u64*)&tmpDouble;
    if (fsize == 4) {
    	*CastFloat = value & 0xFFFFFFFF;
    	sprintf(txtValue, "%f", tmpFloat);
    } else {
    	*CastDouble = value;
    	sprintf(txtValue, "%Lf", tmpDouble);
    }
    SetWindowText(txtbox, txtValue);
    return 0;
}

/****************************************************************************
Pick File dialogs
--To Do: Possibly update these so extension filter can be set through arguments
*****************************************************************************/
int DoFileOpen(HWND hwnd, char* filename)
{
	OPENFILENAME ofn;
	strcpy(filename,"");

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//	ofn.lpstrDefExt = "txt";

	if(GetOpenFileName(&ofn)) //success
	{
//	    if (ofn.Flags & OFN_READONLY) { *access = 1; }
//	    else { *access = 0; }
        return 1;
	} else { return 0; }
}

//To Do: Update these so extension filter can be set through arguments
int DoFileSave(HWND hwnd, char* filename)
{
	OPENFILENAME ofn;
	strcpy(filename,"");
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
//	ofn.lpstrDefExt = "txt";

	if(GetSaveFileName(&ofn))
	{
        return 1;
	} else { return 0; }
}

/****************************************************************************
Pick Folder dialog
*****************************************************************************/
int BrowseForFolder(HWND hwnd, char* filename) {
    BROWSEINFO bInfo; ZeroMemory(&bInfo, sizeof(bInfo));
    char buffer[MAX_PATH]; ZeroMemory(&buffer, sizeof(buffer));
    bInfo.hwndOwner = hwnd;
    bInfo.pszDisplayName = buffer;
//    bInfo.lpszTitle = "Pick Dir";
    bInfo.ulFlags = BIF_RETURNONLYFSDIRS;
    LPCITEMIDLIST pFolder = SHBrowseForFolder(&bInfo);
    if (!pFolder) { return 0; }
    if (!SHGetPathFromIDList(pFolder, filename)) { return 0; }
    strcat(filename, "\\"); //add the trailing backslash
    return 1;
}

/****************************************************************************
Set Menu State - checked/disabled/etc (fState) of a menu item
*****************************************************************************/
int SetMenuState(HMENU hMenu, UINT id, UINT state)
{
    MENUITEMINFO mnuItem; memset(&mnuItem,0,sizeof(mnuItem));
    mnuItem.cbSize = sizeof(MENUITEMINFO);
    mnuItem.fMask = MIIM_STATE;
    mnuItem.fState = state;
    return SetMenuItemInfo(hMenu, id, FALSE, &mnuItem);
}

/****************************************************************************
Set Menu Item Text
*****************************************************************************/
int SetMenuItemText(HMENU hMenu, UINT id, const char* MenuText)
{
    MENUITEMINFO mnuItem; memset(&mnuItem,0,sizeof(MENUITEMINFO));
    mnuItem.cbSize = sizeof(MENUITEMINFO);
    mnuItem.fMask = MIIM_STRING;
    mnuItem.dwTypeData = (char*)MenuText;
    return SetMenuItemInfo(hMenu, id, FALSE, &mnuItem);
}

/****************************************************************************
Set Menu Item Data
*****************************************************************************/
int SetMenuItemData(HMENU hMenu, UINT id, UINT data)
{
    MENUITEMINFO mnuItem; memset(&mnuItem,0,sizeof(MENUITEMINFO));
    mnuItem.cbSize = sizeof(MENUITEMINFO);
    mnuItem.fMask = MIIM_DATA;
    mnuItem.dwItemData = data;
    return SetMenuItemInfo(hMenu, id, FALSE, &mnuItem);
}

/****************************************************************************
Get Menu Item Data
*****************************************************************************/
int GetMenuItemData(HMENU hMenu, UINT id)
{
    MENUITEMINFO mnuItem; memset(&mnuItem,0,sizeof(MENUITEMINFO));
    mnuItem.cbSize = sizeof(MENUITEMINFO);
    mnuItem.fMask = MIIM_DATA;
    GetMenuItemInfo(hMenu, id, FALSE, &mnuItem);
    return mnuItem.dwItemData;
}
