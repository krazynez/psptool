// bulk of code by cory1492, nem, chilly willy, silverspring

#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include <pspsysmem.h>
#include <pspctrl.h>
#include <pspidstorage.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <psputility_netmodules.h>
#include <main.h>
#include <pspnand_driver.h>

#define sceSysregGetFuseId sceSysreg_driver_4F46EEDE
#define sceSysregGetFuseConfig sceSysreg_driver_8F4F4E96
#define sceSysregGetTachyonVersion sceSysreg_driver_E2A5D1EE
#define sceSysconGetBaryonVersion sceSyscon_driver_7EC5A957
#define sceSysconGetPommelVersion sceSyscon_driver_E7E87741

PSP_MODULE_INFO("Kprx", 0x5006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

// syscon function for all versions used by silversprings' code
u32 sceSysconCmdExec(void* param, int unk);

u64 sceSysregGetFuseId(void);
u32 sceSysregGetFuseConfig(void);
u32 sceSysregGetTachyonVersion(void);
u32 sceSysconGetBaryonVersion(u32* val);
u32 sceSysconGetPommelVersion(u32* val);
int sceNandSetScramble(u32 magic);

/* 
	Battery (code from Open Source Pandora Battery Tool by cory1492)
*/
u32 writeBat(u8 addr, u16 data) // reversed function by silverspring (more info: http://my.malloc.us/silverspring/2007/12/19/380-and-pandora/)
{
	int k1 = pspSdkSetK1(0);

	int res;
	u8 param[0x60];

	if (addr > 0x7F)
		return(0x80000102);

	param[0x0C] = 0x73; // write battery eeprom command
	param[0x0D] = 5;	// tx packet length

	// tx data
	param[0x0E] = addr;
	param[0x0F] = data;
	param[0x10] = data>>8;
	
	res = sceSysconCmdExec(param, 0);
	
	if (res < 0)
		return(res);

	pspSdkSetK1(k1);
	return 0;
}

u32 readBat(u8 addr) // reversed function by silverspring (more info: http://my.malloc.us/silverspring/2007/12/19/380-and-pandora/)
{
	int res;
 	int k1 = pspSdkSetK1(0);

	u8 param[0x60];

	if (addr > 0x7F)
		return(0x80000102);
		
	param[0x0C] = 0x74; // read battery eeprom command
	param[0x0D] = 3;	// tx packet length

	// tx data
	param[0x0E] = addr;
		
	res = sceSysconCmdExec(param, 0);

	if (res < 0)
		return(res);

	// rx data
	return((param[0x21]<<8) | param[0x20]);
}
int errCheck(u32 data)
{
	if((data & 0x80250000) == 0x80250000) // old way (data & 0x80000000) <- checking only for -1 wrather than specifically a syscon error
		return -1;
	else if(data & 0xffff0000)
		return ((data & 0xffff0000)>>16);

	return 0;
}

int ReadSerial(u16* serial) // read the serial into serial
{
	int err = 0;
	u32 data;

	// serial number is stored at address 0x07 and address 0x09
	data = readBat(0x07); // lower 16bit
	err = errCheck(data);
	if (!(err < 0))
	{
		serial[0] = (data &0xffff);
		data = readBat(0x09); // upper 16bit
		err = errCheck(data);
		if (!(err<0))
			serial[1] =  (data &0xffff);
		else
			err = data;
	}
	else
		err = data;

	return err;
}

int WriteSerial(u16* serial) // write the serial passed by serial
{
	int err = 0;

	err = writeBat(0x07, serial[0]); // lower 16bit
	if(!err)
		err = writeBat(0x09, serial[1]); // lower 16bit

	return err;
}

int ReadProm(int address, u16* pdata) // read an addres from the eeprom of the battery
{
	int err = 0;
	u32 data;

	// read the data at address
	// serial number is stored at address 0x07 and address 0x09
	data = readBat(address); // lower 16bit
	err = errCheck(data);
	if (!(err<0))
		pdata[0] = (data &0xffff);
	else
		err = data;

	return err;
}

int WriteProm(int address, u16* pdata) // write to an address of the battery eeprom
{
	int err = 0;
	err = writeBat(address, pdata[0]);

	return err;
}

/*
	IDStorage (code from IDStorage Manager by Chilly Willy)
*/
int pspIdStorageLookup(u16 key, u32 offset, void *buf, u32 len)
{
	int k1 = pspSdkSetK1(0);

	memset(buf, 0, len);
	int ret = sceIdStorageLookup(key, offset, buf, len);

	pspSdkSetK1(k1);
}
int pspIdStorageReadLeaf(u16 key, void *buf)
{
	int k1 = pspSdkSetK1(0);

	memset(buf, 0, 512);
	int ret = sceIdStorageReadLeaf(key, buf);

	pspSdkSetK1(k1);

	return ret;
}
int pspIdStorageWriteLeaf(u16 key, void *buf)
{
	int k1 = pspSdkSetK1(0);

	char buf2[512];

	int err = sceIdStorageReadLeaf(key, buf2);
	if(err < 0){sceIdStorageCreateLeaf(key);} /* key probably missing, make it */

	int ret = sceIdStorageWriteLeaf(key, buf);
	sceIdStorageFlush();

	pspSdkSetK1(k1);

	return ret;
}
int pspIdStorageCreateLeaf(u16 key)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceIdStorageCreateLeaf(key);
	sceIdStorageFlush();

	pspSdkSetK1(k1);

	return ret;
}
int pspIdStorageDeleteLeaf(u16 key)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceIdStorageDeleteLeaf(key);
	sceIdStorageFlush();

	pspSdkSetK1(k1);

	return ret;
}
/*
	NAND
*/
int pspNandGetPageSize(void) 
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandGetPageSize();

	pspSdkSetK1(k1);

	return ret;
}
int pspNandGetPagesPerBlock(void) 
{
	// returns 32

	int k1 = pspSdkSetK1(0);

	int ret = sceNandGetPagesPerBlock();

	pspSdkSetK1(k1);

	return ret;
}
int pspNandGetTotalBlocks(void) 
{
	// returns 2048 on psp phat and 4096 on psp slim

	int k1 = pspSdkSetK1(0);

	int ret = sceNandGetTotalBlocks();

	pspSdkSetK1(k1);

	return ret;
}
int pspNandIsBadBlock(u32 ppn) 
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandIsBadBlock(ppn);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandLock(int writeflag)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandLock(writeflag);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReadBlockWithRetry(u32 ppn, void *buf, void *buf2)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadBlockWithRetry(ppn, buf, buf2);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReadId(void *buf, SceSize size)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadId(buf, size);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReadPages(u32 ppn, void *buf, void *buf2, u32 count)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadPages(ppn, buf, buf2, count);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReadStatus(void)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadStatus();

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReset(int flag)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReset(flag);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandSetWriteProtect(int protectFlag)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandSetWriteProtect(protectFlag);

	pspSdkSetK1(k1);

	return ret;
}
void pspNandUnlock(void)
{
	int k1 = pspSdkSetK1(0);

	sceNandUnlock();

	pspSdkSetK1(k1);
}
int pspNandSetScramble(u32 magic)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandSetScramble(magic);

	pspSdkSetK1(k1);

	return ret;
}
u32 pspNandGetScramble()
{
	int k1 = pspSdkSetK1(0);

	u32 magic;
	u32 buf[4];
	u32 sha[5];

	buf[0] = *(vu32*)(0xBC100090);
	buf[1] = *(vu32*)(0xBC100094);
	buf[2] = *(vu32*)(0xBC100090)<<1;
	buf[3] = 0xD41D8CD9;

	sceKernelUtilsSha1Digest((u8*)buf, sizeof(buf), (u8*)sha);

	magic = (sha[0] ^ sha[3]) + sha[2];

	pspSdkSetK1(k1);

	return magic;
}
int pspNandReadAccess(u32 page, void* buffer)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadAccess(page, buffer, NULL, 1, 49);

	pspSdkSetK1(k1);

	return ret;
}
int pspNandReadExtraOnly(u32 page, void* buffer)
{
	int k1 = pspSdkSetK1(0);

	int ret = sceNandReadExtraOnly(page, buffer, 1);

	pspSdkSetK1(k1);

	return ret;
}
void pspReadNandBlock(u32 page, u8 *buffer)
{
	u32 i, j;

	for (i = 0; i < pspNandGetPagesPerBlock(); i++){
		for (j = 0; j < 4; j++){
			pspNandReadAccess(page, buffer);
			pspNandReadExtraOnly(page, buffer+512);
		}

		page++;
		buffer += 528;
	}
}
/*
	System Information
*/
void *pspUmdManGetUmdDrive(int driveNum)
{
	int k1 = pspSdkSetK1(0);
	void *ret = (void*)sceUmdManGetUmdDrive(driveNum);
	pspSdkSetK1(k1);

	return ret;
}
int pspUmdExecInquiryCmd(void *drive, u8 *param, u8 *buf)
{
	int k1 = pspSdkSetK1(0);
	int ret = sceUmdExecInquiryCmd(drive, param, buf);
	pspSdkSetK1(k1);

	return ret;
}
u32 pspGetTachyonVersion()
{
	int k1 = pspSdkSetK1(0);
	u32 ver = sceSysregGetTachyonVersion();
	pspSdkSetK1(k1);

	return ver;
}
u32 pspGetBaryonVersion()
{
	int k1 = pspSdkSetK1(0);
	u32 ver;
	sceSysconGetBaryonVersion(&ver);
	pspSdkSetK1(k1);

	return ver;
}
u32 pspGetPommelVersion()
{
	int k1 = pspSdkSetK1(0);
	u32 ver;
	sceSysconGetPommelVersion(&ver);
	pspSdkSetK1(k1);

	return ver;
}
u64 pspGetFuseId()
{
	return sceSysregGetFuseId();
}
u32 pspGetFuseConfig()
{
	return sceSysregGetFuseConfig();
}
u32 pspGetKirkVersion() { 
   int k1 = pspSdkSetK1(0);

   int ver, i = 0, val = *(u32 *)0xBDE00004;
   sceSysregKirkBusClockEnable();
   do{
       i++;
       sceKernelDelayThread(1000);
       ver = *(u32 *)0xBDE00004;
   }while ((ver == val) && (i < 10));
   
   pspSdkSetK1(k1);
   
   return ver;
}
u32 pspGetSpockVersion() { 
   int k1 = pspSdkSetK1(0);

   int ver, i = 0, val = *(u32 *)0xBDF00004;
   sceSysregAtaBusClockEnable();
   do{
       i++;
       sceKernelDelayThread(1000);
       ver = *(u32 *)0xBDF00004;
   }while ((ver == val) && (i < 10));
   
   pspSdkSetK1(k1);
   
   return ver;
}
int pspGetModel()
{
	return sceKernelGetModel();
}
int GetHardwareRevision()
{
	u32 tachyon = pspGetTachyonVersion();
	u32 baryon = pspGetBaryonVersion();

	if((tachyon == 0x00140000 && baryon == 0x00030600)) return 0x010100; // TA-079
	else if((tachyon == 0x00200000 && baryon == 0x00030600)) return 0x010101; // TA-079v2
	else if((tachyon == 0x00200000 && baryon == 0x00040600)) return 0x010102; // TA-079v3
	else if((tachyon == 0x00300000 && baryon == 0x00040600)) return 0x010200; // TA-081
	else if((tachyon == 0x00400000 && baryon == 0x00114000)) return 0x010300; // TA-082
	else if((tachyon == 0x00400000 && baryon == 0x00121000)) return 0x010400; // TA-086

	else if((tachyon == 0x00500000 && baryon == 0x0022B200)) return 0x020100; // TA-085
	else if((tachyon == 0x00500000 && baryon == 0x00234000)) return 0x020101; // TA-085v2
	else if((tachyon == 0x00500000 && baryon == 0x00243000)) return 0x020200; // TA-088
	else if((tachyon == 0x00600000 && baryon == 0x00243000)) return 0x020202; // TA-088v3
	else if((tachyon == 0x00500000 && baryon == 0x00263100)) return 0x020300; // TA-090

	else if((tachyon == 0x00600000 && baryon == 0x00263100)) return 0x030100; // TA-090v2
	
	return 0xffffff;
}
int GetModel()
{
	u8 region[1];
	pspIdStorageLookup(0x100, 0x3D, region, 1);

	int model = (sceKernelGetModel() + 1) * 1000;

	if(region[0] == 0x03) model += 0;	   // PSP-X000
	else if(region[0] == 0x04) model += 1; // PSP-X001
	else if(region[0] == 0x09) model += 2; // PSP-X002
	else if(region[0] == 0x07) model += 3; // PSP-X003, i don't think this is correct
	else if(region[0] == 0x05) model += 4; // PSP-X004, this may be wrong sometimes because 0x05 is both PSP-1003 and PSP-1004
	else if(region[0] == 0x06) model += 5; // PSP-X005
	else if(region[0] == 0x0A) model += 6; // PSP-X006
	else if(region[0] == 0x0B) model += 7; // PSP-X007
	else if(region[0] == 0x0C) model += 8; // PSP-X008
	else if(region[0] == 0x0D) model += 9; // PSP-X009
	else{model += 0;}

	return model;
}
char *GetRegion(char *buf)
{
	u8 region[1];
	pspIdStorageLookup(0x100, 0x3D, region, 1);

	if(region[0] == 0x03) sprintf(buf, "Japan");	   // PSP-X000
	else if(region[0] == 0x04) sprintf(buf, "America"); // PSP-X001
	else if(region[0] == 0x09) sprintf(buf, "Australia"); // PSP-X002
	else if(region[0] == 0x07) sprintf(buf, "UK"); // PSP-X003
	else if(region[0] == 0x05) sprintf(buf, "Europe"); // PSP-X004, this may be wrong sometimes because 0x05 is both PSP-1003 and PSP-1004
	else if(region[0] == 0x06) sprintf(buf, "Korea"); // PSP-X005
	else if(region[0] == 0x0A) sprintf(buf, "Hong Kong"); // PSP-X006
	else if(region[0] == 0x0B) sprintf(buf, "Taiwan"); // PSP-X007
	else if(region[0] == 0x0C) sprintf(buf, "Russia"); // PSP-X008
	else if(region[0] == 0x0D) sprintf(buf, "China"); // PSP-X009
	else sprintf(buf, "Unk %02X", region); // unknown

	return buf;
}
char *GetShippedFW(char *buf)
{
	pspIdStorageLookup(0x51, 0, buf, 5);

	if(buf[0] == 0)
		sprintf(buf, "N/A");

	return buf;
}
char *GetUMDFW(char *buf)
{
	u8 buf2[0x60];
	u8 param[4] = {0, 0, 0x60, 0};

	int ret = pspUmdExecInquiryCmd(pspUmdManGetUmdDrive(0), param, &buf2);
	
	if(ret < 0)
		sprintf(buf, "N/A");
	else
		sprintf(buf, "%c%c%c%c%c", buf2[36], buf2[37], buf2[38], buf2[39], buf2[40]);

	return buf;
}
char *GetMotherboard(char *buf)
{
	u32 tachyon = pspGetTachyonVersion();
	u32 baryon = pspGetBaryonVersion();

	if((tachyon == 0x00140000 && baryon == 0x00030600)) sprintf(buf, "TA-079"); // get the motherboard version
	else if((tachyon == 0x00200000 && baryon == 0x00030600)) sprintf(buf, "TA-079v2");
	else if((tachyon == 0x00200000 && baryon == 0x00040600)) sprintf(buf, "TA-079v3");
	else if((tachyon == 0x00300000 && baryon == 0x00040600)) sprintf(buf, "TA-081");
	else if((tachyon == 0x00400000 && baryon == 0x00114000)) sprintf(buf, "TA-082");
	else if((tachyon == 0x00400000 && baryon == 0x00121000)) sprintf(buf, "TA-086");

	else if((tachyon == 0x00500000 && baryon == 0x0022B200)) sprintf(buf, "TA-085");
	else if((tachyon == 0x00500000 && baryon == 0x00234000)) sprintf(buf, "TA-085v2");
	else if((tachyon == 0x00500000 && baryon == 0x00243000)) sprintf(buf, "TA-088");
	else if((tachyon == 0x00600000 && baryon == 0x00243000)) sprintf(buf, "TA-088v3");
	else if((tachyon == 0x00500000 && baryon == 0x00263100)) sprintf(buf, "TA-090");

	else if((tachyon == 0x00600000 && baryon == 0x00263100)) sprintf(buf, "TA-090v2");
	else sprintf(buf, "Unknown");

	return buf;
}
char *GetFWVersion(char *buf)
{
	u32 ver = sceKernelDevkitVersion();
	sprintf(buf, "%i.%02i", (ver >> 24) & 0xff, (ver >> 8) & 0xffff);

	return buf;
}
u8 *GetMACAddress(u8 *buf)
{
	sceWlanGetEtherAddr(buf);

	return buf;
}
int GetHENVersion()
{
	u32 hen = typhoonGetVersion();

	if((hen >> 24) != 0xDE)
		return 0;
	else
		return (hen & 0xff) + 1;
}
/*
	General
*/
int pspUtilsBufferCopyWithRange(void *dst, int dstSize, void *src, int srcSize, int cmd)
{
	int k1 = pspSdkSetK1(0);
	int ret = sceUtilsBufferCopyWithRange(dst, dstSize, src, srcSize, cmd);
	pspSdkSetK1(k1);

	return ret;
}
int GetKeyPressKernel(int wait)
{
	int k1 = pspSdkSetK1(0);
	
	SceCtrlData pad;
	int btn = 0;

	while(!btn)
	{
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(1);
		sceCtrlReadBufferPositive(&pad, 1);
		btn = pad.Buttons & 0xFFFF;
		if(!wait){break;}
		sceKernelDelayThread(10000);
	}

	pspSdkSetK1(k1);
	return btn;
}
int StopModule(char *modname)
{
	SceModule2* mod = sceKernelFindModuleByName(modname);

	if(mod)
	{
		sceKernelStopModule(mod->modid, NULL, NULL, NULL, NULL);
		sceKernelUnloadModule(mod->modid);
		return mod->modid;
	}
}
int pspModuleLoaded(char *modname)
{
	SceModule2* mod = sceKernelFindModuleByName(modname);
	
	if(mod)
		return 1;

	return 0;
}
/*
	Start/Stop
*/
int module_start(SceSize args, void *argp)
{
	return 0;
}

int module_stop()
{
	return 0;
}