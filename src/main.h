int OnBackToMainMenu(int enter);
void ResetScreen(int showmenu, int showback, int sel);
void MainMenu();

void ExtractIPL(char *outfile, int type);
void EraseIPL();
void InjectIPL(char *filename, int type);
void CheckIPL(char *input, int type);
void CreateCheckSum(char *input, char *outfile, int type);

void SetBatSer(u16 ser1, u16 ser2);
u32 GetBatSer();
void RestoreBatSer(char *infile);
void BackupBat(char *outfile);
void RestoreBat(char *infile);

void BackupIdStorage(char *filename);
void RestoreIdStorage(char *filename);

void About();
void ConnectUSB(int enable, u32 device);
void CheckMSInfo(int page);
int GetMSInfo(char *input, int type);
int CheckSysInfo(int page);
void CheckSysInfoP1();
void CheckSysInfoP2();

char *Quotes[] = {
	"42 of course!",
	"ARK-4 Team was here....",
	"Code so clean you can eat off it.",
	"If I can't fix it, it isn't broken.",
	"Never test for a bug you don't know how to fix.",
	"I think therefore I create bugs.",
	"Debug is human, de-fix divine.",
	"There's a bug born every minute, and two to replace him.",
	"The Bugs Of Wrath",
	"There are 2 ways to write bug-free code; only the 3rd way works.",
	"Final message received from the Titanic: Fatal crash due to icebug.",
	"Bugs Bunny was an optimist.",
	"One small bug for man, one great program for mankind.",
	"The bug is mightier than the fix.",
	"Man does not live by bug fixes alone.",
	"For every bug fixed, there is a bigger bug not yet discovered.",
	"The bug stops here.",
	"I have just begun to debug.",
	"Bugs bugs everywhere, and not a fix in sight.",
	"A feature is a bug with seniority.",
	"Human knowledge belongs to the world.",
	"Programmers don't die, they just GOSUB without RETURN.",
	"Debuggers have been made by bug-creators.",
	"There are 3 kinds of people in the world: Those who can count and those who can't.",
	"Windows is a set of 32 bit extensions on a 16 bit shell for an 8 bit OS using a 4 bit kernel made by a 2 bit company that can't stand 1 bit of competition.",
	"Not everything that can be counted counts, and not everything that counts can be counted."
};

typedef struct {
	u8 BootStatus;
	u8 StartHead;
	u16 StartSector;
	u8 PartitionType;
	u8 LastHead;
	u16 LastSector;
	u32 AbsSector;
	u32 TotalSectors;
	u16 Signature;
	int IPLSpace;
	u32 SerialNum;
	char Label[12];
	char FileSystem[9];
	char IPLName[50];
	int IPLSize;
} MSStruct;

/*static u8 g_dataPSAR[6291456] __attribute__((aligned(64))); 
static u8 g_dataOut[2000000] __attribute__((aligned(0x40)));
static u8 g_dataOut2[2000000] __attribute__((aligned(0x40)));*/

#define BIG_BUFFER_SIZE		10485760
#define SMALL_BUFFER_SIZE	2000000
#define EVEN_SMALL_BUFFER_SIZE	0x8000

static u8 big_buffer[BIG_BUFFER_SIZE] __attribute__((aligned(0x40))); 
static u8 sm_buffer1[SMALL_BUFFER_SIZE] __attribute__((aligned(0x40)));
static u8 sm_buffer2[SMALL_BUFFER_SIZE] __attribute__((aligned(0x40)));
static u8 sm_buffer3[0x60];// __attribute__((aligned(0x40)));

enum
{
	MODE_ENCRYPT_SIGCHECK,
	MODE_ENCRYPT,
	MODE_DECRYPT
};

#define vlf_text_items 20
VlfText vlf_texts[vlf_text_items];
VlfPicture vlf_picture = NULL;
VlfProgressBar vlf_progressbar = NULL;
VlfShadowedPicture waiticon = NULL;

VlfText titletext = NULL;
VlfPicture titlepicture = NULL;

u32 EBOOT_PSAR, EBOOT_UNKNOWN1, EBOOT_UNKNOWN2, EBOOT_UNKNOWN3, EBOOT_UNKNOWN4, EBOOT_UNKNOWN5;
