/****************************************************************************
FILE IO Library

Functions for file manipulation. I can't say it's completely API free though,
since I like to call MessageBox() for errors.

NO GLOBALS
*****************************************************************************/
#include "ps2cc.h"

/****************************************************************************
File Exsistance check - return 0 if file does NOT exist, >0 if exists.
--To Do: Update so filesize is returned on exist using ftell.
*****************************************************************************/
u32 FileExists(char *filename)
{
    FILE *f = fopen(filename,"rb");
    if (!(f)) { return 0; }
    fclose(f);
    return 1;
}

/****************************************************************************
Load File
Loads a file (filename), cuts the header (headerlen) and reads the data into
buffer. if loadheader is true, it also reads the header into the headerdata.
I might attempt to revise this later so the data arrays don't need to be
globals. Dealing with these pointers has always been a bitch though. I think
the main problem with locals has been in trying to malloc from here.
*****************************************************************************/
int LoadFile(u8 **buffer, char* filename, int headerlen, u8 **headerdata, BOOL loadheader)
{
    FILE *f;
    int i;
    u32 filesize;
	f = fopen(filename,"rb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open file (LoadFile,1) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
    }
	fseek(f,0,SEEK_END);
	filesize = ftell(f);
	fseek(f,0,SEEK_SET);
    if (headerlen > filesize) {
	    MessageBox(NULL,"Header length greater than file size. WTF? Idiot. (LoadFile,1)","Error",0);
	    fclose(f); return 0;
	}
    if (*buffer) { free(*buffer); *buffer = NULL; }
    if (!(*buffer = (unsigned char*)malloc(filesize+1))) {
        sprintf(ErrTxt, "Unable to allocate buffer memory (LoadFile, 1) -- Error %u", GetLastError());
        MessageBox(NULL, ErrTxt, "Error", MB_OK);
        fclose(f); return 0;
    }
    if ((loadheader) && (headerlen)) {
        if (*headerdata) { free(*headerdata); *headerdata = NULL; }
        if (!(*headerdata = (unsigned char*)malloc(headerlen))) {
            sprintf(ErrTxt, "Unable to allocate header memory (LoadFile, 1) -- Error %u", GetLastError());
            MessageBox(NULL, ErrTxt, "Error", MB_OK);
            free(*headerdata); *headerdata = NULL;
            fclose(f); return 0;
        }
        fread(*headerdata,1,headerlen,f);
        filesize -= headerlen;
    } else {
        filesize -= headerlen;
        fseek(f,headerlen,SEEK_SET);
    }
	fread(*buffer,1,filesize,f);
	fclose(f);
	return filesize;
}

/****************************************************************************
SaveFile
*****************************************************************************/
int SaveFile(u8 *buffer, u32 filesize, char* filename, int headerlen, VOID *headerdata)
{
    FILE *f;
    int i;
	f = fopen(filename,"wb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open/create file (SaveFile,1) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
    }
	fseek(f,0,SEEK_SET);
	if (headerlen) { fwrite(headerdata,1,headerlen,f); }
	fwrite(buffer,1,filesize,f);
	fclose(f);
	return filesize;
}

/****************************************************************************
LoadStruct -Read a struct that was saved as binary.
*****************************************************************************/
int LoadStruct(VOID *buffer, u32 filesize, char* filename)
{
    FILE *f;
    int i;
	f = fopen(filename,"rb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open/create file (LoadStruct,1) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
    }
	fseek(f,0,SEEK_SET);
	fread(buffer,1,filesize,f);
	fclose(f);
	return filesize;
}

/****************************************************************************
SaveStruct - Write a struct to file as binary.
*****************************************************************************/
int SaveStruct(VOID *buffer, u32 filesize, char* filename)
{
    FILE *f;
    int i;
	f = fopen(filename,"wb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open/create file (SaveStruct,1) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
    }
	fseek(f,0,SEEK_SET);
	fwrite(buffer,1,filesize,f);
	fclose(f);
	return filesize;
}

/****************************************************************************
CopyBinFile
*****************************************************************************/
int CopyBinFile(char *filename, char *newfilename)
{
    FILE *f;
    u64 filesize;
	f = fopen(filename,"rb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open source file (CopyBinFile) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK); return 0;
	}
	fseek(f,0,SEEK_END);
	filesize = ftell(f);
	fseek(f,0,SEEK_SET);
	u8 *buffer;
    if (!(buffer = (u8*)malloc(filesize+1))) {
        sprintf(ErrTxt, "Unable to allocate buffer memory (CopyBinFile) -- Error %u", GetLastError());
        MessageBox(NULL, ErrTxt, "Error", MB_OK);
        fclose(f); return 0;
    }
	fread(buffer,1,filesize,f);
	fclose(f);
	f = fopen(newfilename,"wb");
	if (!(f)) {
        sprintf(ErrTxt, "Unable to open destination file (CopyBinFile) -- Error %u", GetLastError());
        MessageBox(NULL,ErrTxt,"Error",MB_OK);
        free(buffer); return 0;
	}
	fwrite(buffer,1,filesize,f);
	fclose(f);
	free(buffer);
	return 1;
}
