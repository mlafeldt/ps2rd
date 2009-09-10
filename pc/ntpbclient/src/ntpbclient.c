
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef _WIN32
WSADATA *WsaData;
#define _closesocket	closesocket
#else
#define _closesocket	close
#endif

#define SERVER_TCP_PORT  		4234
#define SERVER_UDP_PORT  		4244
#define SERVER_IP			"192.168.0.80"

unsigned char pktbuffer[65536];
char netlogbuffer[1024];

// NTPB header magic
#define ntpb_MagicSize  6
const unsigned char ntpb_hdrMagic[6] = {'\xff', '\x00', 'N', 'T', 'P', 'B'};
#define ntpb_hdrSize  	10

static int main_socket = -1;

// command line commands stuff
#define CMD_NONE			0
#define CMD_EXIT			1
#define CMD_CONNECT			2
#define CMD_DISCONNECT		3
#define CMD_DUMP			4
#define CMD_LOG				5
#define CMD_HELP			6

#define MEMZONE_EE 			"EE"
#define MEMZONE_IOP 		"IOP"
#define MEMZONE_KERNEL 		"Kernel"
#define MEMZONE_SCRATCHPAD 	"ScratchPad"

// Remote commands to be sent to server
#define REMOTE_CMD_NONE						0x000
#define REMOTE_CMD_DUMPEE					0x101
#define REMOTE_CMD_DUMPIOP					0x102
#define REMOTE_CMD_DUMPKERNEL				0x103
#define REMOTE_CMD_DUMPSCRATCHPAD			0x104

// commands sent in return by server
#define NTPBCMD_PRINT_EEDUMP 				0x301
#define NTPBCMD_PRINT_IOPDUMP				0x302
#define NTPBCMD_PRINT_KERNELDUMP 			0x303
#define NTPBCMD_PRINT_SCRATCHPADDUMP		0x304
#define NTPBCMD_END_TRANSMIT				0xffff

static int clientConnected = 0;

pthread_t netlog_thread_id;


//-------------------------------------------------------------- 
void printUsage(void) {
	
	printf("usage: <command> [args]\n");
	printf("ntpbclient command-line version.\n");
	printf("\n");
	printf("Available commands:\n");
	printf("\t connect\n");
	printf("\t disconnect\n");		
	printf("\t dump <memzone> <start_address> <end_address> <outfile>\n");
	printf("\t \t memzone = 'EE', 'IOP', 'Kernel', 'ScrathPad'\n");
	printf("\t log\n");				
	printf("\t help\n");			
	printf("\t quit, exit\n");
	
}

//-------------------------------------------------------------- 
#ifdef _WIN32
WSADATA *InitWS2(void) {
    int            r;        // catches return value of WSAStartup
    static WSADATA WsaData;  // receives data from WSAStartup
    int 		   ret;  	 // return value flag
    
    ret = 1;

    // Start WinSock 2.  If it fails, we don't need to call
    // WSACleanup().
    r = WSAStartup(MAKEWORD(2, 0), &WsaData);
    
    if (r) {
        printf("error: can't find high enough version of WinSock\n");
        ret = 0;
    } else {
        // Now confirm that the WinSock 2 DLL supports the exact version
        // we want. If not, make sure to call WSACleanup().
        if ((WsaData.wVersion & 0xff) != 2) {
            printf("error: can't find the correct version of WinSock\n");
            WSACleanup();
            ret = 0;
        }
    }
        
    if (ret)
        return &WsaData;
        
    return NULL;
}
#endif

//-------------------------------------------------------------- 
int clientConnect(void)
{
	int r, tcp_socket, err;
	struct sockaddr_in peer;

	peer.sin_family = AF_INET;
	peer.sin_port = htons(SERVER_TCP_PORT);
	peer.sin_addr.s_addr = inet_addr(SERVER_IP);

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		err = -1; 
		goto error;
	}

	r = connect(tcp_socket, (struct sockaddr*)&peer, sizeof(peer));
	if (r < 0) {
		err = -2;
		goto error;
	}

	main_socket = tcp_socket;

	return 0;

error:
	_closesocket(tcp_socket);

	return err;
}

//-------------------------------------------------------------- 
int clientDisconnect(void)
{
	_closesocket(main_socket);
	
	return 0;
}

//-------------------------------------------------------------- 
unsigned int HexaToDecimal(const char* pszHexa)
{
	unsigned int ret = 0, t = 0, n = 0;
	const char *c = pszHexa;

	while (*c && (n < 16)) {

		if ((*c >= '0') && (*c <= '9'))
			t = (*c - '0');
		else if((*c >= 'A') && (*c <= 'F'))
			t = (*c - 'A' + 10);
		else if((*c >= 'a') && (*c <= 'f'))
			t = (*c - 'a' + 10);
		else
			break;

		n++; ret *= 16; ret += t; c++;

		if (n >= 8)
			break;
	}

	return ret;
}

//-------------------------------------------------------------- 
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

//-------------------------------------------------------------- 
int SendRemoteCmd(int cmd, unsigned char *buf, int size)
{
	int ntpbpktSize, sndSize, rcvSize;

	memcpy(&pktbuffer[0], ntpb_hdrMagic, ntpb_MagicSize); //copying NTPB Magic
	*((unsigned short *)&pktbuffer[ntpb_MagicSize]) = size;
	*((unsigned short *)&pktbuffer[ntpb_MagicSize+2]) = cmd;

	if ((buf) && (size > 0)) {
		memcpy(&pktbuffer[ntpb_hdrSize], buf, size);
	}

	ntpbpktSize = ntpb_hdrSize + size;

	// send the ntpb packet
	sndSize = send(main_socket, (char *)&pktbuffer[0], ntpbpktSize, 0);
	if (sndSize <= 0)
		return -1;

	rcvSize = recv(main_socket, (char *)&pktbuffer[0], sizeof(pktbuffer), 0);
	if (rcvSize <= 0)
		return -2;
		
	// packet sanity check
	if (!check_ntpb_header(pktbuffer))
		return -3;
		
	// reply check
	if (*((unsigned short *)&pktbuffer[ntpb_hdrSize]) != 1)
		return -4;	

	return 1;
}

//-------------------------------------------------------------- 
int receiveDump(char *dumpfile, unsigned int dump_size) // retrieving datas sent by server
{
	int rcvSize, sndSize, packetSize, ntpbpktSize, ntpbCmd, recv_size, sizeWritten;
	unsigned int dump_wpos = 0;
	int endTransmit = 0;
	FILE *fh_dump;

	// create the dump file
	fh_dump = fopen(dumpfile, "wb");
	if (!fh_dump)
		return -100;
	
	while (1) {
		
		// receive the first packet
		rcvSize = recv(main_socket, (char *)&pktbuffer[0], sizeof(pktbuffer), 0);
		if (rcvSize < 0)
			return -1;
			
		// packet sanity check
		if (!check_ntpb_header(pktbuffer))
			return -2;

		ntpbpktSize = *((unsigned short *)&pktbuffer[6]);
		packetSize = ntpbpktSize + ntpb_hdrSize;

		recv_size = rcvSize;

		// fragmented packet handling
		while (recv_size < packetSize) {
			rcvSize = recv(main_socket, (char *)&pktbuffer[recv_size], sizeof(pktbuffer) - recv_size, 0);
			if (rcvSize < 0)
				return -1;
			else
				recv_size += rcvSize;
		}

		// parses packet
		if (check_ntpb_header(pktbuffer)) {
			ntpbCmd = *((unsigned short *)&pktbuffer[8]);

			switch(ntpbCmd) { // treat Client Request here

				case NTPBCMD_PRINT_EEDUMP:
				case NTPBCMD_PRINT_IOPDUMP:
				case NTPBCMD_PRINT_KERNELDUMP:
				case NTPBCMD_PRINT_SCRATCHPADDUMP:

					if ((dump_wpos + ntpbpktSize) > dump_size)
						return -3;

					sizeWritten = fwrite(&pktbuffer[ntpb_hdrSize], 1, ntpbpktSize, fh_dump);
					if (sizeWritten != ntpbpktSize)
						return -4;
					
					dump_wpos += sizeWritten;
					break;

				case NTPBCMD_END_TRANSMIT:
					endTransmit = 1;
					break;
			}

			*((unsigned short *)&pktbuffer[ntpb_hdrSize]) = 1;
			*((unsigned short *)&pktbuffer[6]) = 0;
			packetSize = ntpb_hdrSize + 2;

			// send the response packet
			sndSize = send(main_socket, (char *)&pktbuffer[0], packetSize, 0);
			if (sndSize <= 0)
				return -5;
				
			// catch end of dump transmission	
			if (endTransmit)
				break;
		}
	}

	fclose(fh_dump);
	
	return 1;
}

//-------------------------------------------------------------- 
void *netlogThread(void *thread_id)
{
	int udp_socket;
	struct sockaddr_in peer;
	int r;
	fd_set fd;
	FILE *fh_log;

	fh_log = fopen("netlog.log", "w");
	if (fh_log) {
		fclose(fh_log);
	}
	
	peer.sin_family = AF_INET;
	peer.sin_port = htons(SERVER_UDP_PORT);
	peer.sin_addr.s_addr = htonl(INADDR_ANY);

	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket < 0) {
		goto error;
	}

  	if (bind(udp_socket, (struct sockaddr *)&peer, sizeof(struct sockaddr)) < 0) {
		goto error;
	}
	
	FD_ZERO(&fd);

	while (1) {
		FD_SET(udp_socket, &fd);

  		select(FD_SETSIZE, &fd, NULL, NULL, NULL);

		memset(netlogbuffer, 0, sizeof(netlogbuffer));

		r = recvfrom(udp_socket, (void*)netlogbuffer, sizeof(netlogbuffer), 0, NULL, NULL);

		fh_log = fopen("netlog.log", "a");
		if (fh_log) {
			fwrite(netlogbuffer, 1, r, fh_log);
			fclose(fh_log);
		}
	}

error:	
	_closesocket(udp_socket);
	pthread_exit(thread_id);
	
	return 0;
}

//-------------------------------------------------------------- 
int printLog(void)
{
	FILE *fh_log;
	int logsize, r;
	char *buf;
	
	fh_log = fopen("netlog.log", "r");
	if (fh_log) {
		fseek(fh_log, 0, SEEK_END);
		logsize = ftell(fh_log);
		fseek(fh_log, 0, SEEK_SET);		
		if (logsize) {
			buf = malloc(logsize);
			r = fread(buf, 1, logsize, fh_log);
			printf(buf);
			free(buf);
		}
		fclose(fh_log);
	}
	
	return 0;
}

//-------------------------------------------------------------- 
int getUserInput(char *buf, int size)
{	
	printf("ntpb:>");	
	fgets(buf, size, stdin);
	
	buf[strlen(buf) - 1] = 0; // cutting newline
	
	if ((!strcmp(buf, "exit")) || (!strcmp(buf, "quit"))) {
		return CMD_EXIT;
	}
	
	if (!strcmp(buf, "connect")) {
		return CMD_CONNECT;
	}

	if (!strcmp(buf, "disconnect")) {
		return CMD_DISCONNECT;
	}
		
	if (!strncmp(buf, "dump", 4)) {
		return CMD_DUMP;
	}
	
	if (!strcmp(buf, "log")) {
		return CMD_LOG;
	}			
		
	if (!strcmp(buf, "help")) {
		return CMD_HELP;
	}			
			
	return CMD_NONE;
}

//-------------------------------------------------------------- 
int main(int argc, char **argv, char **env) {

	int r, cmd, remote_cmd;
	unsigned int dump_start, dump_end, dump_size;
	char dump_file[1024];
	unsigned char cmdBuf[16];
	char userInput[256];
	char *p = (char *)userInput;
	char *p_end;
	char str_memzone[256], str_dumpstart[256], str_dumpend[256];

#ifdef _WIN32
	// Init WSA
	WsaData = InitWS2();
	if (WsaData == NULL)
		return 0;	
#endif

	// Create netlog thread		
	pthread_create(&netlog_thread_id, NULL, netlogThread, (void *)&netlog_thread_id);
	
	// User Input loop		
	while (1) {
		
		cmd = getUserInput(userInput, sizeof(userInput));

		switch(cmd) {
			
			case CMD_NONE:
				printUsage();
			break;
			
			case CMD_EXIT:
				return 0;
			break;

			case CMD_CONNECT:
				r = clientConnect();
				if (r < 0)
					printf("failed to connect client... error: %d\n", r);
				else	
					clientConnected = 1;	
			break;					

			case CMD_DISCONNECT:
				r = clientDisconnect();
				if (r < 0)
					printf("failed to disconnect client... error: %d\n", r);	
				else	
					clientConnected = 0;	
			break;					

			case CMD_DUMP:
				if (!clientConnected) {
					printf("client not connected...\n");				
					break;
				}
			
				p = strchr(p, ' ');
				if (!p) {
					printUsage();	
					break;
				}								
				p++;
				
				p_end = strchr(p, ' ');
				if (!p_end) {
					printUsage();	
					break;
				}				
				strncpy(str_memzone, p, (p_end - p));
				str_memzone[p_end - p] = 0; 
								
				if ((strcmp(str_memzone, MEMZONE_EE)) && (strcmp(str_memzone, MEMZONE_IOP)) && \
					(strcmp(str_memzone, MEMZONE_KERNEL)) && (strcmp(str_memzone, MEMZONE_SCRATCHPAD))) {
					printUsage();	
					break;
				}				
					
				p = p_end + 1;
				p_end = strchr(p, ' ');
				if (!p_end) {
					printUsage();	
					break;
				}
				strncpy(str_dumpstart, p, (p_end - p));
				str_dumpstart[p_end - p] = 0; 
	
				p = p_end + 1;
				p_end = strchr(p, ' ');
				if (!p_end) {
					printUsage();	
					break;
				}
				strncpy(str_dumpend, p, (p_end - p));
				str_dumpend[p_end - p] = 0; 

				p = p_end + 1;
				p_end = strchr(p, '\0');
				if (!p_end) {
					printUsage();	
					break;
				}				
				strncpy(dump_file, p, (p_end - p));
				dump_file[p_end - p] = 0; 

				if (!(strlen(dump_file) > 0)) {
					printUsage();	
					break;
				}			
				
				dump_start = HexaToDecimal(str_dumpstart);
				dump_end = HexaToDecimal(str_dumpend);
				dump_size = dump_end - dump_start;
				
				if (dump_size <= 0) {
					printUsage();	
					break;					
				}						
			
				if (!strcmp(str_memzone, MEMZONE_EE)) {
					if ((dump_start < 0x00080000) || (dump_start > 0x02000000) ||
						(dump_end < 0x0080000) || (dump_end > 0x02000000)) {
						printf("invalid address range for EE dump...\n");
						break;	
					}					
					remote_cmd = REMOTE_CMD_DUMPEE;	
				}				
				if (!strcmp(str_memzone, MEMZONE_IOP)) {
					if ((dump_start < 0x00000000) || (dump_start > 0x00200000) ||
						(dump_end < 0x00000000) || (dump_end > 0x00200000)) {
						printf("invalid address range for IOP dump...\n");
						break;	
					}					
					remote_cmd = REMOTE_CMD_DUMPIOP;
				}
				if (!strcmp(str_memzone, MEMZONE_KERNEL)) {
					if ((dump_start < 0x80000000) || (dump_start > 0x82000000) ||
						(dump_end < 0x80000000) || (dump_end > 0x82000000)) {
						printf("invalid address range for Kernel dump...\n");
						break;	
					}
					remote_cmd = REMOTE_CMD_DUMPKERNEL;
				}
				if (!strcmp(str_memzone, MEMZONE_SCRATCHPAD)) {
					if ((dump_start < 0x70000000) || (dump_start > 0x70004000) ||
						(dump_end < 0x70000000) || (dump_end > 0x70004000)) {
						printf("invalid address range for ScratchPad dump...\n");
						break;	
					}					
					remote_cmd = REMOTE_CMD_DUMPSCRATCHPAD;
				}				
				
				// fill remote cmd buffer
				*((unsigned int *)&cmdBuf[0]) = dump_start;
				*((unsigned int *)&cmdBuf[4]) = dump_end;
	
				// send remote cmd
				r = SendRemoteCmd(remote_cmd, cmdBuf, 8);
				if (r < 0) {
					printf("failed to send remote command - error %d\n", r);
					break;
				}

				printf("Please wait while dumping %s @0x%08x-0x%08x to %s... ", str_memzone, dump_start, dump_end, dump_file);
														
				// receive dump	
				r = receiveDump(dump_file, dump_size);
				if (r < 0) {
					printf("failed to receive dump datas - error %d\n", r);		
					break;
				}
				
				printf("done\n");			
			break;					

			case CMD_LOG:
				printLog();
			break;				
			
			case CMD_HELP:
				printUsage();
			break;				
		}										
	}

	pthread_exit((void *)&netlog_thread_id);
		
#ifdef _WIN32
	WSACleanup();
#endif

	// End program.
	return 0;

}
