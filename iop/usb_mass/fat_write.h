#ifndef _FAT_WRITE_H
#define _FAT_WRITE_H 1

#include "fat.h"

unsigned char toUpperChar(unsigned char c);
int fat_createFile(fat_bpb* bpb, const char* fname, char directory, char escapeNotExist, unsigned int* cluster, unsigned int* sfnSector, int* sfnOffset);
int fat_deleteFile(fat_bpb* bpb, const char* fname, char directory);
int fat_truncateFile(fat_bpb* bpb, unsigned int cluster, unsigned int sfnSector, int sfnOffset );
int fat_writeFile(fat_bpb* bpb, fat_dir* fatDir, int* updateClusterIndices, unsigned int filePos, unsigned char* buffer, int size) ;
int fat_updateSfn(int size, unsigned int sfnSector, int sfnOffset );

int fat_allocSector(unsigned int sector, unsigned char** buf);
int fat_writeSector(unsigned int sector);
int fat_flushSectors(void);
#endif /* _FAT_WRITE_H */
