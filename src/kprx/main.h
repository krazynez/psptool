typedef struct {
	unsigned char peripheral_device_type;
	unsigned char removable;
	unsigned char standard_ver;
	unsigned char atapi_response;
	unsigned int additional;
	char vendor_id[8];
	char product_id[16];
	char product_rev[4];
	char sony_spec[0x14];
} ATAPI_INQURIY;

ATAPI_INQURIY ai;

char *Directories[] = {
	"ms0:/ISO",
	"ms0:/ISO/VIDEO",
	"ms0:/MP_ROOT",
	"ms0:/MP_ROOT/100MNV01",
	"ms0:/MP_ROOT/101ANV01",
	"ms0:/MUSIC",
	"ms0:/PICTURE",
	"ms0:/PSP",
	"ms0:/PSP/COMMON",
	"ms0:/PSP/GAME",
	"ms0:/PSP/RSSCH",
	"ms0:/PSP/RSSCH/IMPORT",
	"ms0:/PSP/SAVEDATA",
	"ms0:/PSP/SYSTEM",
	"ms0:/PSP/THEME",
	"ms0:/SEPLUGINS",
	"ms0:/VIDEO"
};
int ReadSerial(u16* serial);
int WriteSerial(u16* serial);
int ReadProm(int address, u16* pdata);
int WriteProm(int address, u16* pdata);
int createDirs(void);

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
u8 *UMDDateCode(u8 *buf);

int pspUtilsBufferCopyWithRange(void *dst, int dstSize, void *src, int srcSize, int cmd);
int GetKeyPressKernel(int wait);
int StopModule(char *modulename);

int module_start(SceSize args, void *argp);
int module_stop();
