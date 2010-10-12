/****************************************************************************
Code Search tab procedure(s) and handlers

Most (if not all) of the search tab setup and GUI handling should be found
here. This includes search preparation, like grabbing all the needed info from
the interface to start the search.
*****************************************************************************/

#include "ps2cc.h"
#include "ps2cc_gui.h"

CODE_SEARCH_VARS Search;

BOOL CALLBACK CodeSearchProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndSearchSize = GetDlgItem(hwnd, SEARCH_SIZE_CMB);
    HWND hwndCompareTo = GetDlgItem(hwnd, COMPARE_TO_CMB);
    HWND hwndSearchType = GetDlgItem(hwnd, SEARCH_TYPE_CMB);
    HWND hwndSearchArea = GetDlgItem(hwnd, SEARCH_AREA_CMB);
    HWND hwndExSearchList = GetDlgItem(hwnd, EX_SEARCH_LSV);
    HWND hwndSearchValue1 = GetDlgItem(hwnd, SEARCH_VALUE1_TXT);
    HWND hwndSearchValue2 = GetDlgItem(hwnd, SEARCH_VALUE2_TXT);
    HWND hwndSearchAreaLow = GetDlgItem(hwnd, SEARCH_AREA_LOW_TXT);
    HWND hwndSearchAreaHigh = GetDlgItem(hwnd, SEARCH_AREA_HIGH_TXT);
    HWND hwndProgress = GetDlgItem(hwnd, DUMPSTATE_PRB);
    HWND hwndSignedSearchChk = GetDlgItem(hwnd, SEARCH_SIGNED_CHK);
    HWND hwndExValueTxt = GetDlgItem(hwnd, EX_VALUE_TXT);
    HWND hwndSearchHistory = GetDlgItem(hwnd, SEARCH_HISTORY_TXT);
    HWND hwndPS2Waits = GetDlgItem(hwnd, PS2_WAITS_CHK);
    HWND hwndSearchLabel = GetDlgItem(hwnd, SEARCH_LABEL_TXT);
    HWND hwndtabCtrl = GetDlgItem(DlgInfo.Main, PS2CC_TABCTRL);
    HMENU hMenu = GetMenu(hwnd);
	switch(message)
	{
		case WM_INITDIALOG:
        {
            //Set fonts
/*
            SendMessage(hwndSearchValue1, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            SendMessage(hwndSearchValue2, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            SendMessage(hwndSearchAreaLow, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            SendMessage(hwndSearchAreaHigh, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
            SendMessage(hwndExValueTxt, WM_SETFONT, (WPARAM)Settings.ValueHFont, TRUE);
*/
			//subclassing for controls (value box handler etc)
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndSearchAreaLow, GWLP_WNDPROC), SEARCH_AREA_LOW_TXT);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndSearchAreaHigh, GWLP_WNDPROC), SEARCH_AREA_HIGH_TXT);
		    SetWindowLongPtr (hwndSearchAreaLow, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);
		    SetWindowLongPtr (hwndSearchAreaHigh, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);

			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndSearchValue1, GWLP_WNDPROC), SEARCH_VALUE1_TXT);
			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndSearchValue2, GWLP_WNDPROC), SEARCH_VALUE2_TXT);
		    SetWindowLongPtr (hwndSearchValue1, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);
		    SetWindowLongPtr (hwndSearchValue2, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);

			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndExSearchList, GWLP_WNDPROC), EX_SEARCH_LSV);
		    SetWindowLongPtr (hwndExSearchList, GWLP_WNDPROC, (LONG_PTR)ExSearchListHandler);

			SetSubclassProc((WNDPROC)GetWindowLongPtr (hwndExValueTxt, GWLP_WNDPROC), EX_VALUE_TXT);
		    SetWindowLongPtr (hwndExValueTxt, GWLP_WNDPROC, (LONG_PTR)ValueEditBoxHandler);

            //Search Sizes
            SendMessage(hwndSearchSize,CB_RESETCONTENT,0,0);
            ComboAddItem(hwndSearchSize, "8-Bit (1 Byte)" , 1);
            ComboAddItem(hwndSearchSize, "16-Bit (2 Bytes)" , 2);
            ComboAddItem(hwndSearchSize, "32-Bit (4 Bytes)" , 4);
            ComboAddItem(hwndSearchSize, "64-Bit (8 Bytes)" , 8);

            //Compare To
            SendMessage(hwndCompareTo,CB_RESETCONTENT,0,0);
            ComboAddItem(hwndCompareTo, "New Search" , 0);

			//Search Area
            SendMessage(hwndSearchArea,CB_RESETCONTENT,0,0);
            ComboAddItem(hwndSearchArea, "EE" , 0);
            ComboAddItem(hwndSearchArea, "IOP" , 1);
            ComboAddItem(hwndSearchArea, "Kernel" , 2);
            ComboAddItem(hwndSearchArea, "ScratchPad" , 3);
            ComboAddItem(hwndSearchArea, "Custom" , 4);
            SendMessage(hwndSearchAreaLow, EM_SETLIMITTEXT, 8, 0);
            SendMessage(hwndSearchAreaHigh, EM_SETLIMITTEXT, 8, 0);

            //Search Types
            SendMessage(hwndSearchType,CB_RESETCONTENT,0,0);
            ComboAddItem(hwndSearchType, "Initial Dump" , SEARCH_INIT);
            ComboAddItem(hwndSearchType, "I Forgot (just dump again and keep results)" , SEARCH_FORGOT);
            ComboAddItem(hwndSearchType, "Known Value" , SEARCH_KNOWN);
            ComboAddItem(hwndSearchType, "Known Value w/ Wildcards (Hex Input Only)" , SEARCH_KNOWN_WILD);
            ComboAddItem(hwndSearchType, "Greater Than" , SEARCH_GREATER);
            ComboAddItem(hwndSearchType, "Greater Than By <value>" , SEARCH_GREATER_BY);
            ComboAddItem(hwndSearchType, "Greater Than By At Least <value>" , SEARCH_GREATER_LEAST);
            ComboAddItem(hwndSearchType, "Greater Than By At Most <value>" , SEARCH_GREATER_MOST);
            ComboAddItem(hwndSearchType, "Less Than" , SEARCH_LESS);
            ComboAddItem(hwndSearchType, "Less Than By <value>" , SEARCH_LESS_BY);
            ComboAddItem(hwndSearchType, "Less Than By At Least <value>" , SEARCH_LESS_LEAST);
            ComboAddItem(hwndSearchType, "Less Than By At Most <value>" , SEARCH_LESS_MOST);
            ComboAddItem(hwndSearchType, "Equal To" , SEARCH_EQUAL);
            ComboAddItem(hwndSearchType, "Equal To # Bits" , SEARCH_EQUAL_NUM_BITS);
            ComboAddItem(hwndSearchType, "Not Equal To" , SEARCH_NEQUAL);
            ComboAddItem(hwndSearchType, "Not Equal To <value>" , SEARCH_NEQUAL_TO);
            ComboAddItem(hwndSearchType, "Not Equal By <value>" , SEARCH_NEQUAL_BY);
            ComboAddItem(hwndSearchType, "Not Equal By At Least <value>" , SEARCH_NEQUAL_LEAST);
            ComboAddItem(hwndSearchType, "Not Equal By At Most <value>" , SEARCH_NEQUAL_MOST);
            ComboAddItem(hwndSearchType, "Not Equal To # Bits" , SEARCH_NEQUAL_TO_BITS);
            ComboAddItem(hwndSearchType, "Not Equal By # Bits" , SEARCH_NEQUAL_BY_BITS);
            ComboAddItem(hwndSearchType, "In-Range" , SEARCH_IN_RANGE);
            ComboAddItem(hwndSearchType, "Not In-Range" , SEARCH_NOT_RANGE);
            ComboAddItem(hwndSearchType, "Active Bits (Any)" , SEARCH_BITS_ANY);
            ComboAddItem(hwndSearchType, "Active Bits (All)" , SEARCH_BITS_ALL);

            //extended search options
            SendMessage(hwndExSearchList,LVM_DELETEALLITEMS,0,0);
            SendMessage(hwndExSearchList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES|LVS_EX_GRIDLINES|LVS_EX_LABELTIP);
            ListViewAddCol(hwndExSearchList, "Option", 0, 0x230);
            ListViewAddCol(hwndExSearchList, "Value1/Low", 1, 0x9A);
            ListViewAddCol(hwndExSearchList, "Value2/High", 2, 0x9A);
            ListViewAddCol(hwndExSearchList, "", 3, 0);
            char ExVal[20];
            sprintf(ExVal, "%X", EXCS_IGNORE_0);
            ListViewAddRow(hwndExSearchList,4,"Ignore results that are 0", "-", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_FF);
            ListViewAddRow(hwndExSearchList,4,"Ignore results that are FF,FFFF,etc.", "-", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_VALUE);
            ListViewAddRow(hwndExSearchList,4,"Ignore results that are <value>", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_IN_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore results that are in a specified value range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_NOT_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore results that are NOT in a specified value range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_BYTE_VALUE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 8-Bit value", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_SHORT_VALUE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 16-Bit (aligned) value", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_WORD_VALUE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 32-Bit (aligned) value", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_DWORD_VALUE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 64-Bit (aligned) value", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_BYTE_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 8-Bit range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_SHORT_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 16-Bit (aligned) range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_WORD_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 32-Bit (aligned) range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_IGNORE_DWORD_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Ignore any result that's part of a specified 64-Bit (aligned) range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_EXCLUDE_CONSEC);
            ListViewAddRow(hwndExSearchList,4,"Exclude results with # consecutive addresses", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_EXCLUDE_CONSEC_MATCH_VALUES);
            ListViewAddRow(hwndExSearchList,4,"Exclude results with # consecutive addresses and matching values", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_INCLUDE_CONSEC);
            ListViewAddRow(hwndExSearchList,4,"Include Only results with # consecutive addresses", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_INCLUDE_CONSEC_MATCH_VALUES);
            ListViewAddRow(hwndExSearchList,4,"Include Only results with # consecutive addresses and matching values", "0", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_INCLUDE_ADDRESS_RANGE);
            ListViewAddRow(hwndExSearchList,4,"Include Only results within specified address range", "0", "0", ExVal);
            sprintf(ExVal, "%X", EXCS_EXCLUDE_UPPER16);
            ListViewAddRow(hwndExSearchList,4,"Exclude 16-bit Upper Address (0,4,8,C) results", "-", "-", ExVal);
            sprintf(ExVal, "%X", EXCS_EXCLUDE_LOWER16);
            ListViewAddRow(hwndExSearchList,4,"Exclude 16-bit Lower Address (2,6,A,E) results", "-", "-", ExVal);

            //Set starting positions for the dropdown lists (last)
            SendMessage(hwndSearchSize,CB_SETCURSEL,2,0);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_SIZE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchSize);
            SendMessage(hwndCompareTo,CB_SETCURSEL,0,0);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(COMPARE_TO_CMB, CBN_SELCHANGE),(LPARAM)hwndCompareTo);
            SendMessage(hwndSearchType,CB_SETCURSEL,0,0);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
            SendMessage(hwndSearchArea,CB_SETCURSEL,0,0);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_AREA_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchArea);
            //PS2 Waits checkbox
            SendMessage(hwndPS2Waits,BM_SETCHECK,Settings.CS.PS2Wait,0);

            SendMessage(hwndSearchLabel, EM_SETLIMITTEXT, SEARCH_LABEL_MAX, 0);

        } break;
		case WM_COMMAND:
        {
			switch(LOWORD(wParam))
            {
				/************************************************************
				Search Area Combo Box
				*************************************************************/
                case SEARCH_AREA_CMB:
                {
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
                            switch(SendMessage(hwndSearchArea,CB_GETITEMDATA,SendMessage(hwndSearchArea,CB_GETCURSEL,0,0),0))
                            {
                            	case 0: //EE
                            	{
									SetWindowText(hwndSearchAreaLow, "00100000");
									SetWindowText(hwndSearchAreaHigh, "02000000");
									EnableWindow (hwndSearchAreaLow, FALSE);
									EnableWindow (hwndSearchAreaHigh, FALSE);
								} break;
                            	case 1: //IOP
                            	{
									SetWindowText(hwndSearchAreaLow, "BC000000");
									SetWindowText(hwndSearchAreaHigh, "BC200000");
									EnableWindow (hwndSearchAreaLow, FALSE);
									EnableWindow (hwndSearchAreaHigh, FALSE);
								} break;
                            	case 2: //Kernal
                            	{
									SetWindowText(hwndSearchAreaLow, "80000000");
									SetWindowText(hwndSearchAreaHigh, "82000000");
									EnableWindow (hwndSearchAreaLow, FALSE);
									EnableWindow (hwndSearchAreaHigh, FALSE);
								} break;
                            	case 3: //ScratchPad
                            	{
									SetWindowText(hwndSearchAreaLow, "70000000");
									SetWindowText(hwndSearchAreaHigh, "70004000");
									EnableWindow (hwndSearchAreaLow, FALSE);
									EnableWindow (hwndSearchAreaHigh, FALSE);
								} break;
                            	case 4: //Custom
                            	{
									SetWindowText(hwndSearchAreaLow, "00100000");
									SetWindowText(hwndSearchAreaHigh, "02000000");
									EnableWindow (hwndSearchAreaLow, TRUE);
									EnableWindow (hwndSearchAreaHigh, TRUE);
								} break;
							}
                        } break;
                    }
                } break;
				/************************************************************
				Search Size Combo Box
				*************************************************************/
                case SEARCH_SIZE_CMB:
                {
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
                            Search.Size = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
                            switch((Search.Type == SEARCH_KNOWN_WILD) ? BASE_HEX : Settings.CS.NumBase)
                            {
                                case BASE_DEC:
                                {
                                    SendMessage(hwndSearchValue1, EM_SETLIMITTEXT, 31, 0);
                                    SendMessage(hwndSearchValue2, EM_SETLIMITTEXT, 31, 0);
                                } break;
                                case BASE_HEX:
                                {
                                    SendMessage(hwndSearchValue1, EM_SETLIMITTEXT, Search.Size*2, 0);
                                    SendMessage(hwndSearchValue2, EM_SETLIMITTEXT, Search.Size*2, 0);
                                } break;
                                case BASE_FLOAT:
                                {
                                    SendMessage(hwndSearchValue1, EM_SETLIMITTEXT, 32, 0);
                                    SendMessage(hwndSearchValue2, EM_SETLIMITTEXT, 32, 0);
                                } break;
                            }
                        } break;
                    }
                } break;
				/************************************************************
				Search Type Combo Box
				*************************************************************/
                case SEARCH_TYPE_CMB:
                {
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
                            Search.Type = SendMessage(hwndSearchType,CB_GETITEMDATA,SendMessage(hwndSearchType,CB_GETCURSEL,0,0),0);
                            Search.CompareTo = SendMessage(hwndCompareTo,CB_GETCURSEL,0,0);
                            SetWindowText(hwndSearchValue1, "0");
                            SetWindowText(hwndSearchValue2, "0");
                            switch(Search.Type)
                            {
                                case SEARCH_INIT:
                                {
                                    SetWindowPos(hwndSearchValue1, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                    SetWindowPos(hwndSearchValue2, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                    SendMessage(hwndCompareTo,CB_SETCURSEL,0,0);
                                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(COMPARE_TO_CMB, CBN_SELCHANGE),(LPARAM)hwndCompareTo);
                                } break;
                                case SEARCH_KNOWN: case SEARCH_KNOWN_WILD:
                                case SEARCH_GREATER_BY: case SEARCH_GREATER_LEAST: case SEARCH_GREATER_MOST:
                                case SEARCH_LESS_BY: case SEARCH_LESS_LEAST: case SEARCH_LESS_MOST:
                                case SEARCH_EQUAL_NUM_BITS: case SEARCH_NEQUAL_TO: case SEARCH_NEQUAL_BY: case SEARCH_NEQUAL_LEAST:
                                case SEARCH_NEQUAL_MOST: case SEARCH_NEQUAL_TO_BITS: case SEARCH_NEQUAL_BY_BITS:
                                case SEARCH_BITS_ANY: case SEARCH_BITS_ALL:
                                {
                                    SetWindowPos(hwndSearchValue1, 0, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                    SetWindowPos(hwndSearchValue2, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                } break;
                                case SEARCH_GREATER: case SEARCH_LESS: case SEARCH_EQUAL: case SEARCH_NEQUAL:
                                {
                                    if (Search.CompareTo == 0) { MessageBox(NULL, "You can't compare without an initial dump, asshole.", "Error", MB_OK); return 0; }
                                    SetWindowPos(hwndSearchValue1, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                    SetWindowPos(hwndSearchValue2, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                } break;
                                case SEARCH_IN_RANGE: case SEARCH_NOT_RANGE:
                                {
                                    SetWindowPos(hwndSearchValue1, 0, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                    SetWindowPos(hwndSearchValue2, 0, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
                                } break;
                            }
                            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_SIZE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchSize);
                        } break;
                    }
                } break;
				/************************************************************
                Quick search buttons
				*************************************************************/
			    case QS_INIT_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_INIT);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
			    } break;
			    case QS_FORGOT_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_FORGOT);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
				} break;
			    case QS_EQUAL_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_EQUAL);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
			    } break;
			    case QS_NEQUAL_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_NEQUAL);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
			    } break;
			    case QS_GREATER_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_GREATER);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
			    } break;
			    case QS_LESS_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_LESS);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
			        SendMessage(hwnd, WM_COMMAND, DO_SEARCH_CMD, 0);
			    } break;
			    case QS_KNOWN_CMD:
			    {
                    ComboSelFromData(hwndSearchType, SEARCH_KNOWN);
			        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(SEARCH_TYPE_CMB, CBN_SELCHANGE),(LPARAM)hwndSearchType);
				} break;
				/************************************************************
				Dump button
				*************************************************************/
                case TAKE_DUMP_CMD:
                {
					//get start and end address
					u32 DumpAreaLow = GetHexWindow(hwndSearchAreaLow);
					u32 DumpAreaHigh = GetHexWindow(hwndSearchAreaHigh);
					//get filename
					char dmpFileName[MAX_PATH];
                    if (!DoFileSave(hwnd, dmpFileName)) { break; }
                    //Dump the RAM
                    if (!(DumpRAM(dmpFileName, DumpAreaLow, DumpAreaHigh, 0, NULL))) {
						MessageBox(NULL, ErrTxt, "Error", MB_OK); break;
					}
//					MessageBox(NULL, "Did it work? You shouldn't see this until dumping is complete.", "", MB_OK);
				} break;
				/************************************************************
				Search button click AND EX Filter
				*************************************************************/
			    case DO_SEARCH_CMD: case EX_FILTER_CMD:
			    {
                    memset(&Search,0,sizeof(CODE_SEARCH_VARS));
                    FreeRamInfo();
                    Settings.CS.PS2Wait = SendMessage(hwndPS2Waits,BM_GETCHECK,0,0);
                    Search.Size = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
                    Search.Type = SendMessage(hwndSearchType,CB_GETITEMDATA,SendMessage(hwndSearchType,CB_GETCURSEL,0,0),0);
                    Search.CompareTo = SendMessage(hwndCompareTo,CB_GETCURSEL,0,0);
                    Search.Count = SendMessage(hwndCompareTo,CB_GETCOUNT,0,0);
                    if (LOWORD(wParam) == EX_FILTER_CMD) { Search.Type = SEARCH_EQUAL; }
                    if (Search.CompareTo == 0) { Search.Count = 1; }
                    int i;
                    char txtValue[17];

                    //Switch case based on search type to grab whatever input we need in the format we need.
                    switch(Search.Type)
                    {
                        case SEARCH_INIT:
                        {
                            Search.CompareTo = 0;
                            Search.Count = 1;
#if (SNAKE_DEBUG != 1)
                            ClearDumpsFolder();
#endif
                        } break;
                        case SEARCH_KNOWN_WILD:
                        {
                            GetWindowText(hwndSearchValue1, txtValue, sizeof(txtValue));
                            Search.Values[1] = 0xFFFFFFFFFFFFFFFFLL;
                            for (i = 0; i < strlen(txtValue); i++) {
                                if (txtValue[i] == '*') {
                                    Search.Values[1] &= ~((u64)0xF << ((strlen(txtValue) - i - 1) * 4));
                                    txtValue[i] = '0';
                                }
                            }
                            String2Hex(txtValue,&Search.Values[0]);
                        } break;
                        case SEARCH_KNOWN:
                        case SEARCH_GREATER_BY: case SEARCH_GREATER_LEAST: case SEARCH_GREATER_MOST:
                        case SEARCH_LESS_BY: case SEARCH_LESS_LEAST: case SEARCH_LESS_MOST:
                        case SEARCH_EQUAL_NUM_BITS: case SEARCH_NEQUAL_TO: case SEARCH_NEQUAL_BY: case SEARCH_NEQUAL_LEAST:
                        case SEARCH_NEQUAL_MOST: case SEARCH_NEQUAL_TO_BITS: case SEARCH_NEQUAL_BY_BITS:
                        case SEARCH_BITS_ANY: case SEARCH_BITS_ALL:
                        case SEARCH_IN_RANGE: case SEARCH_NOT_RANGE:
                        {
                            switch((Search.Type & (SEARCH_EQUAL_NUM_BITS|SEARCH_NEQUAL_TO_BITS|SEARCH_NEQUAL_BY_BITS|SEARCH_BITS_ANY|SEARCH_BITS_ALL)) &&
                            (Settings.CS.NumBase == BASE_FLOAT) ? BASE_HEX : Settings.CS.NumBase)
                            {
                                case BASE_HEX:
                                {
                                    Search.Values[0] = GetHexWindow(hwndSearchValue1);
                                    Search.Values[1] = GetHexWindow(hwndSearchValue2);
                                } break;
                                case BASE_DEC:
                                {
                                    Search.Values[0] = GetDecWindow(hwndSearchValue1);
                                    Search.Values[1] = GetDecWindow(hwndSearchValue2);
                                } break;
                                case BASE_FLOAT:
                                {
                                    Search.Values[0] = GetFloatWindow(hwndSearchValue1, Search.Size);
                                    Search.Values[1] = GetFloatWindow(hwndSearchValue2, Search.Size);
                                } break;
                            }
                        } break;
                        case SEARCH_GREATER: case SEARCH_LESS: case SEARCH_EQUAL: case SEARCH_NEQUAL:
                        {
                            if ((Search.Count == 1) || (Search.CompareTo == 0)) { MessageBox(NULL, "You can't compare without an initial dump, n00b.", "Error", MB_OK); return 0; }
                        } break;
                        case SEARCH_FORGOT:
                        {
							if ((Search.Count == 1) || (Search.CompareTo == 0)) { MessageBox(NULL, "No previous results. You apparently forgot to start a search at all, you fuckin idiot. Try again.", "Error", MB_OK); return 0; }
						} break;
                    }
                    if (Search.Count > MAX_SEARCHES) { MessageBox(NULL,"Holy shit! 100 searches? If you didn't find the code by now, give it up.","Error",MB_OK); return 0; }
                    //Signed Search check
                    if (SendMessage(hwndSignedSearchChk,BM_GETCHECK,0,0) == BST_CHECKED) { Search.TypeEx |= EXCS_SIGNED; }
                    //Grab EX Search options
                    int exCount = SendMessage(hwndExSearchList, LVM_GETITEMCOUNT, 0, 0);
                    for (i = 0; i < exCount; i++) {
                        if (ListView_GetCheckState(hwndExSearchList, i)) {
                            Search.TypeEx |= ListViewGetHex(hwndExSearchList, i, 3);
                            SetExValues(&Search, ListViewGetHex(hwndExSearchList, i, 3), ListViewGetHex(hwndExSearchList, i, 1), ListViewGetHex(hwndExSearchList, i, 2));
                        }
                    }

                    //Load previous results if continuing a search.
                    char sdFileName[MAX_PATH];
                    if (Search.CompareTo) {
                        sprintf(sdFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.CompareTo);
                        if (!(LoadStruct(&RamInfo.OldResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), sdFileName))) { FreeRamInfo(); return 0; }
                        sprintf(sdFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.Count - 1);
                        if (!(LoadFile(&RamInfo.Results, sdFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { FreeRamInfo(); return 0; }
                    }
					//setup new results info
                    sprintf(RamInfo.NewResultsInfo.dmpFileName, "%sdump%u.raw", Settings.CS.DumpDir, Search.Count);
                    RamInfo.NewResultsInfo.Endian = LITTLE_ENDIAN_SYS;
                    RamInfo.NewResultsInfo.SearchSize = Search.Size;
                    GetWindowText(hwndSearchLabel, RamInfo.NewResultsInfo.SearchLabel, sizeof(RamInfo.NewResultsInfo.SearchLabel));
					//get start and end address
					u32 DumpAreaLow = GetHexWindow(hwndSearchAreaLow);
					u32 DumpAreaHigh = GetHexWindow(hwndSearchAreaHigh);
					//dump ram
					if (LOWORD(wParam) == EX_FILTER_CMD) {
						memcpy(&RamInfo.NewResultsInfo, &RamInfo.OldResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO));
                    	sprintf(RamInfo.NewResultsInfo.dmpFileName, "%sdump%u.raw", Settings.CS.DumpDir, Search.Count);
//                    	MessageBox(NULL, RamInfo.NewResultsInfo.dmpFileName, "Debug", MB_OK); FreeRamInfo(); return 0;
						if (!CopyBinFile(RamInfo.OldResultsInfo.dmpFileName, RamInfo.NewResultsInfo.dmpFileName)) { FreeRamInfo(); return 0; }
						//send message to continue filter
						SendMessage(hwnd, WM_COMMAND, SEARCH_CONTINUE_VCMD, 1);
						break;
					} else if (Settings.CS.FileMode == MFS_CHECKED) {
						if (!DoFileOpen(hwnd, RamInfo.NewResultsInfo.dmpFileName)) { break; }
                        RamInfo.NewFile = fopen(RamInfo.NewResultsInfo.dmpFileName, "rb");
                        if (!(RamInfo.NewFile)) {
                            sprintf(ErrTxt, "Unable to open ram dump (DO_SEARCH_CMD) -- Error %u", GetLastError());
                            MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
                        }
                        fseek(RamInfo.NewFile,0,SEEK_END);
                        RamInfo.NewResultsInfo.DumpSize = ftell(RamInfo.NewFile);
                        fclose(RamInfo.NewFile);
						//keep track of the memory address the file really starts on for displaying results
                    	RamInfo.NewResultsInfo.MapFileAddy = 0;
                    	RamInfo.NewResultsInfo.MapMemAddy = DumpAreaLow;
						SendMessage(hwnd, WM_COMMAND, SEARCH_CONTINUE_VCMD, 1);
						break;
					} else {
						if (Settings.CS.PS2Wait) { SendMessage(DlgInfo.Main, WM_COMMAND, MNU_HALT, 0); Sleep(100); }
#if (SNAKE_DEBUG != 1)
                    	if (!(DumpRAM(RamInfo.NewResultsInfo.dmpFileName, DumpAreaLow, DumpAreaHigh, SEARCH_CONTINUE_VCMD, hwnd))) {
							MessageBox(NULL, ErrTxt, "Error", MB_OK); FreeRamInfo(); return 0;
						}
#endif
                    	RamInfo.NewResultsInfo.DumpSize = DumpAreaHigh - DumpAreaLow;
						//keep track of the memory address the file really starts on for displaying results
                    	RamInfo.NewResultsInfo.MapFileAddy = 0;
                    	RamInfo.NewResultsInfo.MapMemAddy = DumpAreaLow;
					}
					EnableWindow(hwnd, FALSE);
					EnableWindow(hwndtabCtrl, FALSE);
				} break;
				/************************************************************
				Search Continue (after dumping is complete)
				*************************************************************/
				case SEARCH_CONTINUE_VCMD:
				{
					EnableWindow(hwnd, TRUE);
					EnableWindow(hwndtabCtrl, TRUE);
					if (!lParam) {
						//Dump failed, clear shit
						goto search_end;
					}
                    char sdFileName[MAX_PATH];
                    //if new search, setup a fresh results file
                    if (!Search.CompareTo) {
                        Search.Count = 1;
                        SetWindowText(hwndSearchHistory, "");
                        SendMessage(hwndCompareTo,CB_RESETCONTENT,0,0);
                        ComboAddItem(hwndCompareTo, "New Search" , 0);
                        SendMessage(hwndCompareTo,CB_SETCURSEL,0,0);
                        sprintf(sdFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.Count);
                        if (!(RamInfo.Results = (unsigned char*)malloc(RamInfo.NewResultsInfo.DumpSize/Search.Size/8))) {
                            sprintf(ErrTxt, "Unable to allocate results memory (DO_SEARCH_CMD) -- Error %u", GetLastError());
                            MessageBox(NULL, ErrTxt, "Error", MB_OK); goto search_end;
                        }
                        memset(RamInfo.Results, 0xFF, (RamInfo.NewResultsInfo.DumpSize/Search.Size/8));
                        RamInfo.NewResultsInfo.ResCount = RamInfo.NewResultsInfo.DumpSize/Search.Size;
                        if (!(SaveFile(RamInfo.Results, (RamInfo.NewResultsInfo.DumpSize/Search.Size/8), sdFileName, sizeof(CODE_SEARCH_RESULTS_INFO), &RamInfo.NewResultsInfo))) {
							goto search_end;
						}
                    }
                    //update the Compare To list
                    sprintf(sdFileName, "#%u: %s", Search.Count, RamInfo.NewResultsInfo.SearchLabel);
                    ComboAddItem(hwndCompareTo, sdFileName , Search.Count);
                    SendMessage(hwndCompareTo,CB_SETCURSEL,Search.Count,0);
                    //take care of a couple non-comparision search types
                    if (Search.Type == SEARCH_INIT) {
						UpdateSearchHistory(LOWORD(wParam));
                        goto search_end;
                    }
                    if (Search.Type == SEARCH_FORGOT) {
                        sprintf(sdFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.Count);
                        RamInfo.NewResultsInfo.ResCount = RamInfo.OldResultsInfo.ResCount;
                        SaveFile(RamInfo.Results, (RamInfo.NewResultsInfo.DumpSize/Search.Size/8), sdFileName, sizeof(CODE_SEARCH_RESULTS_INFO), &RamInfo.NewResultsInfo);
                        UpdateSearchHistory(LOWORD(wParam));
                        goto search_end;
                    }
                    //load the dump(s) for compare
                    RamInfo.Access = Settings.CS.DumpAccess;
                    if (RamInfo.Access == SEARCH_ACCESS_ARRAY) {
						if (!(LoadFile(&RamInfo.NewRAM, RamInfo.NewResultsInfo.dmpFileName, 0, NULL, FALSE))) { goto search_end; }
                    	if (Search.CompareTo) {
							if (!LoadFile(&RamInfo.OldRAM, RamInfo.OldResultsInfo.dmpFileName, 0, NULL, FALSE)) { goto search_end; }
                    	}
					} else {
						//only loading file handles
                        RamInfo.NewFile = fopen(RamInfo.NewResultsInfo.dmpFileName, "rb");
                        if (!(RamInfo.NewFile)) {
                            sprintf(ErrTxt, "Unable to open ram dump (CMD_CS_SEARCH,2) -- Error %u", GetLastError());
                            MessageBox(NULL,ErrTxt,"Error",MB_OK); goto search_end;
                        }
                        RamInfo.OldFile = fopen(RamInfo.OldResultsInfo.dmpFileName, "rb");
                        if ((!RamInfo.OldFile) && Search.CompareTo) {
							sprintf(ErrTxt, "Unable to open previous ram dump (CMD_CS_SEARCH,3) -- Error %u", GetLastError());
                            MessageBox(NULL,ErrTxt,"Error",MB_OK); goto search_end;
                        }
                    }
                    if ((RamInfo.OldResultsInfo.DumpSize > 0) && (RamInfo.OldResultsInfo.DumpSize != RamInfo.NewResultsInfo.DumpSize)) {
                        MessageBox(NULL, "RAM dumps don't match in size. Are you trying to compare files of differnet size?", "Error", MB_OK);
                        goto search_end;
                    }
                    //init progress bar
                    UpdateProgressBar(PBM_SETRANGE, 0, MAKELPARAM(0, (RamInfo.NewResultsInfo.DumpSize/0x100000)+((RamInfo.NewResultsInfo.DumpSize % 0x100000) ? 1:0)));
                    UpdateProgressBar(PBM_SETSTEP, 1, 0);
                    UpdateStatusBar("Searching...", 0, 0);
                    //call for search
                  	CodeSearch(Search);
					//store results and such
                    sprintf(sdFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.Count);
                    SaveFile(RamInfo.Results, (RamInfo.NewResultsInfo.DumpSize/Search.Size/8), sdFileName, sizeof(CODE_SEARCH_RESULTS_INFO), &RamInfo.NewResultsInfo);
                    sprintf(sdFileName, "%d Results", RamInfo.NewResultsInfo.ResCount);
                    UpdateStatusBar(sdFileName, 0,0);
                    UpdateSearchHistory(LOWORD(wParam));
search_end:
					if (Settings.CS.PS2Wait) { SendMessage(DlgInfo.Main, WM_COMMAND, MNU_RESUME, 0); }
                    FreeRamInfo();
					UpdateProgressBar(PBM_SETPOS, 0, 0);
				} break;
				/************************************************************
				Prep and show edit box on extended search options listview
				*************************************************************/
			    case LSV_CS_BEGINEDIT:
			    {
			        if (DlgInfo.lvEdit[LV_EX_SEARCH].Status) { MessageBox(NULL,"Already editing. WTF? (LSV_CS_BEGINEDIT)","Error",0); break; }
			        if ((HIWORD(lParam) < 1) || (HIWORD(lParam) > 2)) { break; }
			        char txtInput[20];
			        DlgInfo.lvEdit[LV_EX_SEARCH].iItem = LOWORD(lParam);
			        DlgInfo.lvEdit[LV_EX_SEARCH].iSubItem = HIWORD(lParam);
			        RECT lvEditRect; memset(&lvEditRect,0,sizeof(RECT));
			        lvEditRect.top = DlgInfo.lvEdit[LV_EX_SEARCH].iSubItem;
			        lvEditRect.left = LVIR_LABEL;
			        SendMessage(hwndExSearchList, LVM_GETSUBITEMRECT, DlgInfo.lvEdit[LV_EX_SEARCH].iItem, (LPARAM)&lvEditRect);
			        Search.Size = SendMessage(hwndSearchSize,CB_GETITEMDATA,SendMessage(hwndSearchSize,CB_GETCURSEL,0,0),0);
			        u32 ExType = ListViewGetHex(hwndExSearchList, DlgInfo.lvEdit[LV_EX_SEARCH].iItem, 3);
			        switch(ExType)
			        {
                        case EXCS_IGNORE_VALUE: case EXCS_IGNORE_IN_RANGE:
                        {
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, Search.Size*2, 0);
                        } break;
                        case EXCS_IGNORE_BYTE_VALUE: case EXCS_IGNORE_BYTE_RANGE:
                        {
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, 2, 0);
                        } break;
                        case EXCS_IGNORE_SHORT_VALUE: case EXCS_IGNORE_SHORT_RANGE:
                        {
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, 4, 0);
                        } break;
                        case EXCS_IGNORE_WORD_VALUE: case EXCS_IGNORE_WORD_RANGE:
                        case EXCS_INCLUDE_ADDRESS_RANGE:
                        {
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, 8, 0);
                        } break;
                        case EXCS_IGNORE_DWORD_VALUE: case EXCS_IGNORE_DWORD_RANGE:
                        {
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, 16, 0);
                        } break;
                        case EXCS_EXCLUDE_CONSEC: case EXCS_EXCLUDE_CONSEC_MATCH_VALUES:
                        case EXCS_INCLUDE_CONSEC: case EXCS_INCLUDE_CONSEC_MATCH_VALUES:
                        {
                            if (DlgInfo.lvEdit[LV_EX_SEARCH].iSubItem == 2) { return 0; }
                            SendMessage(hwndExValueTxt, EM_SETLIMITTEXT, 4, 0);
                        } break;
			        }
			        ListViewGetText(hwndExSearchList, DlgInfo.lvEdit[LV_EX_SEARCH].iItem, DlgInfo.lvEdit[LV_EX_SEARCH].iSubItem, txtInput, sizeof(txtInput));
			        SetWindowText(hwndExValueTxt,txtInput);
			        WINDOWPLACEMENT lvPlace; memset(&lvPlace,0,sizeof(WINDOWPLACEMENT));
			        lvPlace.length = sizeof(WINDOWPLACEMENT);
			        GetWindowPlacement(hwndExSearchList, &lvPlace);
			        POINT lvPos;
			        lvPos.x = lvPlace.rcNormalPosition.left;
			        lvPos.y = lvPlace.rcNormalPosition.top;
			        SetWindowPos(hwndExValueTxt,HWND_TOP,lvPos.x+lvEditRect.left+3,lvPos.y+lvEditRect.top+1,(lvEditRect.right-lvEditRect.left),(lvEditRect.bottom-lvEditRect.top)+1,SWP_SHOWWINDOW);
			        SetFocus(hwndExValueTxt);
			        SendMessage(hwndExValueTxt, EM_SETSEL, 0, -1);
			        DlgInfo.lvEdit[LV_EX_SEARCH].Status = 1;
			    } break;
			    case LSV_CS_ENDEDIT:
                {
                    if ((!DlgInfo.lvEdit[LV_EX_SEARCH].Status) || (!lParam)) {
                        DlgInfo.lvEdit[LV_EX_SEARCH].Status = 0;
                        SetWindowPos(hwndExValueTxt,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
                        SetFocus(hwndExSearchList); break;
                    }
			        char txtValue[20];
			        if (isHexWindow(hwndExValueTxt)) { GetWindowText(hwndExValueTxt, txtValue, sizeof(txtValue)); }
			        else { strcpy(txtValue, "0"); }
			        ListViewSetRow(hwndExSearchList, DlgInfo.lvEdit[LV_EX_SEARCH].iItem, DlgInfo.lvEdit[LV_EX_SEARCH].iSubItem, 1, txtValue);
                    DlgInfo.lvEdit[LV_EX_SEARCH].Status = 0;
                    SetWindowPos(hwndExValueTxt,HWND_BOTTOM,0,0,0,0,SWP_HIDEWINDOW);
                    SetFocus(hwndExSearchList);
			    } break;
			    case MNU_CS_UNDO:
			    {
                    Search.Count = SendMessage(hwndCompareTo,CB_GETCOUNT,0,0);
                    if (Search.Count < 3) { break; }
                    sprintf(RamInfo.NewResultsInfo.dmpFileName, "%ssearch%u.bin", Settings.CS.DumpDir, Search.Count - 2);
                    if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), RamInfo.NewResultsInfo.dmpFileName))) { break; }
                    SendMessage(hwndCompareTo, CB_DELETESTRING, Search.Count - 1, 0);
                    SendMessage(hwndCompareTo, CB_SETCURSEL, Search.Count - 2,0);
                    char ResTxt[20];
                    sprintf(ResTxt, "%u Results", RamInfo.NewResultsInfo.ResCount);
                    UpdateStatusBar(ResTxt, 0,0);
                    UpdateSearchHistory(MNU_CS_UNDO);
                    FreeRamInfo();
				} break;
			    case MNU_LOAD_SEARCH:
                {
                    char ResFileName[MAX_PATH];
                    if (!DoFileOpen(hwnd, ResFileName)) { break; }
                    if (!(LoadStruct(&RamInfo.NewResultsInfo, sizeof(CODE_SEARCH_RESULTS_INFO), ResFileName))) { break; }
                    if (!(LoadFile(&RamInfo.Results, ResFileName, sizeof(CODE_SEARCH_RESULTS_INFO), NULL, FALSE))) { break; }
                    sprintf(ResFileName, "%sdump1.raw", Settings.CS.DumpDir);
                    if(!CopyBinFile(RamInfo.NewResultsInfo.dmpFileName, ResFileName)) { FreeRamInfo(); break; }
//                    rename (RamInfo.NewResultsInfo.dmpFileName, ResFileName);
                    strcpy(RamInfo.NewResultsInfo.dmpFileName, ResFileName);
                    sprintf(ResFileName, "%ssearch1.bin", Settings.CS.DumpDir);
                    SaveFile(RamInfo.Results, (RamInfo.NewResultsInfo.DumpSize/Search.Size/8), ResFileName, sizeof(CODE_SEARCH_RESULTS_INFO), &RamInfo.NewResultsInfo);
                    SendMessage(hwndCompareTo,CB_RESETCONTENT,0,0);
                    ComboAddItem(hwndCompareTo, "New Search" , 0);
                    ComboAddItem(hwndCompareTo, "#1:" , 0);
                    SendMessage(hwndCompareTo,CB_SETCURSEL,1,0);
                    sprintf(ResFileName, "%u", RamInfo.NewResultsInfo.ResCount);
                    UpdateStatusBar(ResFileName, 0,0);
                    UpdateSearchHistory(MNU_LOAD_SEARCH);
                    FreeRamInfo();
                } break;
			}
		} break;

	}
	return FALSE;
}

/****************************************************************************
Extended Search Options ListView handler
-Manipulates the Extended Search options listview control as needed
*****************************************************************************/
LRESULT CALLBACK ExSearchListHandler (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
    switch (message)
    {
        case WM_VSCROLL: case WM_MOUSEWHEEL:
        {
            if (DlgInfo.lvEdit[LV_EX_SEARCH].Status) { SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_ENDEDIT, 0); }
        } break;
        case WM_LBUTTONUP:
        {
            int iSelected = ListViewHitTst(hwnd, GetMessagePos(), -1);
            if (iSelected < 0) { break; }
            if (!ListView_GetCheckState(hwnd, iSelected)) { break; }
            if (!(ListViewGetHex(hwnd, iSelected, 3) & (EXCS_EXCLUDE_CONSEC|EXCS_INCLUDE_CONSEC|EXCS_EXCLUDE_CONSEC_MATCH_VALUES|EXCS_INCLUDE_CONSEC_MATCH_VALUES))) { break; }
            int i, x = 0;
            int exCount = SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0);
            for (i = 0; i < exCount; i++) {
                if ((ListView_GetCheckState(hwnd, i)) && (ListViewGetHex(hwnd, i, 3) &
                    (EXCS_EXCLUDE_CONSEC|EXCS_INCLUDE_CONSEC|EXCS_EXCLUDE_CONSEC_MATCH_VALUES|EXCS_INCLUDE_CONSEC_MATCH_VALUES))) {
                        x++;
                }
            }
            if (x > 1) {
                ListView_SetCheckState(hwnd, iSelected, FALSE);
                MessageBox(NULL, "Only 1 Consecutive/Matching Results option can be used at a time.", "Error", MB_OK);
            }
        } break;
        case WM_LBUTTONDBLCLK:
        {
            if (DlgInfo.lvEdit[LV_EX_SEARCH].Status) { SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_ENDEDIT, 0); }
            int iSelected = SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0);
            if (iSelected < 0) { break; }
            int iSubItem = ListViewHitTst(hwnd, GetMessagePos(), iSelected);
            u32 ExType = ListViewGetHex(hwnd, iSelected, 3);
            if (ExType & (EXCS_SIGNED|EXCS_OR_EQUAL|EXCS_IGNORE_0|EXCS_IGNORE_FF|EXCS_IGNORE_N64_POINTERS)) { break; }
            if ((ExType & (EXCS_IGNORE_VALUE|EXCS_IGNORE_BYTE_VALUE|EXCS_IGNORE_SHORT_VALUE|EXCS_IGNORE_WORD_VALUE|EXCS_IGNORE_DWORD_VALUE|EXCS_EXCLUDE_CONSEC|EXCS_EXCLUDE_CONSEC_MATCH_VALUES|EXCS_INCLUDE_CONSEC|EXCS_INCLUDE_CONSEC_MATCH_VALUES)) && (iSubItem == 2)) { break; }
            SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_BEGINEDIT, MAKELPARAM(iSelected, iSubItem));
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
Clear Dump Folder
-This makes a silent attempt at clearing search-related files (dump#.raw, search#.bin)
*****************************************************************************/
int ClearDumpsFolder()
{
	int i;
	char tmpfilename[MAX_PATH];
	for (i = 1; i < MAX_SEARCHES; i++) {
		sprintf(tmpfilename, "%sdump%u.raw", Settings.CS.DumpDir, i);
		if (remove(tmpfilename)) { return 0; }
		sprintf(tmpfilename, "%ssearch%u.bin", Settings.CS.DumpDir, i);
		if (remove(tmpfilename)) { return 0; }
	}
	return 1;
}

/****************************************************************************
Search History
-RamInfo and Search structs still need to be active for this
*****************************************************************************/
int UpdateSearchHistory(int ActionType)
{
	HWND hwndSearchType = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_TYPE_CMB);
	HWND hwndSearchHistory = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_HISTORY_TXT);
    HWND hwndSearchValue1 = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_VALUE1_TXT);
    HWND hwndSearchValue2 = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_VALUE2_TXT);
    HWND hwndSignedSearchChk = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_SIGNED_CHK);
	char SearchTypeText[1000];
	char HistoryText[1500];
	char ValuesText[200];
	char Value1Text[50];
	char Value2Text[50];
	char SignedText[20];
	strcpy(ValuesText, "");
	strcpy(SignedText, "");
	GetWindowText(hwndSearchValue1, Value1Text, sizeof(Value1Text));
	GetWindowText(hwndSearchValue2, Value2Text, sizeof(Value2Text));

	switch (ActionType) {
		case EX_FILTER_CMD: { strcpy(SearchTypeText, "EX Filter"); } break;
		case MNU_CS_UNDO: { strcpy(SearchTypeText, "Undo Previous Search"); } break;
		case MNU_LOAD_SEARCH: { strcpy(SearchTypeText, "Load Search"); } break;
		default:
		{
			SendMessage(hwndSearchType, CB_GETLBTEXT, ComboSelFromData(hwndSearchType, Search.Type), (LPARAM)SearchTypeText);
			if (SendMessage(hwndSignedSearchChk,BM_GETCHECK,0,0) == BST_CHECKED) { strcpy(SignedText, " [Signed]"); }
			//Set values string, if applicable
			if (Search.Type & (SEARCH_KNOWN|SEARCH_KNOWN_WILD|SEARCH_GREATER_BY|SEARCH_GREATER_LEAST|SEARCH_GREATER_MOST|
	    		SEARCH_LESS_BY|SEARCH_LESS_LEAST|SEARCH_LESS_MOST|SEARCH_EQUAL_NUM_BITS|SEARCH_NEQUAL_TO|
	    		SEARCH_NEQUAL_BY|SEARCH_NEQUAL_LEAST|SEARCH_NEQUAL_MOST|SEARCH_NEQUAL_TO_BITS|SEARCH_NEQUAL_BY_BITS|
	    		SEARCH_BITS_ANY|SEARCH_BITS_ALL)) {
					sprintf(ValuesText, " (%s)", Value1Text);
			} else if (Search.Type & (SEARCH_IN_RANGE|SEARCH_NOT_RANGE)) {
				sprintf(ValuesText, " (%s - %s)", Value1Text, Value2Text);
			}
		} break;
	}
	sprintf(HistoryText, "[%s] (%u) %s%s%s\n", RamInfo.NewResultsInfo.SearchLabel, RamInfo.NewResultsInfo.ResCount, SearchTypeText, SignedText, ValuesText);
	SendMessage(hwndSearchHistory, EM_SETSEL, -1, -1);
	SendMessage(hwndSearchHistory, EM_REPLACESEL, TRUE, (LPARAM)HistoryText);
	return 0;
}

