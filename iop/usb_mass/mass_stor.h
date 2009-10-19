#ifndef _MASS_STOR_H
#define _MASS_STOR_H 1

int mass_stor_init();
int mass_stor_disconnect(int devId);
int mass_stor_connect(int devId);
int mass_stor_probe(int devId);
int mass_stor_readSector4096(unsigned int sector, unsigned char* buffer);
int mass_stor_writeSector4096(unsigned int sector, unsigned char* buffer);

#endif
