#ifndef _FAT_DRIVER_H
#define _FAT_DRIVER_H 1

#ifdef _PS2_
#include <io_common.h>
#include <ioman.h>
#else
#include <fcntl.h>
#

#define FIO_SO_IFREG		0x0010		// Regular file
#define FIO_SO_IFDIR		0x0020		// Directory
/* fake struct for non ps2 systems */
typedef struct _iop_file {
	void	*privdata;
} iop_file_t;
typedef void iop_device_t;

typedef struct {
	unsigned int mode;
	unsigned int attr;
	unsigned int size;
	unsigned char ctime[8];
	unsigned char atime[8];
	unsigned char mtime[8];
	unsigned int hisize;
} fio_stat_t;

typedef struct {
	fio_stat_t stat;
	char name[256];
	unsigned int unknown;
} fio_dirent_t;


#endif /* _PS2_ */

#include "fat.h"

typedef struct _fs_rec {
	int fd;
	unsigned int  filePos;
	int           mode;	//file open mode
	unsigned int  sfnSector; //short filename sector  - write support
	int           sfnOffset; //short filename offset  - write support
	int           sizeChange; //flag
} fs_rec;


#define FAT_ERROR           -1

#define MAX_DIR_CLUSTER 512


int mass_stor_getStatus();

/*
int fs_open( int fd, char *name, int mode);
int fs_lseek(int fd, int offset, int whence);
int fs_read( int fd, char * buffer, int size );
int fs_write( int fd, char * buffer, int size );
int fs_close( int fd);
int fs_dummy(void);
*/

int fs_init   (iop_device_t *driver); 
int fs_open   (iop_file_t* , const char *name, int mode);
int fs_lseek  (iop_file_t* , unsigned long offset, int whence);
int fs_read   (iop_file_t* , void * buffer, int size );
int fs_write  (iop_file_t* , void * buffer, int size );
int fs_close  (iop_file_t* );
int fs_dummy  (void);

int fs_deinit (iop_device_t *);
int fs_format (iop_file_t *);
int fs_ioctl  (iop_file_t *, unsigned long, void *);
int fs_remove (iop_file_t *, const char *);
int fs_mkdir  (iop_file_t *, const char *);
int fs_rmdir  (iop_file_t *, const char *);
int fs_dopen  (iop_file_t *, const char *);
int fs_dclose (iop_file_t *);
int fs_dread  (iop_file_t *, fio_dirent_t *);
int fs_getstat(iop_file_t *, const char *, fio_stat_t *);

int fs_chstat (iop_file_t *, const char *, fio_stat_t *, unsigned int);


int getI32(unsigned char* buf);
int getI32_2(unsigned char* buf1, unsigned char* buf2);
int getI16(unsigned char* buf);
int strEqual(unsigned char *s1, unsigned char* s2);
unsigned int fat_getClusterRecord12(unsigned char* buf, int type);
unsigned int fat_cluster2sector(fat_bpb* bpb, unsigned int cluster);

int      fat_initDriver(void);
void     fat_closeDriver(void);
fat_bpb* fat_getBpb(void);
int      fat_getFileStartCluster(fat_bpb* bpb, const char* fname, unsigned int* startCluster, fat_dir* fatDir);
int      fat_getDirentrySectorData(fat_bpb* bpb, unsigned int* startCluster, unsigned int* startSector, int* dirSector);
unsigned int fat_cluster2sector(fat_bpb* bpb, unsigned int cluster);
int      fat_getDirentry(fat_direntry_sfn* dsfn, fat_direntry_lfn* dlfn, fat_direntry* dir );
int      fat_getClusterChain(fat_bpb* bpb, unsigned int cluster, unsigned int* buf, int bufSize, int start);
void     fat_invalidateLastChainResult();
void     fat_getClusterAtFilePos(fat_bpb* bpb, fat_dir* fatDir, unsigned int filePos, unsigned int* cluster, unsigned int* clusterPos);



//void fat_dumpFile(fat_bpb* bpb, int fileCluster, int size, char* fname);
void fat_dumpDirectory(fat_bpb* bpb, int dirCluster);
void fat_dumpPartitionBootSector();
void fat_dumpPartitionTable();
void fat_dumpClusterChain(unsigned int* buf, int maxBuf, int clusterSkip);
int  fat_dumpSystemInfo();
void fat_dumpSectorHex(unsigned char* buf, int bufSize);


int fat_getFirstDirentry(char * dirName, fat_dir* fatDir);
int fat_getNextDirentry(fat_dir* fatDir);
#endif

