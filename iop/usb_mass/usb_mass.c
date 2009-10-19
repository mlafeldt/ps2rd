/*
 * usb_mass.c - USB Mass storage driver for PS2
 *
 * (C) 2001, Gustavo Scotti (gustavo@scotti.com)
 * (C) 2002, David Ryan ( oobles@hotmail.com )
 * (C) 2004, Marek Olejnik (ole00@post.cz)
 *
 * IOP file io driver and RPC server
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include <tamtypes.h>
#include <thbase.h>
#include <thsemap.h>
#include <sifrpc.h>
#include <ioman.h>

#include <cdvdman.h>
#include <sysclib.h>

#include "mass_stor.h"
#include "fat_driver.h"

//#define DEBUG 1
#include "mass_debug.h"


#define BIND_RPC_ID 0x500C0F1

/* function declaration */
void rpcMainThread(void* param);
void *rpcCommandHandler(u32 command, void *buffer, int size);


static SifRpcDataQueue_t rpc_queue __attribute__((aligned(64)));
static SifRpcServerData_t rpc_server __attribute((aligned(64)));
static int _rpc_buffer[512] __attribute((aligned(64)));
static int threadId;

static iop_device_t fs_driver;
static iop_device_ops_t fs_functarray;


/* init file system driver */
void initFsDriver() {
	int i;

	fs_driver.name = "mass";
	fs_driver.type = IOP_DT_FS;
	fs_driver.version = 1;
	fs_driver.desc = "Usb mass storage driver";
	fs_driver.ops = &fs_functarray;

	fs_functarray.init    = fs_init;
	fs_functarray.deinit  = fs_deinit;
	fs_functarray.format  = fs_format;
	fs_functarray.open    = fs_open;
	fs_functarray.close   = fs_close;
	fs_functarray.read    = fs_read;
	fs_functarray.write   = fs_write;
	fs_functarray.lseek   = fs_lseek;
	fs_functarray.ioctl   = fs_ioctl;
	fs_functarray.remove  = fs_remove;
	fs_functarray.mkdir   = fs_mkdir;
	fs_functarray.rmdir   = fs_rmdir;
	fs_functarray.dopen   = fs_dopen;
	fs_functarray.dclose  = fs_dclose;
	fs_functarray.dread   = fs_dread;
	fs_functarray.getstat = fs_getstat;
	fs_functarray.chstat  = fs_chstat;

	DelDrv("mass");
	AddDrv(&fs_driver);

}

extern int driver_ready_sema;

int _start( int argc, char **argv)
{
	iop_thread_t param;
	int th;
	iop_sema_t s;

	s.initial = 0;
	s.max = 1;
	s.option = 0;
	s.attr = 0;
	driver_ready_sema = CreateSema(&s);

	FlushDcache();
	initFsDriver();

	/*create thread*/
	param.attr         = TH_C;
	param.thread     = rpcMainThread;
	param.priority = 40;
	param.stacksize    = 0x8000;
	param.option      = 0;


	th = CreateThread(&param);
	if (th > 0) {
		StartThread(th,0);
		return 0;
	} else  {
		return 1;
	}


}
void rpcMainThread(void* param)
{
	int ret=-1;
	int tid;

	SifInitRpc(0);

	printf("usb_mass: version 0.40");
#ifdef WRITE_SUPPORT
	printf(" wr - experimental! Use at your own risk!\n");
#else
	printf(" ro\n"); //read only 
#endif

	tid = GetThreadId();

	SifSetRpcQueue(&rpc_queue, tid);
	SifRegisterRpc(&rpc_server, BIND_RPC_ID, (void *) rpcCommandHandler, (u8 *) &_rpc_buffer, 0, 0, &rpc_queue);

    SignalSema(driver_ready_sema);

	SifRpcLoop(&rpc_queue);
}

void dumpDiskContent(unsigned int startSector, unsigned int endSector, char* fname) {
	unsigned int i;
	int ret;
	int fd;

	printf("--- dump start: start sector=%i end sector=%i fd=%i--- \n", startSector, endSector, fd);
	
	ret = 1;
	for (i = startSector; i < endSector && ret > 0; i++) {
		ret = fat_dumpSector(i);
	}
	printf("-- dump end --- \n" );
	for (i = 0; i < 256; i++) {
		printf("                                           \n");
	}

}

void dumpDiskContentFile(unsigned int startSector, unsigned int endSector, char* fname) {
	unsigned int i;
	int ret;
	int fd;
	unsigned char* buf;

	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC);
	if (fd <= 0) {
		printf ("dump content: file open failed ret=%d\n", ret);
		return;
	}

	printf("--- dump start: start sector=%i end sector=%i fd=%i--- \n", startSector, endSector, fd);
	
	ret = 1;
	for (i = startSector; i < endSector && ret > 0; i++) {
		ret = fat_readSector(i, &buf);
		//printf("sector= %d ret=%d buf=%p\n", i, ret, buf);
		if (ret > 0) write(fd, buf, ret);
	}
	close (fd);
	printf("-- dump end --- \n" );
}

void overwriteDiskContentFile(unsigned int startSector, unsigned int endSector, char* fname) {
#ifdef WRITE_SUPPORT

	unsigned int i;
	int ret;
	int fd;
	unsigned char* buf;
	unsigned char* rbuf;
	int index;

	rbuf = (unsigned char*) _rpc_buffer;

	fd = open(fname, O_RDONLY);
	if (fd <= 0) {
		printf ("write content: file open failed ret=%d\n", ret);
		return;
	}

	printf("--- write start: start sector=%i end sector=%i fd=%i--- \n", startSector, endSector, fd);
	
	ret = 1;
	for (i = startSector; i < endSector && ret > 0; i++) {
		index = i % 4;
		if (index == 0) {
			ret = read(fd, rbuf, 2048);
			
		}
		ret = fat_allocSector(i, &buf);
		memcpy(buf, rbuf + index * 512, 512);
		fat_writeSector(i);
		if (i % 1024 == 0) 
			printf("sector= %d ret=%d buf=%p\n", i, ret, buf);
	}
	if (ret) fat_flushSectors();
	close (fd);
	printf("-- dump end --- \n" );
/* WRITE_SUPPORT */
#endif 
}

int getFirst(void* buf) {
	fat_dir fatDir;
	int ret;
	ret = fat_getFirstDirentry((char*) buf, &fatDir);
	if (ret > 0) {
		memcpy(buf, &fatDir, sizeof(fat_dir_record)); //copy only important things
	}
	return ret;
}
int getNext(void* buf) {
	fat_dir fatDir;
	int ret;
	
	ret = fat_getNextDirentry(&fatDir);
	if (ret > 0) {
		memcpy(buf, &fatDir, sizeof(fat_dir_record)); //copy only important things
	}
	return ret;
}


void *rpcCommandHandler(u32 command, void *buffer, int size)

{
	int* buf = (int*) buffer;
	int ret = 0;

	switch (command) {
		case 1: //getFirstDirentry
			ret  = getFirst(((char*) buffer) + 4); //reserve 4 bytes for returncode
			break;
		case 2: //getNextDirentry
			ret = getNext(((char*) buffer) + 4);
		 	break;
		case 3: //dumpContent
			printf("rpc called - dump disk content\n");
			dumpDiskContentFile(buf[0], buf[1], ((char*) buffer) + 8);
			break;
		case 4: //dump system info
			ret = fat_dumpSystemInfo(buf[0], buf[1]);
			break;
		case 5: //overwrite disk 
			printf("rpc called - overwrite disk content\n");
			overwriteDiskContentFile(buf[0], buf[1], ((char*) buffer) + 8);
			break;
	}
	buf[0] = ret; //store return code
	return buffer;
}
