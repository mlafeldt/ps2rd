/****************************************************************************
Artemis - PS2 Code Creator (for lack of a better title)
-PS2 dumping/communication functions by Jimmikaelkael
-Code searching by Viper187

I've implemented the source from Jimmikaelkael's core dumper v2. I've reworked
the threading a bit and put my own spin on it. I'll try to keep things somewhat
commented everywhere so the whole app can be followed in case anyone feels the
need to mess with it later. I'm usually around to answer questions though.

p.s. Please don't screw up my source with Visual Stupid 6.0/.net/2008 or any
other random dev environment. MinGW and Textpad for the win! ...even though
MakeFiles are a little annoying to work on.

To Do:
export results
lock menus during search
fonts (default and custom)
memory editor
*****************************************************************************/

#include "ps2cc.h"
#include "ps2cc_gui.h"

char ErrTxt[1000];

//Global structs
MAIN_CFG Settings;
RAM_AND_RES_DATA RamInfo;
HWND_WNDPROC_INFO DlgInfo;

/****************************************************************************
Tab Control
-This creates a dialog (window) for each tab and positions them over the tab
 control. Each tab has it's own windows process as if it were really a
 seperate window.
****************************************************************************/
int InitTabControl(HWND hwnd, LPARAM lParam)
{
    TCITEM tabitem;
    HWND hTab;

    hTab = GetDlgItem(hwnd, PS2CC_TABCTRL);
    //insert tabs
    memset(&tabitem, 0, sizeof(tabitem));
    tabitem.mask = TCIF_TEXT;
	tabitem.cchTextMax = MAX_PATH;

    tabitem.pszText = "Search";
    SendMessage(hTab, TCM_INSERTITEM, CODE_SEARCH_TAB, (LPARAM)&tabitem);

    tabitem.pszText = "Results";
    SendMessage(hTab, TCM_INSERTITEM, SEARCH_RESULTS_TAB, (LPARAM)&tabitem);

    tabitem.pszText = "Memory Editor";
    SendMessage(hTab, TCM_INSERTITEM, MEMORY_EDITOR_TAB, (LPARAM)&tabitem);

//    tabitem.pszText = "Cheat";
//    SendMessage(hTab, TCM_INSERTITEM, CHEAT_TAB, (LPARAM)&tabitem);

	// Get the position that the dialogs should be displayed
	RECT rt,itemrt;
	GetWindowRect(hTab, &rt);
    TabCtrl_GetItemRect(hTab,1,&itemrt);
    rt.top -= (itemrt.top - itemrt.bottom);
    rt.bottom -= rt.top;
    rt.right  -= rt.left;
	ScreenToClient(hTab, (LPPOINT)&rt);

	// Create the dialogs modelessly and move them appropriately
    DlgInfo.TabDlgs[CODE_SEARCH_TAB] = CreateDialog((HINSTANCE)lParam, (LPSTR)SEARCH_DLG, hTab, (DLGPROC)CodeSearchProc);
    DlgInfo.TabDlgs[SEARCH_RESULTS_TAB] = CreateDialog((HINSTANCE)lParam, (LPSTR)RESULTS_DLG, hTab, (DLGPROC)SearchResultsProc);

    DlgInfo.TabDlgs[MEMORY_EDITOR_TAB] = CreateDialog((HINSTANCE)lParam, (LPSTR)MEMORY_EDITOR_DLG, hTab, (DLGPROC)MemoryEditorProc);
//    hTabDlgs[CHEAT_TAB] = CreateDialog((HINSTANCE)lParam, (LPSTR)DLG_CHEAT, hTab, (DLGPROC)CheatProc);

    MoveWindow(DlgInfo.TabDlgs[CODE_SEARCH_TAB], rt.left, rt.top, rt.right, rt.bottom, 0);
    MoveWindow(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], rt.left, rt.top, rt.right, rt.bottom, 0);

    MoveWindow(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], rt.left, rt.top, rt.right, rt.bottom, 0);
//    MoveWindow(hTabDlgs[CHEAT_TAB], rt.left, rt.top, rt.right, rt.bottom, 0);

    // Show the default dialog
    ShowWindow(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SW_SHOW);
//    SendMessage(hTab, TCM_SETCURSEL, CODE_SEARCH_TAB, 0);
    SendMessage(hTab, TCM_SETCURFOCUS, CODE_SEARCH_TAB, 0);
    return 0;
}

/****************************************************************************
Main window thread - Basically a tab control, status bar, and progress bar.
Menu handlers might go here later as well.
****************************************************************************/

BOOL CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	HMENU hMenu = GetMenu(hwnd);
	HWND hwndStatusBar = GetDlgItem(hwnd, NTPB_STATUS_BAR);
	HWND hwndTabCtrl = GetDlgItem(hwnd, PS2CC_TABCTRL);
	switch (msg) {
 		case WM_INITDIALOG:
        {
			LoadSettings();
		    //init menus
		    SetMenuItemData(hMenu, MNU_CS_INPUT_HEX, BASE_HEX);
		    SetMenuItemData(hMenu, MNU_CS_INPUT_DEC, BASE_DEC);
		    SetMenuItemData(hMenu, MNU_CS_INPUT_FLOAT, BASE_FLOAT);
		    SetMenuItemData(hMenu, MNU_MEM_SHOW_BYTES, 1);
		    SetMenuItemData(hMenu, MNU_MEM_SHOW_SHORTS, 2);
		    SetMenuItemData(hMenu, MNU_MEM_SHOW_WORDS, 4);
		    //apply settings to menus
		    SetMenuState(hMenu, Settings.Results.DisplayFmt, MFS_CHECKED);
		    SetMenuState(hMenu, Settings.CS.NumBaseId, MFS_CHECKED);
		    SetMenuState(hMenu, Settings.MemEdit.EditSizeId, MFS_CHECKED);
			EnableMenuItem(hMenu, MNU_RESUME, MF_BYCOMMAND|MF_GRAYED);
			SetMenuState(hMenu, MNU_CS_FILE_MODE, Settings.CS.FileMode);
		    //setup statusbar
/* This is pissing me off
            RECT StatusRect; memset(&StatusRect,0,sizeof(StatusRect));
            GetWindowRect(hwndStatusBar,&StatusRect);
		    int statwidths[1];
		    statwidths[0] = (StatusRect.right - StatusRect.left) * 0.75;
		    statwidths[1] = (StatusRect.right - StatusRect.left) * 0.25;
            SendMessage(hwndStatusBar, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
 */
		    //To Do: set fonts here later

		    InitTabControl(hwnd, lParam);
		} break;
        case WM_NOTIFY:
        {
            NMHDR *hdr = (LPNMHDR)lParam;
            if (hdr->code == TCN_SELCHANGING || hdr->code == TCN_SELCHANGE)  //switching tabs
            {
                int index = TabCtrl_GetCurSel(hdr->hwndFrom);
                if (index >= 0 && index < NUM_TABS) {
					ShowWindow(DlgInfo.TabDlgs[index], (hdr->code == TCN_SELCHANGE) ? SW_SHOW : SW_HIDE);
					DlgInfo.ActiveTab = index;
				}
            }
        } break;
		case WM_COMMAND:
        {
			switch(LOWORD(wParam))
            {
			    case MNU_DUMP_DIR:
			    {
                    char DumpPath[MAX_PATH];
                    if (BrowseForFolder(hwnd, DumpPath)) {
                        strcpy(Settings.CS.DumpDir, DumpPath);
                        SetMenuItemText(hMenu, MNU_DUMP_DIR, Settings.CS.DumpDir);
                    }
			    } break;
			    case MNU_SHOW_CONFIG:
			    {
					DialogBox(DlgInfo.Instance, MAKEINTRESOURCE(SETTINGS_DLG), hwnd, IpConfigDlg);
				} break;
                case MNU_RES_SHOW_HEX: case MNU_RES_SHOW_DECU: case MNU_RES_SHOW_DECS: case MNU_RES_SHOW_FLOAT:
                {
                    Settings.Results.DisplayFmt = LOWORD(wParam);
			        SetMenuState(hMenu, MNU_RES_SHOW_HEX, MFS_UNCHECKED);
			        SetMenuState(hMenu, MNU_RES_SHOW_DECU, MFS_UNCHECKED);
			        SetMenuState(hMenu, MNU_RES_SHOW_DECS, MFS_UNCHECKED);
			        SetMenuState(hMenu, MNU_RES_SHOW_FLOAT, MFS_UNCHECKED);
			        SetMenuState(hMenu, LOWORD(wParam), MFS_CHECKED);
			        if (ResultsList) { ShowResPage(0); } //possibly make CurrResNum a global later
                } break;
				case MNU_RES_PAGE_DOWN: case MNU_RES_PAGE_UP:
				{
					if ((ResultsList) && (SendMessage(hwndTabCtrl, TCM_GETCURFOCUS, 0, 0) == SEARCH_RESULTS_TAB)) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], msg, wParam, lParam); }
				} break;
				/************************************************************
				Menu > Results > Export > Selected Search Number
				*************************************************************/
                case MNU_RES_EXPORT_SEL: case MNU_RES_EXPORT_ALL: case MNU_RES_EXPORT_CHEAT:
                {
					ExportResults(LOWORD(wParam));
				} break;
			    case MNU_CS_INPUT_HEX: case MNU_CS_INPUT_DEC: case MNU_CS_INPUT_FLOAT:
                {
                    Settings.CS.NumBase = GetMenuItemData(hMenu, LOWORD(wParam));
                    Settings.CS.NumBaseId = LOWORD(wParam);
                    SetMenuState(hMenu, MNU_CS_INPUT_HEX, MFS_UNCHECKED);
                    SetMenuState(hMenu, MNU_CS_INPUT_DEC, MFS_UNCHECKED);
                    SetMenuState(hMenu, MNU_CS_INPUT_FLOAT, MFS_UNCHECKED);
                    SetMenuState(hMenu, LOWORD(wParam), MFS_CHECKED);
                } break;
                case MNU_CS_UNDO:
                {
					SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], msg, wParam, lParam);
				} break;
				case MNU_CS_FILE_MODE:
				{
					if (GetMenuState(hMenu, MNU_CS_FILE_MODE, MF_BYCOMMAND) & MFS_CHECKED) {
						SetMenuState(hMenu, MNU_CS_FILE_MODE, MFS_UNCHECKED);
						Settings.CS.FileMode = MFS_UNCHECKED;
					} else {
						SetMenuState(hMenu, MNU_CS_FILE_MODE, MFS_CHECKED);
						Settings.CS.FileMode = MFS_CHECKED;
					}
				} break;
                case MNU_LOAD_SEARCH:
                {
					SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], msg, wParam, lParam);
				} break;
				case MNU_HALT: case MNU_RESUME:
				{
					if (SysHalt((LOWORD(wParam) == MNU_HALT))) {
						EnableMenuItem(hMenu, MNU_HALT, MF_BYCOMMAND|MF_GRAYED);
						EnableMenuItem(hMenu, MNU_RESUME, MF_BYCOMMAND|MF_ENABLED);
					} else {
						EnableMenuItem(hMenu, MNU_HALT, MF_BYCOMMAND|MF_ENABLED);
						EnableMenuItem(hMenu, MNU_RESUME, MF_BYCOMMAND|MF_GRAYED);
					}
				} break;
				case MNU_RECONNECT:
				{
					EnableWindow(DlgInfo.TabDlgs[CODE_SEARCH_TAB], TRUE);
					EnableWindow(hwndTabCtrl, TRUE);
					ClientReconnect();
				} break;
                case MNU_MEM_SHOW_BYTES: case MNU_MEM_SHOW_SHORTS: case MNU_MEM_SHOW_WORDS:
                {
                    Settings.MemEdit.EditSize = GetMenuItemData(hMenu, LOWORD(wParam));
                    Settings.MemEdit.EditSizeId = LOWORD(wParam);
                    SetMenuState(hMenu, MNU_MEM_SHOW_BYTES, MFS_UNCHECKED);
                    SetMenuState(hMenu, MNU_MEM_SHOW_SHORTS, MFS_UNCHECKED);
                    SetMenuState(hMenu, MNU_MEM_SHOW_WORDS, MFS_UNCHECKED);
                    SetMenuState(hMenu, LOWORD(wParam), MFS_CHECKED);
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], msg, wParam, lParam);
				} break;
				case MNU_MEM_GOTO:
				{
					if (DlgInfo.ActiveTab != MEMORY_EDITOR_TAB) { break; }
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_ENDEDIT, 0);
					SendMessage(DlgInfo.TabDlgs[MEMORY_EDITOR_TAB], WM_COMMAND, LSV_MEM_EDIT_BEGINEDIT, 0);
				} break;
                case MNU_EXIT:
                {
					SaveSettings();
					DestroyWindow(hwnd);
				} break;
			}
		} break;
		case WM_SIZE:
		{
		} break;
		case WM_CLOSE:
		{
            SaveSettings();
            if (!ntpbShutdown()) { break; }
            DestroyWindow(hwnd);
		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		default:
			return FALSE;
	}
	return FALSE;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;

   memset(&DlgInfo, 0, sizeof(HWND_WNDPROC_INFO));
   DlgInfo.ActiveTab = 0;
   DlgInfo.Instance = hInstance;
   DlgInfo.Main = CreateDialog(hInstance,MAKEINTRESOURCE(PS2CC_DLG),HWND_DESKTOP, MainWndProc);
   sprintf(ErrTxt, "%u", GetLastError());
   if (!DlgInfo.Main) { MessageBox(NULL,ErrTxt, "Error",MB_OK); }

	// Create our controls
    INITCOMMONCONTROLSEX blah;
    blah.dwSize = sizeof(INITCOMMONCONTROLSEX);
    blah.dwICC = -1;
    InitCommonControlsEx(&blah);

   HACCEL KeyAccel = LoadAccelerators(hInstance, "PS2CC_ACCEL");
   if (!KeyAccel) {
	   sprintf(ErrTxt, "%u", GetLastError());
	   MessageBox(NULL,ErrTxt, "Error",MB_OK);
   }

	ShowWindow(DlgInfo.Main,SW_SHOW);

	extern HANDLE clientThid;
	clientThid = CreateThread(NULL, 0, clientThread, NULL, 0, NULL); // no stack, 1MB by default

	// API message loop
	while (GetMessage(&msg,NULL,0,0)) {
		if ((!IsDialogMessage(DlgInfo.TabDlgs[DlgInfo.ActiveTab], &msg)) && (!TranslateAccelerator(DlgInfo.Main, KeyAccel, &msg)) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

/****************************************************************************
Update Status Bar
*****************************************************************************/
int UpdateStatusBar(const char *StatusText, int PartNum, int Flags)
{
	HWND hwndStatusBar = GetDlgItem(DlgInfo.Main, NTPB_STATUS_BAR);
	if (hwndStatusBar) {
		SendMessage(hwndStatusBar, SB_SETTEXT, PartNum|Flags, (LPARAM)StatusText);
		UpdateWindow(DlgInfo.Main);
	}
	return 0;
}

/****************************************************************************
Update Progress Bar
*****************************************************************************/
int UpdateProgressBar(unsigned int Message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndProgressBar = GetDlgItem(DlgInfo.Main, DUMPSTATE_PRB);
	if (hwndProgressBar) {
		SendMessage(hwndProgressBar, Message, wParam, lParam);
		UpdateWindow(DlgInfo.Main);
	}
	return 0;
}
