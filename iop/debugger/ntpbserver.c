
#include <loadcore.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <thevent.h>
#include <intrman.h>
#include <sifcmd.h>
#include <sifman.h>
#include <thsemap.h>
#include <errno.h>
#include <io_common.h>
#include <ps2ip.h>

#include "netlog.h"

#define MODNAME "NTPBServer"
IRX_ID(MODNAME, 1, 0);

#define SERVER_TCP_PORT  	4234
#define SERVER_UDP_PORT  	4244

#define ntpb_MagicSize  6
#define ntpb_hdrSize  	10
// NTPB header magic
const u8 *ntpb_hdrmagic = "\xff\x00NTPB";

static int client_socket = 0;
static int remote_cmd = 0;  // to hold sent remote commands

u8 cmdbuf[256];
int cmdsize;

int ntpbserver_io_sema; 	// IO semaphore
int ntpbserver_cmd_sema;	// Server CMD ready semaphore

static int server_tid;
static int rpc_tidS_0A10;

static SifRpcDataQueue_t rpc_qdS_0A10 __attribute__((aligned(64)));
static SifRpcServerData_t rpc_sdS_0A10 __attribute__((aligned(64)));

static u8 ntpbserver_rpc_buf[16384] __attribute__((aligned(64)));

static struct {		
	int rpc_func_ret;
	int ntpbserver_version;
} rpc_stat __attribute__((aligned(64)));

int (*rpc_func)(void);

void dummy(void) {}
int  rpcNTPBsendDataToClient(void);
int _rpcNTPBsendDataToClient(void *rpc_buf);
int  rpcNTPBEndReply(void);
int _rpcNTPBEndReply(void *rpc_buf);
int  rpcNTPBgetRemoteCmd(void);
int _rpcNTPBgetRemoteCmd(void *rpc_buf);

// rpc command handling array
void *rpc_funcs_array[16] = {
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)rpcNTPBgetRemoteCmd,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)dummy,
    (void *)rpcNTPBsendDataToClient,
    (void *)rpcNTPBEndReply
};

typedef struct g_sendDataParam { // size = 16384
	u16 cmd;				 //	0
	u8 buf[16378];		 	 // 2
	int size;				 // 16380
} g_sendDataParam_t;

typedef struct g_getRemoteCmdParam { // size = 64
	u16 cmd;			// 0
	u8 buf[56];			// 2
	int size;			// 58
	u16 pad;			// 62
} g_getRemoteCmdParam_t;

static u8 ntpb_buf[16384];


//-------------------------------------------------------------- 
int check_ntpb_header(void *buf) // sanity check to see if the packet have the format we expect
{
	int i;
	u8 *pbuf = (u8 *)buf;

	for (i=0; i<ntpb_MagicSize; i++) {
		if (pbuf[i] != ntpb_hdrmagic[i])
			break;
	}

	if (i == ntpb_MagicSize)
		return 1;

	return 0; 
}

//-------------------------------------------------------------- 
int recv_noblock(int sock, u8 *buf, int bsize)
{
	int r;
	fd_set rfd;
		
	FD_ZERO(&rfd);
	FD_SET(sock, &rfd);

	r = lwip_select(sock+1, &rfd, NULL, NULL, NULL);
	if (r < 0)
		return -1;		
							
	// receive the packet
	r = lwip_recv(sock, buf, bsize, 0);
	if (r < 0)
		return -2;		
    
    return r;		
}

//-------------------------------------------------------------- 
int ntpbserverEndReply(void)
{
	int rcvSize, sndSize, recv_size, sent_size, ntpbpktSize, packetSize;
	
	// Waiting io semaphore
	WaitSema(ntpbserver_io_sema);

	// Build up ntpb packet
	memcpy(&ntpb_buf[0], ntpb_hdrmagic, ntpb_MagicSize); //copying NTPB Magic
	*((u16 *)&ntpb_buf[ntpb_MagicSize]) = 0;		
	*((u16 *)&ntpb_buf[ntpb_MagicSize+2]) = 0xffff;		
	
	packetSize = ntpb_hdrSize;

	// Send ntpb packet to client
	sent_size = 0;
	// fragmented packet handling
	while (sent_size < packetSize) {
		sndSize = lwip_send(client_socket, &ntpb_buf[sent_size], packetSize - sent_size, 0);
		if (sndSize < 0)
			return -1;
			
		sent_size += sndSize;
	}
	
	// receive the response packet from client
	rcvSize = recv_noblock(client_socket, &ntpb_buf[0], sizeof(ntpb_buf));
	if (rcvSize <= 0) 
		return -1;

	ntpbpktSize = *((u16 *)&ntpb_buf[ntpb_MagicSize]);
	packetSize = ntpbpktSize + ntpb_hdrSize;

	recv_size = rcvSize;

	// fragmented packet handling
	while (recv_size < packetSize) {
		rcvSize = recv_noblock(client_socket, &ntpb_buf[recv_size], sizeof(ntpb_buf) - recv_size);
		if (rcvSize <= 0)
			return -1;
			
		recv_size += rcvSize;
	}

	// parses packet
	if (!check_ntpb_header(ntpb_buf))
		return -2;	
		
	// check client reply	
	if (*((u16 *)&ntpb_buf[ntpb_hdrSize]) != 1)
		return -2;	

	// Posting semaphore for server Thread so it's able to wait for requests again
	SignalSema(ntpbserver_cmd_sema);
	
	// posting io semaphore	
	SignalSema(ntpbserver_io_sema);

	return 0;		
}

//-------------------------------------------------------------- 
int ntpbserverSendData(u16 cmd, u8 *buf, int size)
{
	int rcvSize, sndSize, recv_size, sent_size, ntpbpktSize, packetSize;

	// Waiting io semaphore
	WaitSema(ntpbserver_io_sema);
		
	// Build up ntpb packet
	memcpy(&ntpb_buf[0], ntpb_hdrmagic, ntpb_MagicSize); //copying NTPB Magic
	*((u16 *)&ntpb_buf[ntpb_MagicSize]) = size;		
	*((u16 *)&ntpb_buf[ntpb_MagicSize+2]) = cmd;		
	
	// copy data buf to ntpb packet
	if (buf)
		memcpy(&ntpb_buf[ntpb_hdrSize], buf, size); 		
		
	ntpbpktSize = size;
	packetSize = ntpbpktSize + ntpb_hdrSize;

	// Send ntpb packet to client
	sent_size = 0;
	// fragmented packet handling
	while (sent_size < packetSize) {
		sndSize = lwip_send(client_socket, &ntpb_buf[sent_size], packetSize - sent_size, 0);
		if (sndSize < 0)
			return -1;
			
		sent_size += sndSize;
	}
	
	// receive the response packet from client
	rcvSize = recv_noblock(client_socket, &ntpb_buf[0], sizeof(ntpb_buf));
	if (rcvSize <= 0) 
		return -1;

	ntpbpktSize = *((u16 *)&ntpb_buf[ntpb_MagicSize]);
	packetSize = ntpbpktSize + ntpb_hdrSize;

	recv_size = rcvSize;

	// fragmented packet handling
	while (recv_size < packetSize) {
		rcvSize = recv_noblock(client_socket, &ntpb_buf[recv_size], sizeof(ntpb_buf) - recv_size);
		if (rcvSize <= 0)
			return -1;
			
		recv_size += rcvSize;
	}

	// parses packet
	if (!check_ntpb_header(ntpb_buf))
		return -2;	
		
	// check client reply	
	if (*((u16 *)&ntpb_buf[ntpb_hdrSize]) != 1)
		return -2;
		
	// posting io semaphore	
	SignalSema(ntpbserver_io_sema);
		
	return 0;
}

//--------------------------------------------------------------
int handleClient(int client_socket) // retrieving a packet sent by the Client
{
	int rcvSize, sndSize, packetSize, ntpbpktSize, recv_size, sent_size;
	u8 *pbuf;

	netlog_send("%s: Client Connection OK\n", MODNAME);
	
	pbuf = (u8 *)&ntpb_buf[0];

	while (1) {
		// receive the request packet
		rcvSize = recv_noblock(client_socket, &ntpb_buf[0], sizeof(ntpb_buf));
		if (rcvSize <= 0) 
			return -1;

		// Get packet Size
		ntpbpktSize = *((u16 *)&pbuf[6]);
		packetSize = ntpbpktSize + ntpb_hdrSize;

		recv_size = rcvSize;

		// fragmented packet handling
		while (recv_size < packetSize) {
			rcvSize = recv_noblock(client_socket, &ntpb_buf[recv_size], sizeof(ntpb_buf) - recv_size);
			if (rcvSize <= 0)
				return -1;
			
			recv_size += rcvSize;
		}

		// parses packet
		if (check_ntpb_header(pbuf)) {
			
			// Get Remote Command, data size, and cmd datas
			remote_cmd = *((u16 *)&pbuf[8]);
			cmdsize = ntpbpktSize;
			if (cmdsize)
				memcpy(cmdbuf, &pbuf[ntpb_hdrSize], cmdsize);
			
			netlog_send("%s: server received command 0x%04x size=%d\n", MODNAME, remote_cmd, cmdsize);
			
			// prepare a response
			*((u16 *)&pbuf[6]) = 0;
			packetSize = ntpb_hdrSize + 2;
			*((u16 *)&pbuf[ntpb_hdrSize]) = 1;
			
			// Send response
			sent_size = 0;
			// fragmented packet handling
			while (sent_size < packetSize) {
				sndSize = lwip_send(client_socket, &ntpb_buf[sent_size], packetSize - sent_size, 0);
				if (sndSize < 0)
					return -1;
			
				sent_size += sndSize;
			}
						
			// Wait server CMD semaphore before to continue to wait for client requests 
			WaitSema(ntpbserver_cmd_sema);	
		}
	}

	return 0;
}

//--------------------------------------------------------------
void serverThread(void *args) // Server thread: Handle Client & packets
{
	int tcp_socket;
	struct sockaddr_in peer;
	int peerlen, r, err;

conn_retry:
	
	peer.sin_family = AF_INET;
	peer.sin_port = htons(SERVER_TCP_PORT);
	peer.sin_addr.s_addr = htonl(INADDR_ANY);

	netlog_send("%s: server init starting...\n", MODNAME);
		
	// create the socket
	tcp_socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		err = -1;
		goto error;		
	}

	netlog_send("%s: server socket created.\n", MODNAME);
				
	r = lwip_bind(tcp_socket,(struct sockaddr *)&peer,sizeof(peer));
	if (r < 0) {
		err = -2;
		goto error;
	}

	netlog_send("%s: bind OK.\n", MODNAME);
		
	r = lwip_listen(tcp_socket, 3);
	if (r < 0) {
		err = -3;
		goto error;
	}

	netlog_send("%s: server ready!\n", MODNAME);
	
	while(1) {
		peerlen = sizeof(peer);
		r = lwip_accept(tcp_socket,(struct sockaddr *)&peer, &peerlen);
		if (r < 0) {
			err = -4;
			goto error;
		}

		client_socket = r;
 
		r = handleClient(client_socket);
		if (r < 0) {
			lwip_close(client_socket);
			netlog_send("%s: Client Connection closed - error %d\n", MODNAME, r);
		}
	}
	 
error:
	lwip_close(client_socket);
	netlog_send("%s: Client Connection closed - error %d\n", MODNAME, err);
	goto conn_retry;
}

//-------------------------------------------------------------- 
int start_ServerThread(void)
{
	iop_thread_t thread_param;
	register int thread_id;	
			
 	thread_param.attr = TH_C;
 	thread_param.thread = (void *)serverThread;
 	thread_param.priority = 0x64;
 	thread_param.stacksize = 0x1000;
 	thread_param.option = 0;
			
	thread_id = CreateThread(&thread_param);
		
	StartThread(thread_id, 0);
	
	server_tid = thread_id;
	
	return 0;
}

//--------------------------------------------------------------
void *cb_rpc_S_0A10(u32 fno, void *buf, int size)
{
	// Rpc Callback function
	
	if (fno >= 16)
		return (void *)&rpc_stat;

	// Get function pointer
	rpc_func = (void *)rpc_funcs_array[fno];
	
	// Call needed rpc func
	rpc_stat.rpc_func_ret = rpc_func();
	
	//return (void *)&rpc_stat;
	return (void *)buf;
}

//--------------------------------------------------------------
void thread_rpc_S_0A10(void* arg)
{
	if (!sceSifCheckInit())
		sceSifInit();

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&rpc_qdS_0A10, GetThreadId());
	sceSifRegisterRpc(&rpc_sdS_0A10, 0x80000a10, (void *)cb_rpc_S_0A10, &ntpbserver_rpc_buf, NULL, NULL, &rpc_qdS_0A10);
	sceSifRpcLoop(&rpc_qdS_0A10);
}

//-------------------------------------------------------------- 
int start_RPC_server(void)
{
	iop_thread_t thread_param;
	register int thread_id;	
			
 	thread_param.attr = TH_C;
 	thread_param.thread = (void *)thread_rpc_S_0A10;
 	thread_param.priority = 0x68;
 	thread_param.stacksize = 0x1000;
 	thread_param.option = 0;
			
	thread_id = CreateThread(&thread_param);
	rpc_tidS_0A10 = thread_id;
		
	StartThread(thread_id, 0);
	
	return 0;
}

//-------------------------------------------------------------- 
int rpcNTPBsendDataToClient(void)
{
	return _rpcNTPBsendDataToClient(&ntpbserver_rpc_buf);
}

//-------------------------------------------------------------- 
int _rpcNTPBsendDataToClient(void *rpc_buf)
{
	int r;
	g_sendDataParam_t *eP = (g_sendDataParam_t *)rpc_buf;	
				
	r = ntpbserverSendData(eP->cmd, eP->buf, eP->size);
	
	return r;
}

//-------------------------------------------------------------- 
int rpcNTPBgetRemoteCmd(void)
{
	return _rpcNTPBgetRemoteCmd(&ntpbserver_rpc_buf);
}

//-------------------------------------------------------------- 
int _rpcNTPBgetRemoteCmd(void *rpc_buf)
{
	g_getRemoteCmdParam_t *eP = (g_getRemoteCmdParam_t *)rpc_buf;	
				
	eP->cmd = remote_cmd;
	remote_cmd = 0;
	
	memcpy(eP->buf, cmdbuf, cmdsize);
	eP->size = cmdsize;
	
	return 1;
}

//-------------------------------------------------------------- 
int rpcNTPBEndReply(void)
{
	return _rpcNTPBEndReply(&ntpbserver_rpc_buf);
}

//-------------------------------------------------------------- 
int _rpcNTPBEndReply(void *rpc_buf)
{
	int r;
				
	r = ntpbserverEndReply();
	
	return r;
}

//-------------------------------------------------------------------------
int _start(int argc, char** argv)
{			
	iop_sema_t smp;
				
	SifInitRpc(0);
	
	// init netlog
	netlog_init(0, SERVER_UDP_PORT);
	netlog_send("hello from debugger.irx\n");
		
	// Starting ntpbserver Remote Procedure Call server	
	start_RPC_server();
	
	// Starting server thread
	start_ServerThread();		

	smp.attr = 1;
	smp.initial = 1;
	smp.max = 1;
	smp.option = 0;
	ntpbserver_io_sema = CreateSema(&smp);	

	smp.attr = 1;
	smp.initial = 0; // this sema is initialised to 0 !
	smp.max = 1;
	smp.option = 0;
	ntpbserver_cmd_sema = CreateSema(&smp);	
						
	return MODULE_RESIDENT_END;
}
