/****************************************************************************
Memory Editor tab procedure(s) and handlers

Everything for the memory editor display should pretty much be here.
*****************************************************************************/
#include "ps2cc.h"
#include "ps2cc_gui.h"

u32 CurrMemAddress = 0x00100000;

BOOL CALLBACK MemoryEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndMemData = GetDlgItem(hwnd, MEM_EDIT_LSV);
	HWND hwndMemEdit = GetDlgItem(hwnd, MEM_EDIT_TXT);
	switch(message)
	{
		case WM_INITDIALOG:
        {
//            SendMessage(hwndMemData,LVM_DELETEALLITEMS,0,0);
            SendMessage(hwndMemData,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndMemData, GWLP_WNDPROC), MEM_EDIT_LSV);
		    SetWindowLongPtr (hwndMemData, GWLP_WNDPROC, (LONG_PTR)MemDataHandler);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndMemEdit, GWLP_WNDPROC), MEM_EDIT_TXT);
		    SetWindowLongPtr (hwndMemEdit, GWLP_WNDPROC, (LONG_PTR)MemEditHandler);
		    ListViewAddCol(hwndMemData, "Address", 0, 0x80);
		    SendMessage(hwnd, WM_COMMAND, Settings.MemEdit.EditSizeId, 0);
		} break;
		case WM_COMMAND:
        {
			switch(LOWORD(wParam))
            {
				/****************************************************************************
				View Menu
				*****************************************************************************/
                case MNU_MEM_SHOW_BYTES: case MNU_MEM_SHOW_SHORTS: case MNU_MEM_SHOW_WORDS:
                {
					SendMessage(hwndMemData,LVM_DELETEALLITEMS,0,0);
					int i = 1;
					while (i = SendMessage(hwndMemData, LVM_DELETECOLUMN, i, 0)) { i++; }
					i = 1;
					while (i = SendMessage(hwndMemData, LVM_DELETECOLUMN, i, 0)) { i++; }
					char txtCol[4];
					int col = 1;
					for (i = 0; i < 0x10; i += Settings.MemEdit.EditSize)
					{
						sprintf(txtCol, "%X", i);
						ListViewAddCol(hwndMemData, txtCol, col, 0x40);
						col++;
					}
					ListViewAddCol(hwndMemData, "ASCII", col, 0x80);
					SendMessage(hwndMemData, LVM_SETCOLUMNWIDTH, col, LVSCW_AUTOSIZE_USEHEADER);
					if (DlgInfo.ActiveTab == MEMORY_EDITOR_TAB) { CurrMemAddress = ShowMem(CurrMemAddress); }
				} break;
				/****************************************************************************
				Begin Edit
				*****************************************************************************/
				case LSV_MEM_EDIT_BEGINEDIT:
				{
			        if (DlgInfo.lvEdit[LV_MEM_EDIT].Status) { MessageBox(NULL,"Already editing. WTF? (LSV_MEM_EDIT_BEGINEDIT)","Error",MB_OK); break; }
			        DlgInfo.lvEdit[LV_MEM_EDIT].iItem = LOWORD(lParam);
			        DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem = HIWORD(lParam);
		            if (DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem > 0) { SendMessage(hwndMemEdit, EM_SETLIMITTEXT, Settings.MemEdit.EditSize*2, 0); }
		            else { SendMessage(hwndMemEdit, EM_SETLIMITTEXT, 8, 0); }
					char txtInput[32];
					ListViewGetText(hwndMemData, DlgInfo.lvEdit[LV_MEM_EDIT].iItem, DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem, txtInput, sizeof(txtInput));
					SetWindowText(hwndMemEdit,txtInput);
					RECT lvEditRect; memset(&lvEditRect,0,sizeof(RECT));
					lvEditRect.top = DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem;
					lvEditRect.left = LVIR_LABEL;
					SendMessage(hwndMemData, LVM_GETSUBITEMRECT, DlgInfo.lvEdit[LV_MEM_EDIT].iItem, (LPARAM)&lvEditRect);
					WINDOWPLACEMENT lvPlace; memset(&lvPlace,0,sizeof(WINDOWPLACEMENT));
					lvPlace.length = sizeof(WINDOWPLACEMENT);
					GetWindowPlacement(hwndMemData, &lvPlace);
					POINT lvPos;
					lvPos.x = lvPlace.rcNormalPosition.left;
					lvPos.y = lvPlace.rcNormalPosition.top;
					SetWindowPos(hwndMemEdit,HWND_TOP,lvPos.x+lvEditRect.left+3,lvPos.y+lvEditRect.top+1,(lvEditRect.right-lvEditRect.left),(lvEditRect.bottom-lvEditRect.top)+1,SWP_SHOWWINDOW);
					SetFocus(hwndMemEdit);
					SendMessage(hwndMemEdit, EM_SETSEL, 0, -1);
					DlgInfo.lvEdit[LV_MEM_EDIT].Status = 1;
				} break;
				/****************************************************************************
				End Edit
				*****************************************************************************/
				case LSV_MEM_EDIT_ENDEDIT:
				{
					if ((!DlgInfo.lvEdit[LV_MEM_EDIT].Status) || (!lParam)) {
						DlgInfo.lvEdit[LV_MEM_EDIT].Status = 0;
       			        SetWindowPos(hwndMemEdit,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
       			        SetFocus(hwndMemData); break;
       			    }
       			    char txtValue[32], fmtString[17];
    				if (Settings.MemEdit.EditSize <= 4) { sprintf(fmtString, "%%0%uX", Settings.MemEdit.EditSize*2); }
    				else { strcpy(fmtString, "%16I64X"); }
					u64 value = GetHexWindow(hwndMemEdit);
					if (DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem == 0) {
						CurrMemAddress = ShowMem(value);
					}
					else {
						sprintf(txtValue, fmtString, value);
						ListViewSetRow(hwndMemData, DlgInfo.lvEdit[LV_MEM_EDIT].iItem, DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem, 1, txtValue);
					}
            		DlgInfo.lvEdit[LV_MEM_EDIT].Status = 0;
            		SetWindowPos(hwndMemEdit,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
            		SetFocus(hwndMemData);
				} break;
			}
		} break;
        case WM_SHOWWINDOW:
        {
            if (wParam) { CurrMemAddress = ShowMem(CurrMemAddress); }
        } break;
	}
	return FALSE;
}

/****************************************************************************
Memory Listview handler
*****************************************************************************/
LRESULT CALLBACK MemDataHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
    switch (message)
    {
        case WM_LBUTTONDOWN:
        {
            if (DlgInfo.lvEdit[LV_MEM_EDIT].Status) { SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 0); }
            int iSelected = ListViewHitTst(hwnd, GetMessagePos(), -1);
            if (iSelected < 0) { break; }
            int iSubItem = ListViewHitTst(hwnd, GetMessagePos(), iSelected);
            if (iSubItem > (0x10 / Settings.MemEdit.EditSize)) { break; }
            SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_BEGINEDIT, MAKELPARAM(iSelected, iSubItem));
        } return 0;
        case WM_NOTIFY:
        {
                if (((NMHDR*)lParam)->code == HDN_BEGINTRACKW) { return TRUE; }
                if (((NMHDR*)lParam)->code == HDN_BEGINTRACKA) { return TRUE; }
        } break;
	}
	if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
	else { return DefWindowProc (hwnd, message, wParam, lParam); }
}

/****************************************************************************
Memory edit box handler
*****************************************************************************/
LRESULT CALLBACK MemEditHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
	HWND hwndMemData = GetDlgItem(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], MEM_EDIT_LSV);
    switch (message)
    {
        case WM_LBUTTONDOWN: { SendMessage(hwnd, EM_SETSEL, 0, -1); return 0; }
        case WM_CHAR:
        {
        	if ((wParam == VK_BACK) || (wParam == 24) || (wParam == 3) || (wParam == 22) || (wParam == VK_TAB)) { break; } //cut/copy/paste/backspace
            if (wParam == 1) { SendMessage(hwnd, EM_SETSEL, 0, -1); } //select all
			wParam = FilterHexChar(wParam);
		} break;
        case WM_PASTE:
        {
			if (DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem > 0) { return 0; }
            char txtInput[20], txtInput2[20];
            GetWindowText(hwnd, txtInput, sizeof(txtInput));
            if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
            GetWindowText(hwnd, txtInput2, sizeof(txtInput2));
            if ((!isHexWindow(hwnd)) || (strlen(txtInput2) > SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0))) { SetWindowText(hwnd, txtInput); }
		} return 0;
        case WM_KEYUP:
        {
			switch (wParam)
			{
            	case VK_RETURN: {
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 1);
					int iItem = DlgInfo.lvEdit[LV_MEM_EDIT].iItem;
					int iSubItem = DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem;
					if (iSubItem == (0x10 / Settings.MemEdit.EditSize)) {
						if (!ListViewGetHex(hwndMemData, iItem + 1, 0)) { CurrMemAddress = ShowMem(CurrMemAddress + 0x10); }
						else {iItem++; }
						iSubItem = 1;
					} else { iSubItem++; }
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_BEGINEDIT, MAKELPARAM(iItem, iSubItem));
				} break;
            	case VK_DOWN:
            	{
					int iItem = DlgInfo.lvEdit[LV_MEM_EDIT].iItem;
					if (!ListViewGetHex(hwndMemData, iItem + 1, 0)) { CurrMemAddress = ShowMem(CurrMemAddress + 0x10); }
					else { iItem++; }
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 0);
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_BEGINEDIT, MAKELPARAM(iItem, DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem));
				} break;
            	case VK_UP:
            	{
					int iItem = DlgInfo.lvEdit[LV_MEM_EDIT].iItem;
					if (iItem == 0) { CurrMemAddress = ShowMem(CurrMemAddress - 0x10); }
					else { iItem--; }
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 0);
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_BEGINEDIT, MAKELPARAM(iItem, DlgInfo.lvEdit[LV_MEM_EDIT].iSubItem));
				} break;
			}
        } break;
        case WM_KILLFOCUS:
        {
			SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 0);
		} break;
	}
	if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
	else { return DefWindowProc (hwnd, message, wParam, lParam); }
}

/****************************************************************************
ShowMem - Gwt a block of memory and fill the listview
*****************************************************************************/
u32 ShowMem(u32 address)
{
	HWND hwndMemData = GetDlgItem(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], MEM_EDIT_LSV);
    int PageSize = SendMessage(hwndMemData,LVM_GETCOUNTPERPAGE,0,0);
    char txtAddress[9], txtValue[9], txtASCII[17], fmtString[8], tmpString[17];
    int iRow, iCol, offset, BlockSize = (PageSize * 0x10) ;
    u64 value;
    address &= 0xFFFFFFF0;
    u8 *MemBlock;
    //allocate array for memory block
    if (!(MemBlock = (unsigned char*)malloc(BlockSize))) {
		sprintf(ErrTxt, "Unable to allocate buffer memory (ShowMem) -- Error %u", GetLastError());
        MessageBox(NULL, ErrTxt, "Error", MB_OK); goto showmem_error;
    }
    //read memory
    if (!ReadMem(MemBlock, address, address + BlockSize)) { goto showmem_error; }
    SendMessage(hwndMemData,LVM_DELETEALLITEMS,0,0);
    //determine format string for 00 padding values
    if (Settings.MemEdit.EditSize <= 4) { sprintf(fmtString, "%%0%uX", Settings.MemEdit.EditSize*2); }
    else { strcpy(fmtString, "%16I64X"); }
    //loop - outer loop by row, inner loop to fill columns
//    sprintf(ErrTxt, "%i",Settings.MemEdit.EditSize);
//    MessageBox(NULL,ErrTxt, "Debug", MB_OK);
    for (iRow = 0; iRow <= PageSize; iRow++)
    {
		offset = iRow * 0x10;
		strcpy(txtASCII, "");
		if ((offset + 0x10) > BlockSize) { break; }
		sprintf(txtAddress, "%08X", address + offset);
        ListViewAddRow(hwndMemData, 1, txtAddress);
        SendMessage(hwndMemData, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
		for (iCol = 0; iCol < 0x10; iCol += Settings.MemEdit.EditSize)
		{
			switch (Settings.MemEdit.EditSize)
			{
				case 1: { value = MemBlock[offset + iCol]; } break;
				case 2: { value = *((unsigned short *)&MemBlock[offset + iCol]); } break;
				case 4: { value = *((unsigned int *)&MemBlock[offset + iCol]); } break;
			}
			//byteswap here
			sprintf(txtValue, fmtString, value);
			Hex2ASCII(value, Settings.MemEdit.EditSize, tmpString);
			strcat(txtASCII, tmpString);
			ListViewSetRow(hwndMemData, iRow, (iCol / Settings.MemEdit.EditSize) + 1, 1, txtValue);
        	SendMessage(hwndMemData, LVM_SETCOLUMNWIDTH, (iCol / Settings.MemEdit.EditSize) + 1, LVSCW_AUTOSIZE);
		}
		ListViewSetRow(hwndMemData, iRow, (iCol / Settings.MemEdit.EditSize) + 1, 1, txtASCII);
        SendMessage(hwndMemData, LVM_SETCOLUMNWIDTH, (iCol / Settings.MemEdit.EditSize) + 1, LVSCW_AUTOSIZE_USEHEADER);
	}
showmem_error:
    if(MemBlock) free(MemBlock);
    return address;
}
