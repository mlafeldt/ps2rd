/****************************************************************************
Results tab procedure(s) and handlers

Everything for the results display should pretty much be here.
*****************************************************************************/
#include "ps2cc.h"
#include "ps2cc_gui.h"


s64 CurrResNum;
u32 *ResultsList;


BOOL CALLBACK SearchResultsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndResList = GetDlgItem(hwnd, RESULTS_LSV);
    HWND hwndResPage = GetDlgItem(hwnd, RESULTS_PAGE_CMB);
    HWND hwndActiveResAddr = GetDlgItem(hwnd, ACTIVE_RES_ADDR_TXT);
    HWND hwndActiveResSize = GetDlgItem(hwnd, ACTIVE_RES_SIZE_CMB);
    HWND hwndActiveResValue = GetDlgItem(hwnd, ACTIVE_RES_VALUE_TXT);
    HWND hwndActiveList = GetDlgItem(hwnd, ACTIVE_CODES_LSV);
    HWND hwndActiveResEdit = GetDlgItem(hwnd, ACTIVE_RES_EDIT_TXT);
    HWND hwndUseSearchNum = GetDlgItem(hwnd, RESULTS_SEARCH_NUM_CMB);
	switch(message)
	{
		case WM_INITDIALOG:
        {
            SendMessage(hwndResList,LVM_DELETEALLITEMS,0,0);
            SendMessage(hwndResList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_LABELTIP);
//            SendMessage(hwndResList, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            ListViewAddCol(hwndResList, "Address", 0, 0x80);            //subclassing
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndActiveResAddr, GWLP_WNDPROC), ACTIVE_RES_ADDR_TXT);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndActiveResValue, GWLP_WNDPROC), ACTIVE_RES_VALUE_TXT);
		    SetWindowLongPtr (hwndActiveResAddr, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);
		    SetWindowLongPtr (hwndActiveResValue, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndResList, GWLP_WNDPROC), RESULTS_LSV);
		    SetWindowLongPtr (hwndResList, GWLP_WNDPROC, (LONG_PTR)ResultsListHandler);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndActiveList, GWLP_WNDPROC), ACTIVE_CODES_LSV);
		    SetWindowLongPtr (hwndActiveList, GWLP_WNDPROC, (LONG_PTR)ActiveListHandler);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndActiveResEdit, GWLP_WNDPROC), ACTIVE_RES_EDIT_TXT);
		    SetWindowLongPtr (hwndActiveResEdit, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);
            //Active Code Sizes
            SendMessage(hwndActiveResSize,CB_RESETCONTENT,0,0);
            ComboAddItem(hwndActiveResSize, "8-Bit" , 1);
            ComboAddItem(hwndActiveResSize, "16-Bit" , 2);
            ComboAddItem(hwndActiveResSize, "32-Bit" , 4);
//            ComboAddItem(hwndActiveResSize, "64-Bit" , 8);
            //Init Active Codes List
            SendMessage(hwndActiveList,LVM_DELETEALLITEMS,0,0);
            SendMessage(hwndActiveList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_GRIDLINES|LVS_EX_LABELTIP);
//            SendMessage(hwndAciveList, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            ListViewAddCol(hwndActiveList, "Address", 0, 0x70);
            ListViewAddCol(hwndActiveList, "Value", 1, 0x80);
            ListViewAddCol(hwndActiveList, "Size", 2, 0x30);
            SendMessage(hwndActiveList, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER);

		    SendMessage(hwndActiveResAddr, EM_SETLIMITTEXT, 8, 0);
		} break;
		case WM_COMMAND:
        {
			switch(LOWORD(wParam))
            {
				/************************************************************
				Result Page # Combo Box
				*************************************************************/
                case RESULTS_PAGE_CMB:
                {
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
							u32 PageSize = Settings.Results.PageSize ? Settings.Results.PageSize : SendMessage(hwndResList,LVM_GETCOUNTPERPAGE,0,0);
							CurrResNum = ShowResPage(PageSize * SendMessage(hwndResPage,CB_GETITEMDATA,SendMessage(hwndResPage,CB_GETCURSEL,0,0),0));
						} break;
					}
				} break;
				/************************************************************
				Active Code Size Combo Box
				*************************************************************/
                case ACTIVE_RES_SIZE_CMB:
                {
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
                            int ResSize = SendMessage(hwndActiveResSize,CB_GETITEMDATA,SendMessage(hwndActiveResSize,CB_GETCURSEL,0,0),0);
                            SendMessage(hwndActiveResValue, EM_SETLIMITTEXT, ResSize*2, 0);
                        } break;
                    }
                } break;
				/************************************************************
				Write Once button
				*************************************************************/
                case RES_WRITE_ONCE_CMD:
                {
                    unsigned char actcodes[2052];
                    u32 address = GetHexWindow(hwndActiveResAddr);
                    int ResSize = SendMessage(hwndActiveResSize,CB_GETITEMDATA,SendMessage(hwndActiveResSize,CB_GETCURSEL,0,0),0);
    				*((unsigned int *)&actcodes[0]) = 1;
    				*((unsigned int *)&actcodes[4]) = address | ((ResSize / 2) << 28);
    				*((unsigned int *)&actcodes[8]) = (u32)GetHexWindow(hwndActiveResValue);
                    if (address % ResSize) { MessageBox(NULL, "Address must be aligned to match the number of bytes being written, fucknut.", "Error", MB_OK); break; }
    				if (!ActivateCheats(actcodes, 1)) {
    					MessageBox(NULL, ErrTxt, "Error", MB_OK); break;
    				}
                    UpdateActiveCheats();
                } break;
				/************************************************************
				Activate button
				*************************************************************/
				case RES_ACTIVATE_CMD:
                {
                    u32 address = GetHexWindow(hwndActiveResAddr);
                    u64 value = GetHexWindow(hwndActiveResValue);
                    int ResSize = SendMessage(hwndActiveResSize,CB_GETITEMDATA,SendMessage(hwndActiveResSize,CB_GETCURSEL,0,0),0);
                    if (address % ResSize) { MessageBox(NULL, "Address must be aligned to match the number of bytes being written, fucknut.", "Error", MB_OK); break; }
                    ListView_SetCheckState(hwndActiveList, Result2ActiveList(address, value, ResSize), TRUE);
                    UpdateActiveCheats();
                } break;
				/************************************************************
				Delete button
				*************************************************************/
				case RES_DEL_ACTIVE_CMD:
				{
                    int iCount = SendMessage(hwndActiveList, LVM_GETITEMCOUNT, 0, 0);
                    int i;
                    for (i = 0; i < iCount; i++) {
                        if (SendMessage(hwndActiveList, LVM_GETITEMSTATE, i, LVIS_SELECTED)) {
                            SendMessage(hwndActiveList, LVM_DELETEITEM, i, 0);
                        }
                    }
                    UpdateActiveCheats();
				} break;
				/************************************************************
				ALL On/Off buttons
				*************************************************************/
				case RES_ALL_ON_CMD: case RES_ALL_OFF_CMD:
				{
                    int iCount = SendMessage(hwndActiveList, LVM_GETITEMCOUNT, 0, 0);
                    int i;
                    int iState = (LOWORD(wParam) == RES_ALL_ON_CMD) ? TRUE : FALSE;
                    for (i = 0; i < iCount; i++) {
						ListView_SetCheckState(hwndActiveList, i, iState);
					}
					UpdateActiveCheats();
				} break;
				/************************************************************
				Clear ALL button
				*************************************************************/
				case RES_CLEAR_ALL_CMD:
				{
					SendMessage(hwndActiveList,LVM_DELETEALLITEMS,0,0);
					DeActivateCheats();
				} break;
				/************************************************************
				Page Up/Down Key
				*************************************************************/
				case MNU_RES_PAGE_DOWN: case MNU_RES_PAGE_UP:
				{
					int PageNum = SendMessage(hwndResPage,CB_GETITEMDATA,SendMessage(hwndResPage,CB_GETCURSEL,0,0),0);
					PageNum += (LOWORD(wParam) == MNU_RES_PAGE_DOWN) ? 1 : -1;
					ComboSelFromData(hwndResPage, PageNum);
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(RESULTS_PAGE_CMB, CBN_SELCHANGE),(LPARAM)hwndResPage);
				} break;
				/************************************************************
				Begin Editing Active Cheat
				*************************************************************/
				case LSV_ACTIVE_BEGINEDIT:
				{
			        if (DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status) { MessageBox(NULL,"Already editing. WTF? (LSV_ACTIVE_BEGINEDIT)","Error",0); break; }
			        DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iItem = LOWORD(lParam);
			        DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iSubItem = HIWORD(lParam);
		            int iSize = ListViewGetDec(hwndActiveList, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iItem, 2);
		            SendMessage(hwndActiveResEdit, EM_SETLIMITTEXT, iSize*2, 0);
					char txtInput[32];
					ListViewGetText(hwndActiveList, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iItem, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iSubItem, txtInput, sizeof(txtInput));
					SetWindowText(hwndActiveResEdit,txtInput);
					RECT lvEditRect; memset(&lvEditRect,0,sizeof(RECT));
					lvEditRect.top = DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iSubItem;
					lvEditRect.left = LVIR_LABEL;
					SendMessage(hwndActiveList, LVM_GETSUBITEMRECT, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iItem, (LPARAM)&lvEditRect);
					WINDOWPLACEMENT lvPlace; memset(&lvPlace,0,sizeof(WINDOWPLACEMENT));
					lvPlace.length = sizeof(WINDOWPLACEMENT);
					GetWindowPlacement(hwndActiveList, &lvPlace);
					POINT lvPos;
					lvPos.x = lvPlace.rcNormalPosition.left;
					lvPos.y = lvPlace.rcNormalPosition.top;
					SetWindowPos(hwndActiveResEdit,HWND_TOP,lvPos.x+lvEditRect.left+3,lvPos.y+lvEditRect.top+1,(lvEditRect.right-lvEditRect.left),(lvEditRect.bottom-lvEditRect.top)+1,SWP_SHOWWINDOW);
					SetFocus(hwndActiveResEdit);
					SendMessage(hwndActiveResEdit, EM_SETSEL, 0, -1);
					DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status = 1;
				} break;
				/************************************************************
				End Editing Active Cheat
				*************************************************************/
       			case LSV_ACTIVE_ENDEDIT:
       			{
					if ((!DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status) || (!lParam)) {
						DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status = 0;
       			        SetWindowPos(hwndActiveResEdit,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
       			        SetFocus(hwndActiveList); break;
       			    }
       			    char txtValue[32];
					if (isHexWindow(hwndActiveResEdit)) { GetWindowText(hwndActiveResEdit, txtValue, sizeof(txtValue)); }
					else { strcpy(txtValue, "0"); }
					ListViewSetRow(hwndActiveList, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iItem, DlgInfo.lvEdit[LV_ACTIVE_CHEATS].iSubItem, 1, txtValue);
            		DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status = 0;
            		SetWindowPos(hwndActiveResEdit,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
            		SetFocus(hwndActiveList);
            		UpdateActiveCheats();
				} break;
			}
		} break;
        case WM_SHOWWINDOW:
        {
            if (wParam) {
				LoadResultsList();
				//ResSize combo
				HWND hwndSearchSize = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIZE_CMB);
				int SearchSize = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
				if (SearchSize > 4) { SearchSize = 4; }
				ComboSelFromData(hwndActiveResSize, SearchSize);
//          	  SendMessage(hwndActiveResSize,CB_SETCURSEL,0,0);
            	SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ACTIVE_RES_SIZE_CMB, CBN_SELCHANGE),(LPARAM)hwndActiveResSize);
			}
        } break;
	}
	return FALSE;
}

/****************************************************************************
Results Listview Handler
*****************************************************************************/
LRESULT CALLBACK ResultsListHandler (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
    switch (message)
    {
        case WM_LBUTTONDBLCLK: case WM_LBUTTONDOWN:
        {
            if (!(ResultsList)) { break; }
            int iSelected = ListViewHitTst(hwnd, GetMessagePos(), -1);
            if (iSelected < 0) { break; }
            int iSubItem = ListViewHitTst(hwnd, GetMessagePos(), iSelected);
            if (iSubItem <= 0) { iSubItem = 1; }
            u32 address = ListViewGetHex(hwnd, iSelected, 0);
			HWND hwndActiveResValue = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_VALUE_TXT);
			HWND hwndSearchSize = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIZE_CMB);
			int SearchSize = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
			if (SearchSize > 4) { SearchSize = 4; }
            ComboSelFromData(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_SIZE_CMB), SearchSize);
            SetHexWindow(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_ADDR_TXT), address);
            SendMessage(hwndActiveResValue, EM_SETLIMITTEXT, SearchSize*2, 0);
			u64 value = GetResListValue(hwnd, iSelected, iSubItem, SearchSize);
			SetHexWindow(hwndActiveResValue, value);
//            extern CurrMemAddress;
//            if (Settings.Results.RamView == 1) { CurrMemAddress = ShowRAM(address & 0xFFFFFFF0); }
            if (message == WM_LBUTTONDBLCLK) {
				iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				while (iSelected >= 0)
				{
					address = ListViewGetHex(hwnd, iSelected, 0);
					value = GetResListValue(hwnd, iSelected, iSubItem, SearchSize);
                	ListView_SetCheckState(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_CODES_LSV), Result2ActiveList(address, value, SearchSize), TRUE);
					iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);
				}
            }
        } break;
        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_NEXT: case VK_PRIOR:
                {
                    if (!(ResultsList)) { break; }
                    HWND hwndResPage = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_PAGE_CMB);
                    int iSelected = SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0);
                    if (wParam == VK_NEXT) {
                    	SendMessage(hwndResPage,CB_SETCURSEL, SendMessage(hwndResPage,CB_GETCURSEL,0,0) + 1, 0);
            			SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, MAKEWPARAM(RESULTS_PAGE_CMB, CBN_SELCHANGE),(LPARAM)hwndResPage);
					}
                    else {
                    	SendMessage(hwndResPage,CB_SETCURSEL, SendMessage(hwndResPage,CB_GETCURSEL,0,0) - 1, 0);
            			SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, MAKEWPARAM(RESULTS_PAGE_CMB, CBN_SELCHANGE),(LPARAM)hwndResPage);
                    }
                    ListView_SetItemState(hwnd, iSelected, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                } return 0;
                case VK_LEFT: case VK_RIGHT:
                {
					HWND hwndUseSearchNum = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_SEARCH_NUM_CMB);
					int sNum = SendMessage(hwndUseSearchNum,CB_GETCURSEL,0,0);
					int sMax = SendMessage(hwndUseSearchNum,CB_GETCOUNT,0,0);
					sNum += (wParam == VK_LEFT) ? -1 : 1;
					if ((sNum < 0) || (sNum >= sMax)) { return 0; }
					SendMessage(hwndUseSearchNum,CB_SETCURSEL,sNum,0);
					//update value/address boxes in active cheats window
					int iSelected = SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0);
					int iSubItem = SendMessage(hwndUseSearchNum,CB_GETCURSEL,0,0) + 1;
					u32 address = ListViewGetHex(hwnd, iSelected, 0);
					HWND hwndSearchSize = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIZE_CMB);
					int SearchSize = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
					if (SearchSize > 4) { SearchSize = 4; }
					u64 value = GetResListValue(hwnd, iSelected, iSubItem, SearchSize);
					HWND hwndActiveResValue = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_VALUE_TXT);
            		SendMessage(hwndActiveResValue, EM_SETLIMITTEXT, SearchSize*2, 0);
					SetHexWindow(hwndActiveResValue, value);
            		SetHexWindow(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_ADDR_TXT), address);
            		ComboSelFromData(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_SIZE_CMB), SearchSize);
				} return 0;
				case VK_UP: case VK_DOWN:
				{
					int iSelected = SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0);
					HWND hwndUseSearchNum = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_SEARCH_NUM_CMB);
					int iSubItem = SendMessage(hwndUseSearchNum,CB_GETCURSEL,0,0) + 1;
					u32 address = ListViewGetHex(hwnd, iSelected, 0);
					HWND hwndSearchSize = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIZE_CMB);
					int SearchSize = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
					if (SearchSize > 4) { SearchSize = 4; }
					u64 value = GetResListValue(hwnd, iSelected, iSubItem, SearchSize);
					HWND hwndActiveResValue = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_VALUE_TXT);
            		SendMessage(hwndActiveResValue, EM_SETLIMITTEXT, SearchSize*2, 0);
					SetHexWindow(hwndActiveResValue, value);
            		SetHexWindow(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_ADDR_TXT), address);
            		ComboSelFromData(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_RES_SIZE_CMB), SearchSize);
				} break;
				case VK_RETURN:
				{
					HWND hwndUseSearchNum = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_SEARCH_NUM_CMB);
					int iSubItem = SendMessage(hwndUseSearchNum,CB_GETCURSEL,0,0) + 1;
					HWND hwndSearchSize = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIZE_CMB);
					int SearchSize = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
					if (SearchSize > 4) { SearchSize = 4; }
					u32 address;
					u64 value;
					int iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
					while (iSelected >= 0)
					{
						address = ListViewGetHex(hwnd, iSelected, 0);
						value = GetResListValue(hwnd, iSelected, iSubItem, SearchSize);
                		ListView_SetCheckState(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_CODES_LSV), Result2ActiveList(address, value, SearchSize), TRUE);
						iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);
					}

				} return 0;
				case VK_DELETE:
				{
					if (!(ResultsList)) { break; }
		            u32 address;
    				HWND hwndCompareTo = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], COMPARE_TO_CMB);
    				int SearchCount = SendMessage(hwndCompareTo,CB_GETCOUNT,0,0) - 1;
    				char resFileName[MAX_PATH];
    				sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, SearchCount);
    				if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { FreeRamInfo(); return 0; }
    				if (!(LoadFile(&RamInfo.Results, resFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { FreeRamInfo(); return 0; }

					int iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
					while (iSelected >= 0)
					{
						address = ListViewGetHex(hwnd, iSelected, 0);
						SetBitFlag(RamInfo.Results, (address - RamInfo.NewResultsInfo.MapMemAddy)/RamInfo.NewResultsInfo.SearchSize, 0);
						RamInfo.NewResultsInfo.ResCount--;
						iSelected = SendMessage(hwnd, LVM_GETNEXTITEM, iSelected, LVNI_SELECTED);
					}
                    SaveFile(RamInfo.Results, (RamInfo.NewResultsInfo.DumpSize/RamInfo.NewResultsInfo.SearchSize/8), resFileName, sizeof(CODE_SEARCH_RESULTS_INFO), &RamInfo.NewResultsInfo);
                    HWND hwndResPage = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_PAGE_CMB);
                    int ResPage = SendMessage(hwndResPage,CB_GETCURSEL,0,0);
                    LoadResultsList();
                    SendMessage(hwndResPage,CB_SETCURSEL,ResPage,0);
		            SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, MAKEWPARAM(RESULTS_PAGE_CMB, CBN_SELCHANGE),(LPARAM)hwndResPage);

				} return 0;
            }
        } break;
    }
   if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
   else { return DefWindowProc (hwnd, message, wParam, lParam); }
}

/****************************************************************************
Active List Handler
*****************************************************************************/
LRESULT CALLBACK ActiveListHandler (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
    switch (message)
    {
        case WM_VSCROLL: case WM_MOUSEWHEEL:
        {
            if (DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_ENDEDIT, 0); }
        } break;
		case WM_LBUTTONUP:
		{
			UpdateActiveCheats();
		} break;
        case WM_LBUTTONDBLCLK:
        {
            if (DlgInfo.lvEdit[LV_ACTIVE_CHEATS].Status) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_ENDEDIT, 0); }
            int iSelected = SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0);
            if (iSelected < 0) { break; }
            int iSubItem = ListViewHitTst(hwnd, GetMessagePos(), iSelected);
            if (iSubItem > 1) { break; }
            SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_BEGINEDIT, MAKELPARAM(iSelected, iSubItem));
        } return 0;
		case WM_KEYUP:
		{
			if (wParam == VK_DELETE) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, RES_DEL_ACTIVE_CMD, 0); }
			if (wParam == VK_SPACE) { UpdateActiveCheats(); }
		} break;
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
Load results
*****************************************************************************/
int LoadResultsList()
{
    CurrResNum = 0;
    FreeRamInfo();
    u32 i, DumpAddy, DumpNum;
    char txtValue[32];
    HWND hwndResList = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_LSV);
    HWND hwndCompareTo = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], COMPARE_TO_CMB);
    HWND hwndResPage = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_PAGE_CMB);
    HWND hwndUseSearchNum = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_SEARCH_NUM_CMB);
    int SearchCount = SendMessage(hwndCompareTo,CB_GETCOUNT,0,0) - 1;
    u32 PageSize = Settings.Results.PageSize ? Settings.Results.PageSize : SendMessage(hwndResList,LVM_GETCOUNTPERPAGE,0,0);
    if (!SearchCount) { return 0; }
	//reset columns and pages
	SendMessage(hwndResList,LVM_DELETEALLITEMS,0,0);
	SendMessage(hwndResPage,CB_RESETCONTENT,0,0);
	SendMessage(hwndUseSearchNum,CB_RESETCONTENT,0,0);
	ComboAddItem(hwndResPage, "1" , 0);
	i = 1;
    while (SendMessage(hwndResList,LVM_DELETECOLUMN,i,0)) { i++; }
	//load current results info
    char resFileName[MAX_PATH];
    sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, SearchCount);
    if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { goto LoadResError; }
    if (!(LoadFile(&RamInfo.Results, resFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { goto LoadResError; }
	//Set columns for values
    for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
	    sprintf(txtValue, "%u", SearchCount - DumpNum);
	    ListViewAddCol(hwndResList, txtValue, DumpNum + 1, 0x80);
	    ComboAddItem(hwndUseSearchNum, txtValue, SearchCount - DumpNum);
    }
	//allocate results list memory;
    if (ResultsList) { free(ResultsList); ResultsList = NULL; }
    if (!(ResultsList = (u32*)malloc(sizeof(u32) * (RamInfo.NewResultsInfo.ResCount+1)))) {
        sprintf(ErrTxt, "Unable to allocate results list memory (LoadResultsList) -- Error %u", GetLastError());
        MessageBox(NULL, ErrTxt, "Error", MB_OK);
        goto LoadResError;
    }

	//get the file address of every result
	i = 0; DumpAddy = 0;
    while ((DumpAddy < RamInfo.NewResultsInfo.DumpSize) && (i < RamInfo.NewResultsInfo.ResCount)) {
        if (!(GetBitFlag(RamInfo.Results, DumpAddy/RamInfo.NewResultsInfo.SearchSize))) { DumpAddy += RamInfo.NewResultsInfo.SearchSize; continue; }
        ResultsList[i] = DumpAddy;
        DumpAddy += RamInfo.NewResultsInfo.SearchSize;
        i++;
        if (((i % PageSize) == 0) && ((i/ PageSize) < Settings.Results.MaxResPages)) {
			sprintf(txtValue, "%u", (i / PageSize) + 1);
			ComboAddItem(hwndResPage, txtValue , i / PageSize);
		}
    }
	SendMessage(hwndResPage,CB_SETCURSEL,0,0);
    SendMessage(hwndUseSearchNum,CB_SETCURSEL,0,0);

	//cleanup
	FreeRamInfo();
    CurrResNum = ShowResPage(CurrResNum);
	return 1;
LoadResError:
    if (ResultsList) { free(ResultsList); ResultsList = NULL; }
	FreeRamInfo();
	return 0;
}
/****************************************************************************
ShowResPage - Show 1 page of results starting at Result number (ResNum)
*****************************************************************************/
s64 ShowResPage(s64 ResNum)
{
    u32 i, DumpNum;
    float tmpFloat=0;
    u32 *CastFloat=(u32*)(&tmpFloat);
    double tmpDouble=0;
    u64 *CastDouble=(u64*)&tmpDouble;
    HWND hwndResList = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_LSV);
    HWND hwndCompareTo = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], COMPARE_TO_CMB);
    u32 PageSize = Settings.Results.PageSize ? Settings.Results.PageSize : SendMessage(hwndResList,LVM_GETCOUNTPERPAGE,0,0);
    int SearchCount = SendMessage(hwndCompareTo,CB_GETCOUNT,0,0) - 1;
    FILE *ramFiles[MAX_SEARCHES];

    char resFileName[MAX_PATH];
    sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, SearchCount);
    if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { return 0; }
    if (!(LoadFile(&RamInfo.Results, resFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { return 0; }
	if (ResNum > RamInfo.NewResultsInfo.ResCount) { ResNum = RamInfo.NewResultsInfo.ResCount - PageSize; }
	if (ResNum < 0) { ResNum = 0; }

	SendMessage(hwndResList,LVM_DELETEALLITEMS,0,0);
	//open dump file handles - Notice they're being loaded in order from the current search back.
    for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
        sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, (SearchCount - DumpNum));
        if (!(LoadStruct(&RamInfo.OldResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { goto ShowResPageError; }
        ramFiles[DumpNum] = fopen(RamInfo.OldResultsInfo.dmpFileName,"rb");
	    if (!(ramFiles[DumpNum])) {
            sprintf(ErrTxt, "Unable to open dump file (ShowResPage,1) -- Error %u", GetLastError());
            MessageBox(NULL,ErrTxt,"Error",MB_OK);
            goto ShowResPageError;
	    }
    }
	fseek(ramFiles[0],0,SEEK_END);
	RamInfo.NewResultsInfo.DumpSize = ftell(ramFiles[0]);
	fseek(ramFiles[0],0,SEEK_SET);
	RamInfo.Access = SEARCH_ACCESS_FILE;
    char txtAddress[9], txtValue[32], fmtString[20];
	ResFormatString(fmtString, Settings.Results.DisplayFmt, RamInfo.NewResultsInfo.SearchSize);
	i = 0;
	u64 ResValue = 0;
	/*loop through the results list starting from ResNum, and loop through the
	dump handles to get the value from each.*/
    while (((ResNum + i) < RamInfo.NewResultsInfo.ResCount) && (i < PageSize)) {
		//make address a string and add row with address in 1st column
        sprintf(txtAddress, "%08X", ResultsList[ResNum + i] + RamInfo.NewResultsInfo.MapMemAddy);
        ListViewAddRow(hwndResList, 1, txtAddress);
        SendMessage(hwndResList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
        //loop and update the row with the value from each search
        for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
            RamInfo.NewFile = ramFiles[DumpNum];
            GetSearchValues(&ResValue, NULL, ResultsList[ResNum + i], RamInfo.NewResultsInfo.SearchSize, RamInfo.NewResultsInfo.Endian);
            //Check if we need to cast the value to floating point
            if (Settings.Results.DisplayFmt == MNU_RES_SHOW_FLOAT) {
                *CastDouble = ResValue;
                *CastFloat = ResValue & 0xFFFFFFFF;
                sprintf(txtValue, fmtString, (RamInfo.NewResultsInfo.SearchSize == 4) ? tmpFloat : tmpDouble);
            } else { sprintf(txtValue, fmtString, ResValue); }
            ListViewSetRow(hwndResList, i, DumpNum + 1, 1, txtValue);
            SendMessage(hwndResList, LVM_SETCOLUMNWIDTH, DumpNum + 1, LVSCW_AUTOSIZE);
        }
        i++;
    }

//    SetDecWindowU(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_PAGE_TXT), ((ResNum + i)/PageSize) + ((ResNum + i) % PageSize));

ShowResPageError:
	for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
		if(ramFiles[DumpNum]) { fclose(ramFiles[DumpNum]); }
	}
	FreeRamInfo();
	return ResNum;
}

/****************************************************************************
Res Format String

Pick the format string to be used in sprintf() statements that show result values.
*****************************************************************************/
int ResFormatString(char *tmpstring, int outfmt, int numbytes)
{
    switch (outfmt) {
        case MNU_RES_SHOW_HEX: case MNU_RES_EXPORT_HEX:
        {
			if (numbytes <= 4) { sprintf(tmpstring,"%%0%uX", numbytes*2); }
			else { strcpy(tmpstring, "%16I64X"); }
//				sprintf(tmpstring,"%%0%uI64X", numbytes*2); //doesn't like me
		} break;
        MessageBox(NULL, tmpstring, "Debug", 0);
        case MNU_RES_SHOW_DECU: case MNU_RES_EXPORT_DECU:
        {
            sprintf(tmpstring,"%%I64u");
        } break;
        case MNU_RES_SHOW_DECS: case MNU_RES_EXPORT_DECS:
        {
            sprintf(tmpstring,"%%I64d");
        } break;
        case MNU_RES_SHOW_FLOAT: case MNU_RES_EXPORT_FLOAT:
        {
            if (numbytes == 4) sprintf(tmpstring,"%%f");
            else sprintf(tmpstring,"%%Lf");
        } break;
    }
    return 0;
}
/****************************************************************************
Add Code to active list
*****************************************************************************/
int Result2ActiveList(u32 address, u64 value, int size)
{
    HWND hwndActList = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_CODES_LSV);
    int iCount = SendMessage(hwndActList, LVM_GETITEMCOUNT, 0, 0);
    char txtValue[32], txtSize[4], txtAddress[20];
    int i;
    sprintf(txtAddress, "%08I64X", address);
    sprintf(txtValue, "%08I64X", value);
    sprintf(txtSize, "%u", size);
    for (i = 0; i < iCount; i++) {
        if (ListViewGetHex(hwndActList, i, 0) == address) {
            ListViewSetRow(hwndActList, i, 1, 2, txtValue, txtSize);
            return i;
        }
    }
    i = ListViewAddRow(hwndActList, 3, txtAddress, txtValue, txtSize);
    SendMessage(hwndActList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
    SendMessage(hwndActList, LVM_SETCOLUMNWIDTH, 1, LVSCW_AUTOSIZE);
    return i;
}
/****************************************************************************
Update Active Cheats - Make array of cheats to send to PS2 and calls said function
*****************************************************************************/
int UpdateActiveCheats()
{
	if(!DeActivateCheats()) { MessageBox(NULL, ErrTxt, "Error", MB_OK); return 0; }
	unsigned char actcodes[2052];
    HWND hwndActList = GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], ACTIVE_CODES_LSV);
    int i = 0, iCount = SendMessage(hwndActList, LVM_GETITEMCOUNT, 0, 0);
    int aCount = 4;
    u32 address, value;
    while ((i < iCount) && (aCount < 2052))
    {
    	if (ListView_GetCheckState(hwndActList, i)) {
    		*((unsigned int *)&actcodes[aCount]) = ListViewGetHex(hwndActList, i, 0) | ((ListViewGetDec(hwndActList, i, 2) / 2) << 28);
    		*((unsigned int *)&actcodes[aCount + 4]) = ListViewGetHex(hwndActList, i, 1);
//    		actcodes[aCount + 8] = ListViewGetDec(hwndActList, i, 2) / 2;
    		aCount += 8;
    	}
    	i++;
    }
    if (aCount == 4) { //deactivate
		if(!DeActivateCheats()) { MessageBox(NULL, ErrTxt, "Error", MB_OK); return 0; }
		return 1;
	}
    *((unsigned int *)&actcodes[0]) = (aCount - 4)/8;
    if (!ActivateCheats(actcodes, (aCount - 4)/8)) {
    	MessageBox(NULL, ErrTxt, "Error", MB_OK); return 0;
    }
    return 1;
}

/****************************************************************************
GetResListValue - Get value from results listview based on location and display type
*****************************************************************************/
u64 GetResListValue(HWND hwndResList, int iItem, int iSubItem, int SearchSize)
{
	switch (Settings.Results.DisplayFmt)
	{
		case MNU_RES_SHOW_HEX:
		{
			return ListViewGetHex(hwndResList, iItem, iSubItem);
		} break;
		case MNU_RES_SHOW_DECU:
		case MNU_RES_SHOW_DECS:
		{
			return ListViewGetDec(hwndResList, iItem, iSubItem);
		} break;
		case MNU_RES_SHOW_FLOAT:
		{
			return ListViewGetFloat(hwndResList, iItem, iSubItem, SearchSize);
		} break;
	}
	return 0;
}

/**************************************************************
Export results
**************************************************************/
int ExportResults(int ExportType)
{
    if (!(ResultsList)) { return 0; }
    char ErrTxt[1000];
    u32 i, DumpNum, address;
    u64 value;
    float tmpFloat=0;
    u32 *CastFloat=(u32*)(&tmpFloat);
    double tmpDouble=0;
    u64 *CastDouble=(u64*)&tmpDouble;
    int SearchCount = SendMessage(GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], COMPARE_TO_CMB),CB_GETCOUNT,0,0) - 1;
	int SearchNum = SendMessage(GetDlgItem(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], RESULTS_SEARCH_NUM_CMB),CB_GETCURSEL,0,0);
    char resFileName[MAX_PATH], txtValue[32], fmtString[20], txtLine[4096];
    sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, SearchCount);
//    RAM_AND_RES_DATA RamInfo; memset(&RamInfo,0,sizeof(RAM_AND_RES_DATA));
    if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { goto EXPORT_RES_ERROR; }
    if (!(LoadFile(&RamInfo.Results, resFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { goto EXPORT_RES_ERROR; }

    if (!DoFileSave(NULL, resFileName)) { goto EXPORT_RES_ERROR; }
    FILE *ResTxtFile = fopen(resFileName, "wt");
    if (!(ResTxtFile)) {
        sprintf(ErrTxt, "Unable to open/create results file (ExportResults,1) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); goto EXPORT_RES_ERROR;
    }

	//open dump file handles - Notice they're being loaded in order from the current search back.
    FILE *ramFiles[MAX_SEARCHES];
    for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
        sprintf(resFileName,"%ssearch%u.bin",Settings.CS.DumpDir, (SearchCount - DumpNum));
        if (!(LoadStruct(&RamInfo.OldResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), resFileName))) { goto EXPORT_RES_ERROR; }
        if (!(ramFiles[DumpNum] = fopen(RamInfo.OldResultsInfo.dmpFileName,"rb"))) {
            sprintf(ErrTxt, "Unable to open dump file (ShowResPage,1) -- Error %u", GetLastError());
            MessageBox(NULL,ErrTxt,"Error",MB_OK);
            goto EXPORT_RES_ERROR;
	    }
    }

    ResFormatString(fmtString, Settings.Results.DisplayFmt, RamInfo.NewResultsInfo.SearchSize);
    for (i = 0; i < RamInfo.NewResultsInfo.ResCount; i++) {
        if (ExportType == MNU_RES_EXPORT_ALL) {
			sprintf(txtLine, "%08X ", ResultsList[i] + RamInfo.NewResultsInfo.MapMemAddy);
            for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
            	RamInfo.NewFile = ramFiles[DumpNum];
            	GetSearchValues(&value, NULL, ResultsList[i], RamInfo.NewResultsInfo.SearchSize, LITTLE_ENDIAN_SYS);
             	if (Settings.Results.DisplayFmt == MNU_RES_SHOW_FLOAT) {
                	*CastDouble = value;
                	*CastFloat = value & 0xFFFFFFFF;
                	sprintf(txtValue, fmtString, (RamInfo.NewResultsInfo.SearchSize == 4) ? tmpFloat : tmpDouble);
             	} else { sprintf(txtValue, fmtString, value); }
             	strcat(txtValue, " ");

 				strcat(txtLine, txtValue);
            }
        } else {
            address = ResultsList[i] + RamInfo.NewResultsInfo.MapMemAddy;
            RamInfo.NewFile = ramFiles[SearchNum]; //CHECK THIS
            GetSearchValues(&value, NULL, ResultsList[i], RamInfo.NewResultsInfo.SearchSize, LITTLE_ENDIAN_SYS);
			if (ExportType == MNU_RES_EXPORT_CHEAT) {
				switch (RamInfo.NewResultsInfo.SearchSize) {
					case 1:
					{
						sprintf(txtLine, "%08X %08X", address, value);
					} break;
					case 2:
					{
						sprintf(txtLine, "%08X %08X", address|0x10000000, value);
					} break;
					case 4:
					{
						sprintf(txtLine, "%08X %08X", address|0x20000000, value);
					} break;
					case 8:
					{
						sprintf(txtLine, "%08X %08X", address|0x2000000, WordFromU64(value,0));
						sprintf(txtLine, "%s%08X %08X", txtLine, (address|0x20000000) + 4, WordFromU64(value,1));
					} break;
				}
			} else if (Settings.Results.DisplayFmt == MNU_RES_SHOW_FLOAT) {
                *CastDouble = value;
                *CastFloat = value & 0xFFFFFFFF;
                sprintf(txtLine, "%08X ", address);
                sprintf(txtValue, fmtString, (RamInfo.NewResultsInfo.SearchSize == 4) ? tmpFloat : tmpDouble);
                strcat(txtLine, txtValue);
			} else {
                sprintf(txtLine, "%08X ", address);
                sprintf(txtValue, fmtString, value);
                strcat(txtLine, txtValue);
            }
		}
        fprintf(ResTxtFile, "%s\n", txtLine);
    }
EXPORT_RES_ERROR:
    fclose(ResTxtFile);
	for (DumpNum = 0; DumpNum < SearchCount; DumpNum++) {
		if(ramFiles[DumpNum]) { fclose(ramFiles[DumpNum]); }
	}
    FreeRamInfo();
    return i;
}
