/****************************************************************************
PS2CC API Library

Collection of functions more specific to this app. API and globals in use.
*****************************************************************************/
#include "ps2cc.h"
#include "ps2cc_gui.h"

/****************************************************************************
Value editbox procedure - forces a textbox to hex/dec/whatever input
-check length of current text on maxlength change???
*****************************************************************************/
LRESULT CALLBACK ValueEditBoxHandler (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC wpOriginalProc = GetSubclassProc(GetDlgCtrlID(hwnd));
    switch (message)
    {
        case WM_CHAR:
        {
        	if ((wParam == VK_BACK) || (wParam == 24) || (wParam == 3) || (wParam == 22) || (wParam == VK_TAB)) { break; } //cut/copy/paste/backspace
            if (wParam == 1) { SendMessage(hwnd, EM_SETSEL, 0, -1); } //select all
        	switch (GetDlgCtrlID(hwnd))
        	{
        		case SEARCH_AREA_LOW_TXT: case SEARCH_AREA_HIGH_TXT: case EX_VALUE_TXT:
        		case ACTIVE_RES_ADDR_TXT: case ACTIVE_RES_VALUE_TXT:
        		case ACTIVE_RES_EDIT_TXT:
        		{
                	wParam = FilterHexChar(wParam);
                } break;
                case SEARCH_VALUE1_TXT: case SEARCH_VALUE2_TXT:
                {
					CODE_SEARCH_VARS Search; memset(&Search,0,sizeof(CODE_SEARCH_VARS));
            		HWND hwndSearchType = GetDlgItem(DlgInfo.TabDlgs[CODE_SEARCH_TAB], SEARCH_TYPE_CMB);
            		Search.Type = SendMessage(hwndSearchType,CB_GETITEMDATA,SendMessage(hwndSearchType,CB_GETCURSEL,0,0),0);
            		if ((Search.Type & SEARCH_KNOWN_WILD) && (wParam == 42)) { break; } //asterisk
            		switch ((Search.Type & (SEARCH_KNOWN_WILD|SEARCH_EQUAL_NUM_BITS|SEARCH_NEQUAL_TO_BITS|SEARCH_NEQUAL_BY_BITS|SEARCH_BITS_ANY|SEARCH_BITS_ALL)) ?
            		BASE_HEX : Settings.CS.NumBase)
            		{
            		    case BASE_HEX: { wParam = FilterHexChar(wParam); } break;
            		    case BASE_DEC: { wParam = ((isdigit(wParam))|| (wParam == '-')) ? wParam : 0; } break;
            		    case BASE_FLOAT: { wParam = (isdigit(wParam) || (wParam == '.') || (wParam == '-')) ? wParam : 0; } break;
            		}
                } break;
            }
        } break;
        case WM_PASTE:
        {
            char txtInput[20], txtInput2[20];
            GetWindowText(hwnd, txtInput, sizeof(txtInput));
            if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
            GetWindowText(hwnd, txtInput2, sizeof(txtInput2));
            if ((!isHexWindow(hwnd)) || (strlen(txtInput2) > SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0))) { SetWindowText(hwnd, txtInput); }
        } return 0;
        case WM_KEYDOWN:
        {
        	switch (GetDlgCtrlID(hwnd))
        	{
                case EX_VALUE_TXT:
                {
            		if (wParam == VK_RETURN) { SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_ENDEDIT, 1); }
            		if (wParam == VK_ESCAPE) { SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_ENDEDIT, 0); }
            	} break;
            	case ACTIVE_RES_EDIT_TXT:
            	{
            		if (wParam == VK_RETURN) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_ENDEDIT, 1); }
            		if (wParam == VK_ESCAPE) { SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_ENDEDIT, 0); }
				} break;
            }
        } break;
        case WM_KILLFOCUS:
        {
        	switch (GetDlgCtrlID(hwnd))
        	{
                case EX_VALUE_TXT:
                {
            		SendMessage(DlgInfo.TabDlgs[CODE_SEARCH_TAB], WM_COMMAND, LSV_CS_ENDEDIT, 0);
            	} return 0;
            	case ACTIVE_RES_EDIT_TXT:
            	{
            		SendMessage(DlgInfo.TabDlgs[SEARCH_RESULTS_TAB], WM_COMMAND, LSV_ACTIVE_ENDEDIT, 0);
				} break;
            }
        } break;
   }
   if (wpOriginalProc) { return CallWindowProc (wpOriginalProc, hwnd, message, wParam, lParam); }
   else { return DefWindowProc (hwnd, message, wParam, lParam); }
}

/****************************************************************************
Get Subclassed control handle - Take Control ID and find the original HWND
*****************************************************************************/
WNDPROC GetSubclassProc(int ControlId)
{
	int i;
	for (i = 0; i < MAX_SUBCLASSES; i++)
	{
		if (DlgInfo.SubclassIds[i] == ControlId) { return DlgInfo.SubclassProcs[i]; }
	}
	return 0;
}

/****************************************************************************
Set Subclassed control handle - Take Control ID and find an empty array slot for the original HWND
*****************************************************************************/
int SetSubclassProc(WNDPROC ControlProc, int ControlId)
{
	int i;
	for (i = 0; i < MAX_SUBCLASSES; i++)
	{
		if (DlgInfo.SubclassIds[i] == 0) {
			DlgInfo.SubclassIds[i] = ControlId;
			DlgInfo.SubclassProcs[i] = ControlProc;
			return i;
		}
	}
	MessageBox(NULL, "Out of Subclass IDs", "Error", MB_OK);
	return -1;
}

/****************************************************************************
LoadSettings
*****************************************************************************/
int LoadSettings()
{
	MAIN_CFG Defaults;
    memset(&Defaults,0,sizeof(Defaults));
    memset(&Settings,0,sizeof(Settings));
	char CFGFile[MAX_PATH];
    if (GetModuleFileName(NULL,CFGFile,sizeof(CFGFile)) ) {
        char *fndchr = strrchr(CFGFile,'\\');
        *(fndchr + 1) = '\0';
        strcpy(Defaults.CS.DumpDir, CFGFile);
        strcat(Defaults.CS.DumpDir, "Searches\\");
        strcat(CFGFile,"ps2cc.cfg");
    } else {
        sprintf(CFGFile,"ps2cc.cfg");
        strcpy(Defaults.CS.DumpDir, "Searches\\");
    }
    Defaults.CFGVersion = 9; //increment this if settings struct or sub-struct definitions in ps2cc.h change
    sprintf(Defaults.ServerIp, "192.168.0.10");
    Defaults.ValueFontInfo = (LOGFONT){ 0, 10, 0, 0, 10, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, "Terminal"} ;
    Defaults.ValueHFont = CreateFontIndirect(&Defaults.ValueFontInfo);
	//Search Defaults
    Defaults.CS.NumBase = BASE_HEX;
    Defaults.CS.NumBaseId = MNU_CS_INPUT_HEX;
    Defaults.CS.DumpAccess = SEARCH_ACCESS_ARRAY;
    Defaults.CS.PS2Wait = BST_UNCHECKED;
    Defaults.CS.FileMode = MFS_UNCHECKED;
	//Results Defaults
    Defaults.Results.DisplayFmt = MNU_RES_SHOW_HEX;
    Defaults.Results.PageSize = 500;
    Defaults.Results.MaxResPages = 20;
    Defaults.MemEdit.EditSize = 1;
    Defaults.MemEdit.EditSizeId = MNU_MEM_SHOW_BYTES;
	if (FileExists(CFGFile)) { LoadStruct(&Settings, sizeof(MAIN_CFG), CFGFile); }
    if (Settings.CFGVersion != Defaults.CFGVersion) {
		MessageBox(NULL, "New CFG version. Settings are at default again.", "FYI", MB_OK);
		memset(&Settings,0,sizeof(Settings));
        memcpy(&Settings,&Defaults,sizeof(Defaults));
	}
    mkdir(Settings.CS.DumpDir);
    return 0;
}

/****************************************************************************
SaveSettings
*****************************************************************************/
int SaveSettings()
{
    char CFGFile[MAX_PATH];
    if (GetModuleFileName(NULL,CFGFile,sizeof(CFGFile)) ) {
        char *fndchr = strrchr(CFGFile,'\\');
        *(fndchr + 1) = '\0';
        strcat(CFGFile,"ps2cc.cfg");
    } else {
        sprintf(CFGFile,"ps2cc.cfg");
    }
    SaveStruct(&Settings, sizeof(Settings), CFGFile);
    return 0;
}

/****************************************************************************
Free shit -Everything that is or might be malloc()'d should be tested and/or
free()'d here.
*****************************************************************************/
int FreeShit()
{
    FreeRamInfo();
    if (ResultsList) { free(ResultsList); ResultsList = NULL; }
    return 0;
}

/****************************************************************************
Free RAM info -Reset the RamInfo struct.
*****************************************************************************/
int FreeRamInfo()
{
    if (RamInfo.NewRAM) { free(RamInfo.NewRAM); RamInfo.NewRAM = NULL; }
    if (RamInfo.OldRAM) { free(RamInfo.OldRAM); RamInfo.OldRAM = NULL; }
    if (RamInfo.NewFile) { fclose(RamInfo.NewFile); RamInfo.NewFile = NULL; }
    if (RamInfo.OldFile) { fclose(RamInfo.OldFile); RamInfo.OldFile = NULL; }
    if (RamInfo.Results) { free(RamInfo.Results); RamInfo.Results = NULL; }
    memset(&RamInfo.OldResultsInfo, 0, sizeof(CODE_SEARCH_RESULTS_INFO));
    memset(&RamInfo.NewResultsInfo, 0, sizeof(CODE_SEARCH_RESULTS_INFO));
    return 0;
}
