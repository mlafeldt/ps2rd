/****************************************************************************
Dialog Library

Small, secondary dialog handlers should go here.
*****************************************************************************/
#include "ps2cc.h"
#include "ps2cc_gui.h"

BOOL CALLBACK IpConfigDlg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndIpAddr = GetDlgItem(hwnd, IP_ADDR_TXT);
    HWND hwndDumpDirText = GetDlgItem(hwnd, DUMP_DIR_TXT);
    HWND hwndResPageSize = GetDlgItem(hwnd, RESULTS_PAGE_SIZE_TXT);
    HWND hwndResPageMax = GetDlgItem(hwnd, RESULTS_PAGE_MAX_TXT);
    switch(message)
    {
        case WM_INITDIALOG:
        {
			SetWindowText(hwndIpAddr, Settings.ServerIp);
			SetWindowText(hwndDumpDirText, Settings.CS.DumpDir);
			CheckDlgButton(hwnd, (Settings.CS.DumpAccess == SEARCH_ACCESS_ARRAY) ? SEARCH_ACCESS_ARRAY_OPT : SEARCH_ACCESS_FILE_OPT, BST_CHECKED);
			SetDecWindowU(hwndResPageSize, Settings.Results.PageSize);
			SetDecWindowU(hwndResPageMax, Settings.Results.MaxResPages);
        } break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case TEST_IP_CMD:
				{
					char ip[16];
					GetWindowText(hwndIpAddr, ip, sizeof(ip));
					if (!isIPAddr(ip)) {
						MessageBox(NULL, "Invalid IP", "Error", MB_OK);
						return 0;
					}
					strcpy(Settings.ServerIp, ip);
					if(ClientReconnect()) { MessageBox(NULL, "Connection Successful!", "", MB_OK); }
					else { MessageBox(NULL, "Unable to Connect", "Error", MB_OK); }
				} break;
				case SET_DUMP_DIR_CMD:
				{
					char DumpPath[MAX_PATH];
                    if (BrowseForFolder(hwnd, DumpPath)) {
                        SetWindowText(hwndDumpDirText, DumpPath);
                    }
				} break;
				case SET_OPTIONS_CMD:
				{
					char ip[16];
					GetWindowText(hwndIpAddr, ip, sizeof(ip));
					if (!isIPAddr(ip)) {
						MessageBox(NULL, "Invalid IP", "Error", MB_OK);
						return 0;
					}
					strcpy(Settings.ServerIp, ip);

					GetWindowText(hwndDumpDirText, Settings.CS.DumpDir, sizeof(Settings.CS.DumpDir));
					Settings.CS.DumpAccess = IsDlgButtonChecked(hwnd, SEARCH_ACCESS_ARRAY_OPT) ? SEARCH_ACCESS_ARRAY : SEARCH_ACCESS_FILE;
					Settings.Results.PageSize = GetDecWindow(hwndResPageSize);
					if (Settings.Results.PageSize > 10000) { Settings.Results.PageSize = 10000; }
					Settings.Results.MaxResPages = GetDecWindow(hwndResPageMax);
					if (Settings.Results.MaxResPages > 1000) { Settings.Results.MaxResPages = 1000; }
					if (Settings.Results.MaxResPages <= 0) { Settings.Results.MaxResPages = 1; }
					EndDialog(hwnd, 0);
				} break;
			}
        } break;
		case WM_CLOSE:
        {
			EndDialog(hwnd, 0);
        } break;
        default:
            return FALSE;
   }
   return TRUE;
}
