#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>
#include <pspreg.h>
#include <pspctrl.h>
#include <common.h>

/*
	File/Directory
*/
int ReadFile(char *file, int seek, char *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if(fd < 0){return fd;}

	if(seek > 0){
		if(sceIoLseek(fd, seek, PSP_SEEK_SET) != seek){
			sceIoClose(fd);
			return -1;
		}
	}

	int read = sceIoRead(fd, buf, size);
	
	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, int seek, char *buf, int size)
{
	int i, pathlen = 0;
	char dirpath[128];

	for(i=1; i<(strlen(file)); i++){
		if(strncmp(file+i-1, "/", 1) == 0){
			pathlen=i-1;
			strncpy(dirpath, file, pathlen);
			dirpath[pathlen] = 0;
			sceIoMkdir(dirpath, 0777);
		}
	}

	if(FileExists(file)){sceIoRemove(file);}

	SceUID fd = sceIoOpen(file, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
	if(fd < 0){return fd;}

	if(seek > 0){
		if(sceIoLseek(fd, seek, PSP_SEEK_SET) != seek){
			sceIoClose(fd);
			return -1;
		}
	}

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int FileExists(char *file)
{
	if(access(file, F_OK)){return 0;}
	else{return 1;}
}
int GetFileSize(char *file)
{
	SceIoStat info;
	memset(&info, 0, sizeof(info));
	sceIoGetstat(file, &info);
	return info.st_size;
}
int DirExists(char *dir)
{
	SceUID d = sceIoDopen(dir);
	if(d < 0){return 0;}
	sceIoClose(d);

	return 1;
}
/*
	ZIP
*/
int zipFileExtract(char *archivepath, int archiveoffs, char *filename, char *outname, char *buf)
{
	int size = zipFileRead(archivepath, archiveoffs, filename, buf);
	if(size == NULL){return NULL;}

	size = WriteFile(outname, 0, buf, size);  

	return size;
}
int zipFileRead(char *archivepath, int archiveoffs, char *filename, char *buf)
{
	struct SZIPFileHeader data;
	char foundfilename[1024];
	u8 *cbuffer;

	SceUID fd = sceIoOpen(archivepath, PSP_O_RDONLY, 0);
	sceIoLseek(fd, archiveoffs, SEEK_CUR);
	
	while(1){
		sceIoRead(fd, &data, sizeof(struct SZIPFileHeader));
		if(data.Sig[0] != 0x50 || data.Sig[1] != 0x4B || data.Sig[2] != 0x03 || data.Sig[3] != 0x04){sceIoClose(fd);return NULL;} // check correct sig

		sceIoRead(fd, foundfilename, data.FilenameLength); // get filename
		foundfilename[data.FilenameLength] = 0;
		if(data.ExtraFieldLength){sceIoLseek(fd, data.ExtraFieldLength, SEEK_CUR);} // seek to the data start

		if(!strcmp(strlwr(foundfilename), strlwr(filename))){break;}
		else{sceIoLseek(fd, data.DataDescriptor.CompressedSize, SEEK_CUR);} // seek to the end of the file
	}

	if(data.CompressionMethod == 0){
		sceIoRead(fd, buf, data.DataDescriptor.UncompressedSize);
	}
	else if(data.CompressionMethod == 8){
		int inflatedbytes = 0, readbytes = 0;
		z_stream stream;
        u32 err;

        cbuffer = malloc(131072);

		if(cbuffer == NULL){sceIoClose(fd);return NULL;}

        stream.next_in = (Bytef*)cbuffer;
        stream.avail_in = 131072;
		
        stream.next_out = (Bytef*)buf;
        stream.avail_out = data.DataDescriptor.UncompressedSize;

        stream.zalloc = Z_NULL;
        stream.zfree  = Z_NULL;
        stream.opaque = Z_NULL;

        err = inflateInit2(&stream, -MAX_WBITS);

		if(data.DataDescriptor.CompressedSize > 131072){readbytes += sceIoRead(fd, cbuffer, 131072);}
		else{readbytes += sceIoRead(fd, cbuffer, data.DataDescriptor.CompressedSize);}

		if(err == Z_OK){
 			while(inflatedbytes < data.DataDescriptor.CompressedSize){
				err = inflate(&stream, Z_SYNC_FLUSH);
				inflatedbytes = readbytes;

				if(err == Z_BUF_ERROR){
					stream.next_in = (Bytef*)cbuffer;
					stream.avail_in = 131072;

					if(data.DataDescriptor.CompressedSize > readbytes + 131072){readbytes += sceIoRead(fd, cbuffer, 131072);}
					else{readbytes += sceIoRead(fd, cbuffer, data.DataDescriptor.CompressedSize - readbytes);}
				}
			}
			err = Z_OK;
			inflateEnd(&stream);
		}

		free(cbuffer);
	}

	sceIoClose(fd);

	return data.DataDescriptor.UncompressedSize;
}

int zipFileExists(char *archivepath, int archiveoffs, char *filename)
{
	struct SZIPFileHeader data;
	char foundfilename[1024];

	SceUID fd = sceIoOpen(archivepath, PSP_O_RDONLY, 0);
	sceIoLseek(fd, archiveoffs, SEEK_CUR);
	
	while(1){
		sceIoRead(fd, &data, sizeof(struct SZIPFileHeader));
		if(data.Sig[0] != 0x50 || data.Sig[1] != 0x4B || data.Sig[2] != 0x03 || data.Sig[3] != 0x04){sceIoClose(fd);return -1;} // check correct sig

		sceIoRead(fd, foundfilename, data.FilenameLength); // get filename
		foundfilename[data.FilenameLength] = 0;
		if(data.ExtraFieldLength){sceIoLseek(fd, data.ExtraFieldLength, SEEK_CUR);} // seek to the data start

		if(!strcmp(strlwr(foundfilename), strlwr(filename))){sceIoClose(fd);return 1;}
		else{sceIoLseek(fd, data.DataDescriptor.CompressedSize, SEEK_CUR);} // seek to the end of the file
	}

	sceIoClose(fd);

	return 0;
}
int zipFileSize(char *archivepath, int archiveoffs, char *filename)
{
	struct SZIPFileHeader data;
	char foundfilename[1024];

	SceUID fd = sceIoOpen(archivepath, PSP_O_RDONLY, 0);
	sceIoLseek(fd, archiveoffs, SEEK_CUR);
	
	while(1){
		sceIoRead(fd, &data, sizeof(struct SZIPFileHeader));
		if(data.Sig[0] != 0x50 || data.Sig[1] != 0x4B || data.Sig[2] != 0x03 || data.Sig[3] != 0x04){sceIoClose(fd);return NULL;} // check correct sig

		sceIoRead(fd, foundfilename, data.FilenameLength); // get filename
		foundfilename[data.FilenameLength] = 0;
		if(data.ExtraFieldLength){sceIoLseek(fd, data.ExtraFieldLength, SEEK_CUR);} // seek to the data start

		if(strcmp(strlwr(foundfilename), strlwr(filename)) == 0){break;}
		else{sceIoLseek(fd, data.DataDescriptor.CompressedSize, SEEK_CUR);} // seek to the end of the file
	}

	sceIoClose(fd);

	return data.DataDescriptor.UncompressedSize;
}
int zipFileSizeCmp(char *archivepath, int archiveoffs, char *filename)
{
	struct SZIPFileHeader data;
	char foundfilename[1024];

	SceUID fd = sceIoOpen(archivepath, PSP_O_RDONLY, 0);
	sceIoLseek(fd, archiveoffs, SEEK_CUR);
	
	while(1){
		sceIoRead(fd, &data, sizeof(struct SZIPFileHeader));
		if(data.Sig[0] != 0x50 || data.Sig[1] != 0x4B || data.Sig[2] != 0x03 || data.Sig[3] != 0x04){sceIoClose(fd);return NULL;} // check correct sig

		sceIoRead(fd, foundfilename, data.FilenameLength); // get filename
		foundfilename[data.FilenameLength] = 0;
		if(data.ExtraFieldLength){sceIoLseek(fd, data.ExtraFieldLength, SEEK_CUR);} // seek to the data start

		if(!strcmp(strlwr(foundfilename), strlwr(filename))){break;}
		else{sceIoLseek(fd, data.DataDescriptor.CompressedSize, SEEK_CUR);} // seek to the end of the file
	}

	sceIoClose(fd);

	return data.DataDescriptor.CompressedSize;
}
/*
	Misc
*/
int GetKeyPress(int wait)
{
	SceCtrlData pad;
	int btn = 0;

	while(!btn){
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(1);
		sceCtrlReadBufferPositive(&pad, 1);
		btn = pad.Buttons & 0xFFFF;
		if(!wait){break;}
		sceKernelDelayThread(10000);
	}

	return btn;
}
int Rand(int min, int max)
{
	u64 tick;
	SceKernelUtilsMt19937Context ctx;
	sceRtcGetCurrentTick(&tick);
	sceKernelUtilsMt19937Init(&ctx, (u32)tick);

	return min + (sceKernelUtilsMt19937UInt(&ctx) % max);
}
void *GetRegistryValue(const char *dir, const char *name, void *buf, int bufsize)
{
	int ret = NULL;
	struct RegParam reg;
	REGHANDLE h;

	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	reg.namelen = strlen("/system");
	reg.unk2 = 1;
	reg.unk3 = 1;
	strcpy(reg.name, "/system");
	if(sceRegOpenRegistry(&reg, 2, &h) == 0){
		REGHANDLE hd;
		if(!sceRegOpenCategory(h, dir, 2, &hd)){
			REGHANDLE hk;
			unsigned int type, size;

			if(!sceRegGetKeyInfo(hd, name, &hk, &type, &size)){
				if(!sceRegGetKeyValue(hd, hk, buf, bufsize)){
					ret = buf;
					sceRegFlushCategory(hd);
				}
			}
			sceRegCloseCategory(hd);
		}
		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	return ret;
}
int LoadStartModule(char *filepath)
{
    SceUID mod;

    mod = sceKernelLoadModule(filepath, 0, NULL);
	if(mod < 0){return mod;}

	return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}
void LoadStartModuleBuffer(char *path, char *buf, int size)
{
    SceUID mod, out;

	sceIoRemove(path);
	out = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777);
	sceIoWrite(out, buf, size);
	sceIoClose(out);

    mod = sceKernelLoadModule(path, 0, NULL);
	mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}
void LoadStartModuleDirectory(char *dirpath)
{
	int dl = sceIoDopen(dirpath);
	if(dl < 0){return;}

	SceIoDirent sid;
	memset(&sid, 0, sizeof(SceIoDirent));

	while(sceIoDread(dl, &sid)){
		if(sid.d_name[0] == '.'){continue;}

		char filepath[256];
		sprintf(filepath, "%s/%s", dirpath, sid.d_name);

		if(!FIO_S_ISDIR(sid.d_stat.st_mode)){
			if(sceKernelDevkitVersion() > 0x02070110 && sid.d_name == "iop.prx"){continue;}

			char *ext = strrchr(filepath, '.');
			if(!stricmp(ext, ".prx")){LoadStartModule(filepath);}
		}

		memset(&sid, 0, sizeof(SceIoDirent));
	}

	sceIoDclose(dl);
}
int GetEBOOToffset(char *file, u32 filename)
{
	char buf[PBPHEADERSIZE];
	memset(buf, 0, PBPHEADERSIZE);
	ReadFile(file, 0, buf, PBPHEADERSIZE);

	int ret = *(u32 *)&buf[filename];
	return ret;
}
int GetEBOOToffsetBuff(char *buf, u32 filename)
{
	int ret = *(u32 *)&buf[filename];
	return ret;
}
