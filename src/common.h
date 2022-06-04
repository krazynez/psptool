#define PBPHEADERSIZE 60

#define	PARAM_SFO	0x08
#define	ICON0_PNG	0x0C
#define	ICON1_PMF	0x10
#define	PIC0_PNG	0x14
#define	PIC1_PNG	0x18
#define	SND0_AT3	0x1C
#define	DATA_PSP	0x20
#define	DATA_PSAR	0x24
#define	UNKNOWN1	0x28
#define	UNKNOWN2	0x2C
#define	UNKNOWN3	0x30
#define	UNKNOWN4	0x34
#define	UNKNOWN5	0x38

struct SZIPFileDataDescriptor
{
  u32 CRC32;
  u32 CompressedSize;
  u32 UncompressedSize;
} __attribute__((packed));

struct SZIPFileHeader
{
  char Sig[4];
  u16 VersionToExtract;
  u16 GeneralBitFlag;
  u16 CompressionMethod;
  u16 LastModFileTime;
  u16 LastModFileDate;
  struct SZIPFileDataDescriptor DataDescriptor;
  u16 FilenameLength;
  u16 ExtraFieldLength;
}  __attribute__((packed));