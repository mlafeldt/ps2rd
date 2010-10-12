/****************************************************************************
Memory Library (threaded)

This is where the all important PC-PS2 comms code should reside. I've reworked
the threading from Jimmi's Win32 client in a way I hope he'll find useful, and
hopefully not too buggy. Seems workable so far.

The basic idea is to leave the client thread run the entire time the program
is to handle netlog and reconnect automatically (or try to) when disconnected
(ConnectClient set to 0).

The SendReceiveThread is used as SendRemoteCmd and rcvData/rcvReply in one. It's
not always threaded though. When dumping memory, it's threaded. If we're just
updating codes, there's virtually no wait time so it's called as a function and
works the same way. the NTPB_IO struct I setup provides the input when calling
or threading SendReceiveThread. I might update it later to include an output
buffer to be used in place of writing to file on demand. This would be useful
for grabbing a few bytes at a time--like for the display in a memory editor.
*****************************************************************************/

#include "ps2cc.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_TCP_PORT 4234
#define SERVER_UDP_PORT 4244
//#define SERVER_IP		"192.168.0.80"

static int main_socket = -1;
static int udp_socket = -1;

char pktbuffer[65536];

static int ClientConnected = 0;
static int haltState = 0;

//unsigned int dump_size;
//unsigned int dump_wpos;
//FILE *fh_dump;
FILE *NetLog;

// NTPB header magic
#define ntpb_MagicSize  6
const unsigned char *ntpb_hdrMagic = "\xff\x00NTPB";
#define ntpb_hdrSize  	10

#define NTPBCMD_SEND_DUMP 					0x300
#define NTPBCMD_END_TRANSMIT				0xffff

#define REMOTE_CMD_NONE						0x000
#define REMOTE_CMD_DUMP						0x100
#define REMOTE_CMD_HALT						0x201
#define REMOTE_CMD_RESUME					0x202
#define REMOTE_CMD_ADDMEMPATCHES			0x501
#define REMOTE_CMD_CLEARMEMPATCHES			0x502
#define REMOTE_CMD_ADDRAWCODES				0x601
#define REMOTE_CMD_CLEARRAWCODES			0x602

#define MAX_PATCHES 	256
#define MAX_CODES 	256

static int remote_cmd;

HANDLE clientThid;

WSADATA *WsaData;

typedef struct _NTPB_IO {
	int RemoteCMD;		//remote cmd to send - data thread keeps the global updated, hopefully
	int cmdSize;		//size of cmd to be sent
	unsigned char cmdBuf[64];	//cmd buffer
	int NotifyId;		//ID of the command to be sent to a window (NotifyHwnd) when done
	HWND NotifyHwnd;	//window handle to send a message to when finished, if desired.
	FILE *fh_dump; //dump file
	unsigned char *outBuffer; //buffer for returned data
} NTPB_IO;


/****************************************************************************
// WSADATA *InitWS2(void)
// Routine Description:
//
// Calls WSAStartup, makes sure we have a good version of WinSock2
//
//
// Return Value:
//  A pointer to a WSADATA structure - WinSock 2 DLL successfully started up
//  NULL - Error starting up WinSock 2 DLL.
//
*****************************************************************************/

WSADATA *InitWS2(void)
{
    int           Error;              // catches return value of WSAStartup
    WORD          VersionRequested;   // passed to WSAStartup
    static WSADATA       WsaData;            // receives data from WSAStartup
    BOOL          ReturnValue = TRUE; // return value flag

    // Start WinSock 2.  If it fails, we don't need to call
    // WSACleanup().
    VersionRequested = MAKEWORD(2, 0);
    Error = WSAStartup(VersionRequested, &WsaData);
    if (Error) {
        MessageBox(GetActiveWindow(),
                   "Could not find high enough version of WinSock",
                   "Error", MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
        ReturnValue = FALSE;
    } else {

        // Now confirm that the WinSock 2 DLL supports the exact version
        // we want. If not, make sure to call WSACleanup().
        if (LOBYTE(WsaData.wVersion) != 2) {
            MessageBox(GetActiveWindow(),
                       "Could not find the correct version of WinSock",
                       "Error",  MB_OK | MB_ICONSTOP | MB_SETFOREGROUND);
            WSACleanup();
            ReturnValue = FALSE;
        }
    }
    if (ReturnValue)
        return &WsaData;
    return(NULL);

} // InitWS2()

/****************************************************************************/
int check_ntpb_header(void *buf) // sanity check to see if the packet have the format we expect
{
	int i;
	unsigned char *pbuf = (unsigned char *)buf;

	for (i=0; i<ntpb_MagicSize; i++) {
		if (pbuf[i] != ntpb_hdrMagic[i])
			break;
	}

	if (i == ntpb_MagicSize)
		return 1;

	return 0;
}

/****************************************************************************
Send/Receive Data Thread

Can also be called as a function. lParam is a pointer to an NTPB_IO struct.
*****************************************************************************/
DWORD WINAPI SendReceiveThread(LPVOID lpParam) // retrieving datas sent by server
{
	unsigned int dump_size, dump_wpos = 0;
	int rcvSize, sndSize, packetSize, ntpbpktSize, ntpbCmd, ln, recv_size;
	char *pbuf;
	int endTransmit = 0;
	NTPB_IO *cmdInfo = (NTPB_IO*)lpParam;

	//Send Remote CMD
	memcpy(&pktbuffer[0], ntpb_hdrMagic, ntpb_MagicSize); //copying NTPB Magic
	*((unsigned short *)&pktbuffer[ntpb_MagicSize]) = cmdInfo->cmdSize;
	*((unsigned short *)&pktbuffer[ntpb_MagicSize+2]) = cmdInfo->RemoteCMD;

	if ((!ClientConnected) || (remote_cmd != REMOTE_CMD_NONE)) {
		MessageBox(GetActiveWindow(),"Client busy or not connected. Please check your request and hack again.","NTPB Client Error",MB_ICONERROR | MB_OK);
		goto error;
	}
	remote_cmd = cmdInfo->RemoteCMD;

	if ((cmdInfo->cmdBuf) && (cmdInfo->cmdSize > 0)) {
		memcpy(&pktbuffer[ntpb_hdrSize], cmdInfo->cmdBuf, cmdInfo->cmdSize);
	}

	ntpbpktSize = ntpb_hdrSize + cmdInfo->cmdSize;

	switch(remote_cmd)
	{
		case REMOTE_CMD_DUMP:
		{
			dump_size = *((unsigned int *)&pktbuffer[ntpb_hdrSize + 4]) - *((unsigned int *)&pktbuffer[ntpb_hdrSize]);
		} break;
		case REMOTE_CMD_NONE: goto error;
	}

	// send the ntpb packet
	sndSize = send(main_socket, &pktbuffer[0], ntpbpktSize, 0);
	if (sndSize <= 0) {
		MessageBox(GetActiveWindow(),"Error: send failed !","ntpbclient",MB_ICONERROR | MB_OK);
		goto error;
	}
	//I'm guessing the server sends a packet back just to acknowledge the request?
	//This looked redundant, but it wasn't working without it.
	rcvSize = recv(main_socket, &pktbuffer[0], sizeof(pktbuffer), 0);
	if (rcvSize <= 0) {
		MessageBox(GetActiveWindow(),"Error: recv failed !","ntpbclient",MB_ICONERROR | MB_OK);
		goto error;
	}


	//Receive Data/Reply
	while (1) {

		pbuf = (char *)&pktbuffer[0];

		// receive the first packet
		rcvSize = recv(main_socket, &pktbuffer[0], sizeof(pktbuffer), 0);
		if (rcvSize < 0) {
			MessageBox(GetActiveWindow(),"Error: recv failed !","ntpbclient",MB_ICONERROR | MB_OK);
			goto error;
		}

		// packet sanity check
		if (!check_ntpb_header(pbuf)) {
			MessageBox(GetActiveWindow(),"Error: not ntpb packet !","ntpbclient",MB_ICONERROR | MB_OK);
			goto error;
		}

		ntpbpktSize = *((unsigned short *)&pbuf[6]);
		packetSize = ntpbpktSize + ntpb_hdrSize;

		recv_size = rcvSize;

		// fragmented packet handling
		while (recv_size < packetSize) {
			rcvSize = recv(main_socket, &pktbuffer[recv_size], sizeof(pktbuffer) - recv_size, 0);
			if (rcvSize < 0) {
				MessageBox(GetActiveWindow(),"Error: recv failed !","ntpbclient",MB_ICONERROR | MB_OK);
				goto error;
			}
			else {
				recv_size += rcvSize;
			}
		}

		// parses packet
		if (check_ntpb_header(pbuf)) {
			ntpbCmd = *((unsigned short *)&pbuf[8]);

			switch(ntpbCmd) { // treat Client Request here

				case NTPBCMD_SEND_DUMP:
					if ((dump_wpos + ntpbpktSize) > dump_size) {
						MessageBox(GetActiveWindow(),"Error: dump size exeeded !","ntpbclient",MB_ICONERROR | MB_OK);
						goto error;
					}

					if (cmdInfo->fh_dump) { fwrite(&pktbuffer[ntpb_hdrSize], 1, ntpbpktSize, cmdInfo->fh_dump); }
					else { memcpy(cmdInfo->outBuffer, &pktbuffer[ntpb_hdrSize], ntpbpktSize); }
					dump_wpos += ntpbpktSize;

					// stepping progress bar
					UpdateProgressBar(PBM_STEPIT, 0, 0);
					break;

				case NTPBCMD_END_TRANSMIT:
					Sleep(100);
					if(cmdInfo->fh_dump) fclose(cmdInfo->fh_dump);
					endTransmit = 1;
					break;
			}

			*((unsigned short *)&pktbuffer[ntpb_hdrSize]) = 1;
			*((unsigned short *)&pktbuffer[6]) = 0;
			packetSize = ntpb_hdrSize + 2;

			// send the response packet
			sndSize = send(main_socket, &pktbuffer[0], packetSize, 0);
			if (sndSize <= 0) {
				MessageBox(GetActiveWindow(),"Error: send failed !","ntpbclient",MB_ICONERROR | MB_OK);
				goto error;
			}

			if (endTransmit)
				break;
		}
	}

	// resetting progress bar
	UpdateProgressBar(PBM_SETPOS, 0, 0);
	UpdateStatusBar("Idle", 0, 0);

	remote_cmd = REMOTE_CMD_NONE;
	//Notify main thread (1 on success, 0 on failure)
	if (cmdInfo->NotifyId) SendMessage(cmdInfo->NotifyHwnd, WM_COMMAND, cmdInfo->NotifyId, 1);
	return 1;

error:
	remote_cmd = REMOTE_CMD_NONE;
	UpdateProgressBar(PBM_SETPOS, 0, 0);
	UpdateStatusBar("Client disconnected...", 0, 0);
	if (main_socket) closesocket(main_socket);
	if (cmdInfo->fh_dump) fclose(cmdInfo->fh_dump);
	ClientConnected = 0;
	ntpbShutdown();
	clientThid = CreateThread(NULL, 0, clientThread, NULL, 0, NULL); // no stack, 1MB by default
	//Notify main thread (1 on success, 0 on failure)
	if (cmdInfo->NotifyId) SendMessage(cmdInfo->NotifyHwnd, WM_COMMAND, cmdInfo->NotifyId, 0);
	return 0;
}
/****************************************************************************
Client Thread

Netlog prints to netlog.txt, which stays open the whole time the app is running.
It gets cleared each time it's opened, but it should serve it's purpose.
*****************************************************************************/
DWORD WINAPI clientThread(LPVOID lpParam)
{

	int r, tcp_socket, peerlen, UDPBound;
	struct sockaddr_in peer, udp_peer;
	char recvbuffer[65536];
	fd_set fd;
	char netlogbuffer[1024];

	// Init WSA
	WsaData = InitWS2();
	if (WsaData == NULL) {
		MessageBox(GetActiveWindow(), "Failed to initialize WsaData.", "Error", MB_OK);
		return 0;
	}

	if (main_socket != -1)
		closesocket(main_socket);

	peer.sin_family = AF_INET;
	peer.sin_port = htons(SERVER_TCP_PORT);
	peer.sin_addr.s_addr = inet_addr(Settings.ServerIp);
	ClientConnected = 0;

	//UDP
	udp_peer.sin_family = AF_INET;
	udp_peer.sin_port = htons(SERVER_UDP_PORT);
	udp_peer.sin_addr.s_addr = htonl(INADDR_ANY);
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	UDPBound = 0;
	if (udp_socket) {
  		if (bind(udp_socket, (struct sockaddr *)&udp_peer, sizeof(struct sockaddr)) == 0) { UDPBound = 1; }
	}
	FD_ZERO(&fd);
	peerlen = sizeof(udp_peer);

	//log file setup
	char nlFileName[MAX_PATH];
    if (GetModuleFileName(NULL,nlFileName,sizeof(nlFileName)) ) {
        char *fndchr = strrchr(nlFileName,'\\');
        *(fndchr + 1) = '\0';
        strcat(nlFileName, "netlog.txt");
	} else { sprintf(nlFileName,"netlog.txt"); }
	NetLog = fopen(nlFileName, "wt");


	while (1) {
		if (!ClientConnected) {
			UpdateStatusBar("Contacting PS2 Server...", 0, 0);
			tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (tcp_socket < 0) { goto socket_error; }

			r = connect(tcp_socket, (struct sockaddr*)&peer, sizeof(peer));
			if (r < 0) { goto socket_error; }

			UpdateStatusBar("Client connected...", 0, 0);
			main_socket = tcp_socket;
			ClientConnected = 1;

			continue;
socket_error:
			closesocket(tcp_socket);
			Sleep(500);
			continue;
		}
		if ((UDPBound) && (NetLog)) {
			FD_SET(udp_socket, &fd);

  			select(FD_SETSIZE, &fd, NULL, NULL, NULL);

			memset(netlogbuffer, 0, sizeof(netlogbuffer));

			recvfrom(udp_socket, netlogbuffer, sizeof(netlogbuffer), 0, (struct sockaddr *)&udp_peer, &peerlen);
			if (strlen(netlogbuffer)) {
				strcat(netlogbuffer, "\r\n");
				fwrite(netlogbuffer, 1, strlen(netlogbuffer), NetLog);
			}
		}
	}
	return 0;
}

/****************************************************************************
NTPB Client Shutdown

Call this before exiting the program to shutdown the client thread and close
sockets, netlog, whatever.
*****************************************************************************/
int ntpbShutdown()
{
	int ret = 0;
	if (haltState) {
		sprintf(ErrTxt, "PS2 is still halted. Are you sure you want to exit without resuming?");
		ret = 1;
	}
	switch (remote_cmd)
	{
		case REMOTE_CMD_DUMP:
		{
			sprintf(ErrTxt, "Please wait for dump to finish and request application shutdown again.");
			ret = 1;
		} break;
		case REMOTE_CMD_HALT: case REMOTE_CMD_RESUME:
		case REMOTE_CMD_ADDMEMPATCHES: case REMOTE_CMD_CLEARMEMPATCHES:
		case REMOTE_CMD_ADDRAWCODES: case REMOTE_CMD_CLEARRAWCODES:
		{
			sprintf(ErrTxt, "You appear to be updating cheats or trying to halt or resume. \r\nPlease wait for command to finish and request shutdown again.");
			ret = 1;
		} break;
	}
	if (ret)
	{
		ret = MessageBox(GetActiveWindow(),ErrTxt,"NTPB Client Error",MB_ICONERROR | MB_ABORTRETRYIGNORE);
		switch (ret) {
			case IDABORT:
				return 0;
			case IDRETRY:
			{
				Sleep(1000);
				return ntpbShutdown();
			}
			case IDIGNORE: break;
		}
	}

	TerminateThread(clientThid, 0);
	closesocket(main_socket);
	closesocket(udp_socket);
	WSACleanup();
	fclose(NetLog);
	return 1;
}

/****************************************************************************
DumpRAM
*****************************************************************************/
extern char ErrTxt[1000];

int DumpRAM(char *dump_file, unsigned int dump_start, unsigned int dump_end, int NotifyId, HWND NotifyHwnd)
{
	static NTPB_IO cmdInfo; memset(&cmdInfo, 0, sizeof(NTPB_IO));
	unsigned int dump_size;

	//checking address range to determine course of action
	cmdInfo.RemoteCMD = REMOTE_CMD_NONE;
	if (dump_start > dump_end) {
		sprintf(ErrTxt, "Search area start (%08X) is higher thand end (%08X). THINK about it. (DumpRAM)", dump_start, dump_end);
		return 0;
	}
/* The checks should be reinstated at some point
	if ((dump_start >= 00100000) && (dump_end <= 0x02000000)) { //EE Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPEE;
	} else if ((dump_start >= 0) && (dump_end <= 0x00200000)) { //IOP Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPIOP;
	} else if ((dump_start >= 0x80000000) && (dump_end <= 0x82000000)) { //Kernel Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPKERNEL;
	} else if ((dump_start >= 0x70000000) && (dump_end <= 0x70004000)) { //ScratchPad Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPSCRATCHPAD;
	}
*/
	cmdInfo.RemoteCMD = REMOTE_CMD_DUMP;
	//Check that we're trying to dump a valid area
	if (cmdInfo.RemoteCMD == REMOTE_CMD_NONE) {
		sprintf(ErrTxt, "Invalid search/dump area specifed. (DumpRAM)");
		return 0;
	}

	// create the dump file
	cmdInfo.fh_dump = fopen(dump_file, "wb");
	if (!cmdInfo.fh_dump) {
		sprintf(ErrTxt, "Failed to create dump file! (DumpRAM");
		return 0;
	}

	// fill remote cmd buffer
	*((unsigned int *)&cmdInfo.cmdBuf[0]) = dump_start;
	*((unsigned int *)&cmdInfo.cmdBuf[4]) = dump_end;
	cmdInfo.cmdSize = 8;

	cmdInfo.NotifyId = NotifyId;
	cmdInfo.NotifyHwnd = NotifyHwnd;

	//init progress bar
	dump_size = dump_end - dump_start;
	UpdateProgressBar(PBM_SETRANGE, 0, MAKELPARAM(0, dump_size/8192));
	UpdateProgressBar(PBM_SETSTEP, 1, 0);
	UpdateStatusBar("Dumping Memory...", 0, 0);

	// send remote cmd
	HANDLE ioThread = CreateThread(NULL, 0, SendReceiveThread, &cmdInfo, 0, NULL); // no stack, 1MB by default

	return 1;
}

/****************************************************************************
Activate Cheats
*****************************************************************************/
int ActivateCheats(unsigned char *codes, int numcodes)
{
	static NTPB_IO cmdInfo; memset(&cmdInfo, 0, sizeof(NTPB_IO));
	int codestosend, numcodes_sent;

	UpdateStatusBar("Updating Cheats...", 0, 0);
	cmdInfo.RemoteCMD = REMOTE_CMD_ADDMEMPATCHES;

	numcodes_sent = 0;
	while ((numcodes_sent < numcodes) && (numcodes_sent < 256)) {

		codestosend = numcodes - numcodes_sent;

		if (codestosend > 6)
			codestosend = 6;

		*((unsigned int *)&cmdInfo.cmdBuf[0]) = codestosend;
		memcpy(&cmdInfo.cmdBuf[4], &codes[(numcodes_sent * 8) + 4], codestosend * 8);
		cmdInfo.cmdSize = (codestosend * 8) + 4;

		if ((!ClientConnected) || (remote_cmd != REMOTE_CMD_NONE)) {
			MessageBox(GetActiveWindow(),"Client busy or not connected. Please check your request and hack again. (ActivateCheats)","NTPB Client Error",MB_ICONERROR | MB_OK);
			return 0;
		}

		if(!SendReceiveThread(&cmdInfo)) { return 0; }

		numcodes_sent += codestosend;
	}

	UpdateStatusBar("Cheats Updated", 0, 0);
	return 1;
}

/****************************************************************************
DeActivate Cheats
*****************************************************************************/
int DeActivateCheats()
{
	static NTPB_IO cmdInfo; memset(&cmdInfo, 0, sizeof(NTPB_IO));

	UpdateStatusBar("Updating Cheats...", 0, 0);
	cmdInfo.RemoteCMD = REMOTE_CMD_CLEARMEMPATCHES;
	cmdInfo.cmdSize = 0;
	if(!SendReceiveThread(&cmdInfo)) { return 0; }
	UpdateStatusBar("Cheats Updated", 0, 0);

	return 1;
}

/****************************************************************************
Halt/Resume
*****************************************************************************/
int SysHalt(int halt)
{
	static NTPB_IO cmdInfo; memset(&cmdInfo, 0, sizeof(NTPB_IO));
	if (halt == haltState) { return haltState; }
	UpdateStatusBar("Sending CMD...", 0, 0);
	haltState ^= 1;
	cmdInfo.RemoteCMD = (haltState)? REMOTE_CMD_HALT:REMOTE_CMD_RESUME;
	cmdInfo.cmdSize = 0;
	if(!SendReceiveThread(&cmdInfo)) {
		haltState = 0;
		return 0;
	}
	if (haltState) { UpdateStatusBar("Halted", 0, 0); }
	else { UpdateStatusBar("Resumed", 0, 0); }
	return haltState;
}

/****************************************************************************
ReadMem - Read memory into a buffer
*****************************************************************************/
int ReadMem(unsigned char *read_buffer, unsigned int dump_start, unsigned int dump_end)
{
	static NTPB_IO cmdInfo; memset(&cmdInfo, 0, sizeof(NTPB_IO));
	unsigned int dump_size;

	//checking address range to determine course of action
	cmdInfo.RemoteCMD = REMOTE_CMD_NONE;
	if (dump_start > dump_end) {
		sprintf(ErrTxt, "Read area start (%08X) is higher than end (%08X).(ReadMem)", dump_start, dump_end);
		return 0;
	}
/* These checks should be reinstated at some point
	if ((dump_start >= 00100000) && (dump_end <= 0x02000000)) { //EE Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPEE;
	} else if ((dump_start >= 0) && (dump_end <= 0x00200000)) { //IOP Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPIOP;
	} else if ((dump_start >= 0x80000000) && (dump_end <= 0x82000000)) { //Kernel Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPKERNEL;
	} else if ((dump_start >= 0x70000000) && (dump_end <= 0x70004000)) { //ScratchPad Dump
		cmdInfo.RemoteCMD = REMOTE_CMD_DUMPSCRATCHPAD;
	}
*/
	cmdInfo.RemoteCMD = REMOTE_CMD_DUMP;
	//Check that we're trying to dump a valid area
	if (cmdInfo.RemoteCMD == REMOTE_CMD_NONE) {
		sprintf(ErrTxt, "Memory area not recognized. (ReadMem)");
		return 0;
	}

	// fill remote cmd buffer
	*((unsigned int *)&cmdInfo.cmdBuf[0]) = dump_start;
	*((unsigned int *)&cmdInfo.cmdBuf[4]) = dump_end;
	cmdInfo.cmdSize = 8;
	cmdInfo.outBuffer = read_buffer;
	cmdInfo.NotifyId = 0;

	//init progress bar
	dump_size = dump_end - dump_start;
	UpdateProgressBar(PBM_SETRANGE, 0, MAKELPARAM(0, dump_size/8192));
	UpdateProgressBar(PBM_SETSTEP, 1, 0);
	UpdateStatusBar("Dumping Memory...", 0, 0);

	// send remote cmd
	if(!SendReceiveThread(&cmdInfo)) { return 0; }

	return 1;
}

/****************************************************************************
Client Reconnect
*****************************************************************************/
int ClientReconnect()
{
	remote_cmd = REMOTE_CMD_NONE;
	UpdateProgressBar(PBM_SETPOS, 0, 0);
	UpdateStatusBar("Client disconnected...", 0, 0);
	if (main_socket) closesocket(main_socket);
	ClientConnected = 0;
	if (!ntpbShutdown()) { return 0; }
	clientThid = CreateThread(NULL, 0, clientThread, NULL, 0, NULL); // no stack, 1MB by default
	return 1;
}
