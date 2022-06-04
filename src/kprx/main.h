int ReadSerial(u16* serial);
int WriteSerial(u16* serial);
int ReadProm(int address, u16* pdata);
int WriteProm(int address, u16* pdata);

int pspIdStorageLookup(u16 key, u32 offset, void *buf, u32 len);
int pspIdStorageReadLeaf(u16 key, void *buf);
int pspIdStorageWriteLeaf(u16 key, void *buf);
int pspIdStorageCreateLeaf(u16 key);
int pspIdStorageDeleteLeaf(u16 key);

u32 pspGetTachyonVersion();
u32 pspGetBaryonVersion();
u32 pspGetPommelVersion();
u64 pspGetFuseId();
u32 pspGetFuseConfig();
u32 pspGetKirkVersion();
u32 pspGetSpockVersion();
int pspGetModel();
int GetHardwareRevision();
int GetModel();
char *GetRegion(char *buf);
char *GetShippedFW(char *buf);
char *GetMotherboard(char *buf);
char *GetFWVersion(char *buf);
u8 *GetMACAddress(u8 *buf);

int pspUtilsBufferCopyWithRange(void *dst, int dstSize, void *src, int srcSize, int cmd);
int GetKeyPressKernel(int wait);
int StopModule(char *modulename);

int module_start(SceSize args, void *argp);
int module_stop();