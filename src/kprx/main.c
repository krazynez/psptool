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
	return ret;
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
void* (*_sceUmdManGetUmdDrive)(int) = NULL;
void *pspUmdManGetUmdDrive(int driveNum)
{
	int k1 = pspSdkSetK1(0);
	if(pspGetModel() == 4 || pspGetModel() == 5) {
		pspSdkSetK1(k1);
		return NULL;
	}
	_sceUmdManGetUmdDrive = (void *)sctrlHENFindFunction("sceUmdMan_driver", "sceUmdMan_driver", 0x47E2B6D8);
	void *ret = _sceUmdManGetUmdDrive(driveNum);
	pspSdkSetK1(k1);

	return ret;
}
int (*_sceUmdExecInquiryCmd)(void*, u8*, u8*) = NULL;
int pspUmdExecInquiryCmd(void *drive, u8 *param, u8 *buf)
{
	int k1 = pspSdkSetK1(0);
	if(pspGetModel() == 4 || pspGetModel() == 5) {
		pspSdkSetK1(k1);
		return -1;
	}
	_sceUmdExecInquiryCmd = (void *)sctrlHENFindFunction("sceUmdMan_driver", "sceUmdMan_driver", 0x1B19A313);
	int ret = _sceUmdExecInquiryCmd(drive, param, buf);
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
int (*_sceSysregAtaBusClockEnable)(void) = NULL;
u32 pspGetSpockVersion() { 
   int k1 = pspSdkSetK1(0);

   int ver, i = 0, val = *(u32 *)0xBDF00004;
   _sceSysregAtaBusClockEnable = (void *)sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0x16909002);
   _sceSysregAtaBusClockEnable();
 
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

	// 01g
	if((tachyon == 0x00140000 && baryon == 0x00010600)) return 0x010100; // TA-079v1
	else if((tachyon == 0x00140000 && baryon == 0x00020600)) return 0x010101; // TA-079v2
	else if((tachyon == 0x00140000 && baryon == 0x00020600)) return 0x010102; // TA-079v3
	else if((tachyon == 0x00140000 && baryon == 0x00030600)) return 0x010102; // TA-079v3
	else if((tachyon == 0x00200000 && baryon == 0x00030600)) return 0x010103; // TA-079v4
	else if((tachyon == 0x00200000 && baryon == 0x00040600)) return 0x010104; // TA-079v4
	else if((tachyon == 0x00300000 && baryon == 0x00040600)) return 0x010200; // TA-081v1
	else if((tachyon == 0x00300000 && baryon == 0x00040600) && pspGetPommelVersion() == 0x00000104) return 0x010200; // TA-081v2
	else if((tachyon == 0x00400000 && baryon == 0x00114000)) return 0x010300; // TA-082
	else if((tachyon == 0x00400000 && baryon == 0x00121000)) return 0x010400; // TA-086

	// 02g
	else if((tachyon == 0x00500000 && baryon == 0x0022B200)) return 0x020100; // TA-085v1
	else if((tachyon == 0x00500000 && baryon == 0x00234000)) return 0x020101; // TA-085v2
	else if((tachyon == 0x00500000 && baryon == 0x00243000)) return 0x020200; // TA-088v1/v2
	else if((tachyon == 0x00600000 && baryon == 0x00243000)) return 0x020202; // TA-088v3
	else if((tachyon == 0x00500000 && baryon == 0x00243000)) return 0x020300; // TA-090v1 (?)

	// 03g/04g/07g/09g
	else if((tachyon == 0x00600000 && baryon == 0x00263100 && pspGetPommelVersion() == 0x00000132)) return 0x030101; // TA-090v2
	else if((tachyon == 0x00600000 && baryon == 0x00263100 && pspGetPommelVersion() == 0x00000133)) return 0x030102; // TA-090v3
	else if((tachyon == 0x00600000 && baryon == 0x00285000)) return 0x030200; // TA-092v1
	else if((tachyon == 0x00810000 && baryon == 0x002C4000 && pspGetPommelVersion() == 0x00000143)) return 0x030301; // TA-093v2
	else if((tachyon == 0x00810000 && baryon == 0x002E4000)) return 0x090100; // TA-095v1
	else if((tachyon == 0x00820000 && baryon == 0x002E4000)) return 0x090101; // TA-095v2
	else if((tachyon == 0x00810000 && baryon == 0x012E4000)) return 0x070102; // TA-095v3
	else if((tachyon == 0x00820000 && baryon == 0x012E4000)) return 0x070103; // TA-095v4

	// 05g/06g
	else if((tachyon == 0x00720000 && baryon == 0x00304000)) return 0x050100; // TA-091v1
	else if((tachyon == 0x00810000 && baryon == 0x00323000)) return 0x060100; // TA-094v1
	else if((tachyon == 0x00810000 && baryon == 0x00324000)) return 0x060101; // TA-094v2
	
	// 11g
	else if((tachyon == 0x00900000 && baryon == 0x00403000)) return 0x060101; // TA-096v1/v2
																			  
	return 0xffffff;
}
int GetModel()
{
	u8 region[1];
	// https://github.com/Yoti/psp_pspident/blob/2aa209cb164b5a19f38f4f2dc281fe3ff913ceef/ident_pbp/kernel.c#L195
	memset(region, 0, sizeof(region));
	pspIdStorageLookup(0x0100, 0xF5, &region, 1);

	int k1 = pspSdkSetK1(0);
	SceCtrlData pad;
	int model = (pspGetModel() + 1) * 1000;

	sceCtrlReadBufferPositive(&pad, 1);
	if(!(pad.Buttons & PSP_CTRL_LTRIGGER)) {
		if(model == 4000 || model == 9000 || model == 7000)
			model = 3000;
		if (model == 5000 || model == 11000)
			model = 1000;
	}
	pspSdkSetK1(k1);

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
	else if(region[0] == 0x08) model += 10; // PSP-X010
	else{model += 0;}

	return model;
}
char *GetRegion(char *buf)
{
	u8 region[1];
	// https://github.com/Yoti/psp_pspident/blob/2aa209cb164b5a19f38f4f2dc281fe3ff913ceef/ident_pbp/kernel.c#L195
	pspIdStorageLookup(0x0100, 0xF5, region, 1);

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
	else if(region[0] == 0x08) sprintf(buf, "Mexico"); // PSP-X010
	else sprintf(buf, "Unk %02X", region); // unknown

	return buf;
}
char *GetShippedFW(char *buf)
{
	memset(buf, 0, sizeof(buf));
	pspIdStorageLookup(0x51, 0, buf, 4);

	if(buf[0] == 0)
		sprintf(buf, "N/A");

	return buf;
}
char *GetBTMAC(char *buf) {
	u8 idsbtmac[6];
	pspIdStorageLookup(0x0050, 0x41, idsbtmac, 6);
	sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", idsbtmac[0], idsbtmac[1], idsbtmac[2], idsbtmac[3], idsbtmac[4], idsbtmac[5]);

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


	if((tachyon == 0x00140000 && baryon == 0x00010601)) sprintf(buf, "?TMU0--1?"); // get the motherboard version
	else if((tachyon == 0x00140000 && baryon == 0x00020601)) sprintf(buf, "TMU-001");
	else if((tachyon == 0x00140000 && baryon == 0x00030601)) sprintf(buf, "TMU-002");
	else if((tachyon == 0x00140000 && baryon == 0x00010600)) sprintf(buf, "TA-079v1");
	else if((tachyon == 0x00140000 && baryon == 0x00020600)) sprintf(buf, "TA-079v2");
	else if((tachyon == 0x00140000 && baryon == 0x00030600)) sprintf(buf, "TA-079v3");
	else if((tachyon == 0x00200000 && baryon == 0x00030600)) sprintf(buf, "TA-079v4");
	else if((tachyon == 0x00200000 && baryon == 0x00040600)) sprintf(buf, "TA-079v5");
	else if((tachyon == 0x00300000 && baryon == 0x00040600)) sprintf(buf, "TA-081v1");
	else if((tachyon == 0x00300000 && baryon == 0x00040600 && pspGetPommelVersion() == 0x00000104)) sprintf(buf, "TA-081v2");
	else if((tachyon == 0x00400000 && baryon == 0x00114000)) sprintf(buf, "TA-082");
	else if((tachyon == 0x00400000 && baryon == 0x00121000)) sprintf(buf, "TA-086");

	else if((tachyon == 0x00500000 && baryon == 0x0022B200)) sprintf(buf, "TA-085v1");
	else if((tachyon == 0x00500000 && baryon == 0x00234000)) sprintf(buf, "TA-085v2");
	else if((tachyon == 0x00500000 && baryon == 0x00243000)) sprintf(buf, "TA-088v1/v2");
	else if((tachyon == 0x00600000 && baryon == 0x00243000)) sprintf(buf, "TA-088v3");
	else if((tachyon == 0x00500000 && baryon == 0x00263000)) sprintf(buf, "TA-090v1");

	else if((tachyon == 0x00600000 && baryon == 0x00263100 && pspGetPommelVersion() == 0x00000132)) sprintf(buf, "TA-090v2");
	else if((tachyon == 0x00600000 && baryon == 0x00263100 && pspGetPommelVersion() == 0x00000133)) sprintf(buf, "TA-090v3");
	else if((tachyon == 0x00600000 && baryon == 0x00285000)) sprintf(buf, "TA-092");
	else if((tachyon == 0x00810000 && baryon == 0x002C4000 && pspGetPommelVersion() == 0x00000141)) sprintf(buf, "TA-093v1");
	else if((tachyon == 0x00810000 && baryon == 0x002C4000 && pspGetPommelVersion() == 0x00000143)) sprintf(buf, "TA-093v2");
	else if((tachyon == 0x00810000 && baryon == 0x002E4000)) sprintf(buf, "TA-095v1");
	else if((tachyon == 0x00820000 && baryon == 0x002E4000)) sprintf(buf, "TA-095v2");
	else if((tachyon == 0x00810000 && baryon == 0x012E4000)) sprintf(buf, "TA-095v3");
	else if((tachyon == 0x00820000 && baryon == 0x012E4000)) sprintf(buf, "TA-095v4");


	else if((tachyon == 0x00720000 && baryon == 0x00304000)) sprintf(buf, "TA-091");
	else if((tachyon == 0x00810000 && baryon == 0x00323000)) sprintf(buf, "TA-094v1");
	else if((tachyon == 0x00810000 && baryon == 0x00324000)) sprintf(buf, "TA-094v2");


	else if((tachyon == 0x00900000 && baryon == 0x00403000)) sprintf(buf, "TA-096/097");

	else sprintf(buf, "Unk: tachyon: %08X baryon: %08X", tachyon, baryon);

	return buf;
}
char *GetFWVersion(char *buf)
{
	u32 ver = sceKernelDevkitVersion();
	int maj = (ver >> 24) & 0xFF;
    int min = (ver >> 16) & 0xFF;
    int pat = (ver >> 8) & 0xFF;	
	sprintf(buf, "%d.%d%d", maj, min, pat);

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
