#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <pspdebug.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pspctrl.h>
#include <pspusbdevice.h>
#include <pspusbstor.h>
#include <pspusb.h>
#include <pspopenpsid.h>

#include <vlf.h>
#include <pandorafiles.h>
#include <main.h>
#include <common.h>
#include <kprx/main.h>

/*
	TODO:
	update motherboards to support all known ones
*/

PSP_MODULE_INFO("PSP Tool", 0, 1, 69);
PSP_MAIN_THREAD_ATTR(0);


char *mode = "Main";

char *path;
char ebootpath[256];

MSStruct MSInfo;
int tmmode, selitem, bguseflash = 0, wavespeed = 1, showback_prev = 0, showenter_prev = 0;

static int go = -1;

/*
	Callbacks
*/
int exit_callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();

	return 0;
}

int SetupCallbacks()
{
	int thid = 0;
	
	thid = sceKernelCreateThread("exit_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	
	if(thid >= 0){sceKernelStartThread(thid, 0, 0);}
	
	return thid;
}
/*
	Menus & VLF
*/
void SetTitle(char *rco, char *name, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if(titletext != NULL){titletext = vlfGuiRemoveText(titletext);}

	if(titlepicture != NULL){titlepicture = vlfGuiRemovePicture(titlepicture);}

	titletext = vlfGuiAddText(0, 0, msg);
	titlepicture = vlfGuiAddPictureResource(rco, name, 4, -2);

	vlfGuiSetTitleBarEx(titletext, titlepicture, 1, 0, NULL);

	return;
}
void SetStatus(int showback, int num, int x, int y, int alignment, char *fmt, ...)
{
	va_list list;
	char msg[256];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if(num == 0){ResetScreen(0, showback, 0);}
	if(vlf_texts[num] != NULL){vlf_texts[num] = vlfGuiRemoveText(vlf_texts[num]);}

	vlf_texts[num] = vlfGuiAddText(x, y, msg);
	vlfGuiSetTextAlignment(vlf_texts[num], alignment);

	vlfGuiDrawFrame();

	return;
}
void SetProgress(int value, int max)
{
	// TODO: The progress bar is probably not showing the correct value because it doesn't instantly set the progress to the percent value so it must probably be done on a separate thread where it can continue to progress

	if(max == 0){if(vlf_progressbar != NULL){vlf_progressbar = vlfGuiRemoveProgressBar(vlf_progressbar);}return;}
	//if(vlf_progressbar == NULL){vlf_progressbar = vlfGuiAddProgressBar(136);}

	int percent = ((100 * value) / max);
	SetStatus(0, 1, 240, 148, VLF_ALIGNMENT_CENTER, "%i%%", percent);
	//if(vlf_progressbar != NULL){vlfGuiProgressBarSetProgress(vlf_progressbar, 10);}
}
void ErrorReturn(char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	sceKernelVolatileMemUnlock(0);

	vlfGuiMessageDialog(msg, VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);
	OnBackToMainMenu(0);

	return;
}
void ResetScreen(int showmenu, int showback, int sel)
{
	int i;

	for(i = 0; i < vlf_text_items; i++){if(vlf_texts[i] != NULL){vlf_texts[i] = vlfGuiRemoveText(vlf_texts[i]);}}
	if(vlf_picture != NULL){vlf_picture = vlfGuiRemovePicture(vlf_picture);}
	if(vlf_progressbar != NULL){vlf_progressbar = vlfGuiRemoveProgressBar(vlf_progressbar);}
	vlfGuiCancelCentralMenu();
	if((vlfGuiGetButtonConfig() && showback_prev) || (!vlfGuiGetButtonConfig() && showenter_prev)){vlfGuiCancelBottomDialog();showback_prev = 0;showenter_prev = 0;}
	
	if(showmenu){MainMenu(sel);}
	if(showback && !showback_prev){vlfGuiBottomDialog(VLF_DI_BACK, -1, 1, 0, VLF_DEFAULT, OnBackToMainMenu);showback_prev = 1;}
}
int OnBackToMainMenu(int enter)
{
	if(waiticon != NULL){waiticon = vlfGuiRemoveShadowedPicture(waiticon);}
	waiticon = vlfGuiAddWaitIcon();

	if(!enter){
		if(mode == "USB Connection"){ConnectUSB(0, -1);mode = "Main";}
		else if(mode == "Memory Stick Options"){mode = "Main";}
		else if(mode == "Battery Options"){mode = "Main";}
		else if(mode == "IdStorage Options"){mode = "Main";}
		else if(mode == "About"){mode = "Main";}
		else if(mode == "Inject IPL to Memory Stick"){mode = "Memory Stick Options";}
		else if(mode == "Check Memory Stick Information"){mode = "Memory Stick Options";}
		else if(mode == "Extract Memory Stick Data"){mode = "Memory Stick Options";}
		else if(mode == "Create Magic Memory Stick"){mode = "Memory Stick Options";}
		else if(mode == "USB Connection.1"){ConnectUSB(0, -1);mode = "USB Connection";}
		else if(mode == "Memory Stick Options.1"){mode = "Memory Stick Options";}
		else if(mode == "Inject IPL to Memory Stick.1"){mode = "Inject IPL to Memory Stick";}
		else if(mode == "Check Memory Stick Information.1"){mode = "Check Memory Stick Information";}
		else if(mode == "Extract Memory Stick Data.1"){mode = "Extract Memory Stick Data";}
		else if(mode == "Create Magic Memory Stick.1"){mode = "Create Magic Memory Stick";}
		else if(mode == "Battery Options.1"){mode = "Battery Options";}
		else if(mode == "IdStorage Options.1"){mode = "IdStorage Options";}
		else if(mode == "About.1"){mode = "About";}
		else{mode == "Main";}

		vlfGuiNextPageControl(NULL); // We'll assign the pages to nothing first
		vlfGuiPreviousPageControl(NULL);
		vlfGuiCancelPreviousPageControl(); // And now we'll unassign them (if they aren't assigned it causes a crash)
		vlfGuiCancelNextPageControl();
		vlfGuiSetPageControlEnable(0);
		
		if(mode == "Main"){ResetScreen(1, 0, selitem);}
		else{ResetScreen(1, 1, selitem);}

		vlfGuiSetRectangleFade(0, VLF_TITLEBAR_HEIGHT, 480, 272-VLF_TITLEBAR_HEIGHT, VLF_FADE_MODE_IN, VLF_FADE_SPEED_VERY_FAST, 0, NULL, NULL, 0);
	}

	if(waiticon != NULL){waiticon = vlfGuiRemoveShadowedPicture(waiticon);}

	return;
}
int OnMainMenuSelect(int sel)
{
	ResetScreen(0, 0, 0);
	selitem = sel;

	if(waiticon != NULL){waiticon = vlfGuiRemoveShadowedPicture(waiticon);}
	waiticon = vlfGuiAddWaitIcon();

	if (mode == "Main"){
		if(sel == 0){if(FileExists("flash0:/kd/usbdevice.prx") || pspModuleLoaded("pspUsbDev_Driver")){mode = "USB Connection";ResetScreen(1, 1, 0);}else{mode = "USB Connection";ResetScreen(0, 1, 0);ConnectUSB(1, -1);}}
		else if(sel == 1){mode = "Memory Stick Options";ResetScreen(1, 1, 0);} // show memory stick menu
		else if(sel == 2){if(pspGetBaryonVersion() >= 0x00234000){ErrorReturn("The PSP hardware does not support reading or writing to the Battery EEPROM.");}else{mode = "Battery Options";ResetScreen(1, 1, 0);}} // show battery menu
		else if(sel == 3){mode = "IdStorage Options";ResetScreen(1, 1, 0);} // show idstorage menu
		else if(sel == 4){mode = "About";ResetScreen(1, 1, 0);} // show about menu
	}
	else if (mode == "USB Connection"){
		mode = "USB Connection.1";
		if(sel == 0){ConnectUSB(1, -1);} // connect ms0 to usb
		else if(sel == 1){ConnectUSB(1, PSP_USBDEVICE_FLASH0);} // connect flash0 to usb
		else if(sel == 2){ConnectUSB(1, PSP_USBDEVICE_FLASH1);} // connect flash1 to usb
		else if(sel == 3){ConnectUSB(1, PSP_USBDEVICE_FLASH2);} // connect flash2 to usb
		else if(sel == 4){ConnectUSB(1, PSP_USBDEVICE_FLASH3);} // connect flash3 to usb
		else if(sel == 5){ConnectUSB(1, PSP_USBDEVICE_UMD9660);} // connect umd disc to usb
	}
	else if(mode == "Memory Stick Options"){
		mode = "Memory Stick Options.1";
		if(sel == 0){mode = "Inject IPL to Memory Stick";ResetScreen(1, 1, 0);} // show ipl injection menu
		else if(sel == 1){EraseIPL();} // show ipl erase menu
		else if(sel == 2){mode = "Check Memory Stick Information";ResetScreen(1, 1, 0);} // show memory stick check menu
		else if(sel == 3){mode = "Extract Memory Stick Data";ResetScreen(1, 1, 0);} // show extraction menu
		else if(sel == 4){FormatMS("PANDORA", 1024);} // format the memory stick
		else if(sel == 5){mode = "Create Magic Memory Stick";ResetScreen(1, 1, 0);} // show magic memory stick menu
	}
	else if(mode == "Battery Options"){
		mode = "Battery Options.1";
		if(sel == 0){ShowBatSer();} // check battery serial
		else if(sel == 1){BackupBat("ms0:/eeprom.bin");} // backup battery eeprom to ms0:/eeprom.bin
		else if(sel == 2){RestoreBat("ms0:/eeprom.bin");} // restore battery eeprom from ms0:/eeprom.bin
		else if(sel == 3){RestoreBatSer("ms0:/eeprom.bin");} // restore battery serial from ms0:/eeprom.bin
		else if(sel == 4){SetBatSer(0x5241, 0x4E44);} // convert to normal battery (generates a random serial, when those params are passed)
		else if(sel == 5){SetBatSer(0xFFFF, 0xFFFF);} // convert to service mode battery (0xFFFFFFFF)
		else if(sel == 6){SetBatSer(0x0000, 0x0000);} // convert to autoboot battery (0x00000000)
	}
	else if(mode == "IdStorage Options"){
		mode = "IdStorage Options.1";
		if(sel == 0){BackupIdStorage("ms0:/keys");} // backup idstorage to ms0:/keys_SER
		else if(sel == 1){RestoreIdStorage("ms0:/keys");} // restore idstorage from ms0:/keys_SER
	}
	else if(mode == "About"){
		mode = "About.1";
		if(sel == 0){CheckSysInfo(0);} // show system information
		else if(sel == 1){About();} // show software information
	}
	else if(mode == "Inject IPL to Memory Stick"){
		mode = "Inject IPL to Memory Stick.1";
		if(sel == 0){InjectIPL("IPLs/Time Machine.bin", 0);} // inject time machine ipl
		else if(sel == 1){InjectIPL("IPLs/Boosters Multi IPL.bin", 0);} // inject boosters multi ipl
		else if(sel == 2){InjectIPL("IPLs/z3ros0ul Single IPL.bin", 0);} // inject z3ros0ul single ipl
		else if(sel == 3){InjectIPL("IPLs/z3ros0ul Multi IPL.bin", 0);} // inject z3ros0ul multi ipl
		else if(sel == 4){InjectIPL("IPLs/Classic Pandora IPL.bin", 0);} // inject classic pandora ipl
		else if(sel == 5){InjectIPL("ms0:/ipl.bin", 1);} // inject ms0:/ipl.bin
	}
	else if(mode == "Check Memory Stick Information"){
		mode = "Check Memory Stick Information.1";
		if(sel == 0){CheckIPL("msstor:", 0);} // check injected ipl
		else if(sel == 1){CheckIPL("ms0:/ipl.bin", 1);} // check ms0:/ipl.bin
		else if(sel == 2){CheckMSInfo(0);} // check ms mbr
		else if(sel == 3){CreateCheckSum("msstor:", "ms0:/checksum.bin", 0x2000);} // create a checksum of the first 4096 bytes of the injected ipl
		else if(sel == 4){CreateCheckSum("ms0:/ipl.bin", "ms0:/checksum.bin", 0);} // create a checksum of the first 4096 bytes of the ipl at ms0:/ipl.bin
	}
	else if(mode == "Extract Memory Stick Data"){
		mode = "Extract Memory Stick Data.1";
		if(sel == 0){ExtractIPL("ms0:/ipl.bin", 0);} // extract ipl from ms to ms0:/ipl.bin
		else if(sel == 1){ExtractIPL("ms0:/ipl.bin", 1);} // extract entire ipl space from ms to ms0:/ipl.bin
	}
	else if(mode == "Create Magic Memory Stick"){
		mode = "Create Magic Memory Stick.1";
		if(sel == 0){CreateMMS("150 Update Flasher");} // create 1.50 update flasher mms
		else if(sel == 1){CreateMMS("DC3");} // create despertar del cementerio 3 mms
		else if(sel == 2){CreateMMS("DC4");} // create despertar del cementerio 4 mms
		else if(sel == 3){CreateMMS("DC5");} // create despertar del cementerio 5 mms
		else if(sel == 4){CreateMMS("DC7");} // create despertar del cementerio 7 mms
		else if(sel == 5){CreateMMS("DC8");} // create despertar del cementerio 8 mms
		else if(sel == 6){CreateMMS("DC9");} // create despertar del cementerio 9 mms
		else if(sel == 7){CreateMMS("DC10");} // create despertar del cementerio 10 mms
	}

	if(waiticon != NULL){waiticon = vlfGuiRemoveShadowedPicture(waiticon);}

    return;
}

void MainMenu(int sel)
{
	SetTitle("sysconf_plugin", "tex_bar_init_icon", "%s v%i.%02i", module_info.modname, module_info.modversion[1], module_info.modversion[0]);

	if(mode == "Main"){
		char *items[] ={"USB Connection",
						"Memory Stick Options",
						"Battery Options",
						"IdStorage Options",
						"About"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
	}
	else if(mode == "USB Connection"){
		SetTitle("sysconf_plugin", "tex_bar_usb_icon", "%s v%i.%02i", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
		char *items[] ={"Memory Stick",
						"Flash 0",
						"Flash 1",
						"Flash 2",
						"Flash 3",
						"UMD Disc"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 0;
	}
	else if(mode == "Memory Stick Options"){
		char *items[] ={"Inject IPL to Memory Stick",
						"Erase IPL from Memory Stick",
						"Check Memory Stick Information",
						"Extract Memory Stick Data",
						"Format Memory Stick",
						"Create Magic Memory Stick"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 1;
	}
	else if(mode == "Battery Options"){
		char *items[] ={"Check Battery Serial",
						"Backup Battery EEPROM",
						"Restore Battery EEPROM",
						"Restore Battery Serial",
						"Convert to Normal Battery",
						"Convert to Service Mode Battery",
						"Convert to AutoBoot Battery"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 2;
	}
	else if(mode == "IdStorage Options"){
		char *items[] ={"Backup IdStorage",
						"Restore IdStorage"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 3;
	}
	else if(mode == "About"){
		SetTitle("video_plugin_videotoolbar.rco", "tex_help_bar_icon", "%s v%i.%02i", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
		char *items[] ={"System Information",
						"Software Information"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 4;
	}
	else if(mode == "Inject IPL to Memory Stick"){
		char *items[] ={"Inject Time Machine IPL",
						"Inject Boosters Multi IPL",
						"Inject z3ros0ul Single IPL (Sleep Fix)",
						"Inject z3ros0ul Multi IPL (Sleep Fix)",
						"Inject Classic Pandora IPL",
						"Inject IPL from ms0:/ipl.bin"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 0;
	}
	else if(mode == "Check Memory Stick Information"){
		char *items[] ={"Check Injected IPL",
						"Check IPL at ms0:/ipl.bin",
						"Check Memory Stick MBR",
						"Create Checksum of Injected IPL",
						"Create Checksum of ms0:/ipl.bin"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 2;
	}
	else if(mode == "Extract Memory Stick Data"){
		char *items[] ={"Extract Memory Stick IPL",
						"Extract Entire Memory Stick IPL Space"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 3;
	}
	else if(mode == "Create Magic Memory Stick"){
		SetTitle("update_plugin.rco", "tex_update_icon", "%s v%i.%02i", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
		char *items[] ={"1.50 Update Flasher (Original Pandora)",
						"Despertar del Cementerio v3 (3.71 M33-2)",
						"Despertar del Cementerio v4 (3.80 M33-5)",
						"Despertar del Cementerio v5 (3.90 M33-3)",
						"Despertar del Cementerio v7 (4.01 M33-2)",
						"Despertar del Cementerio v8 (5.00 M33-4)",
						"Despertar del Cementerio v9 (5.02 M33-5)",
						"Despertar del Cementerio v10 (6.61 ARK-4)"};
		vlfGuiCentralMenu(sizeof(items) / sizeof(items[0]), items, sel, OnMainMenuSelect, 0, 0);
		selitem = 5;
	}

	if(!showenter_prev){
		vlfGuiBottomDialog(-1, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnBackToMainMenu);
		showenter_prev = 1;
	}

	vlfGuiSetRectangleFade(0, VLF_TITLEBAR_HEIGHT, 480, 272-VLF_TITLEBAR_HEIGHT, VLF_FADE_MODE_IN, VLF_FADE_SPEED_VERY_FAST, 0, NULL, NULL, 0);

	return;
}
int app_main(SceSize args, void *argp)
{
	SetupCallbacks();

	sprintf(ebootpath, "%s/EBOOT.PBP", path);
	ReadFile(ebootpath, 0, big_buffer, PBPHEADERSIZE);
	EBOOT_PSAR = GetEBOOToffsetBuff(big_buffer, DATA_PSAR);
	EBOOT_UNKNOWN1 = GetEBOOToffsetBuff(big_buffer, UNKNOWN1);
	EBOOT_UNKNOWN2 = GetEBOOToffsetBuff(big_buffer, UNKNOWN2);
	EBOOT_UNKNOWN3 = GetEBOOToffsetBuff(big_buffer, UNKNOWN3);
	EBOOT_UNKNOWN4 = GetEBOOToffsetBuff(big_buffer, UNKNOWN4);
	EBOOT_UNKNOWN5 = GetEBOOToffsetBuff(big_buffer, UNKNOWN5);

	if(FileExists("flash0:/kd/resurrection.prx") || FileExists("flash0:/kd/pspbtcnf_dc.bin") || FileExists("flash0:/kd/hibari.prx") || DirExists("flach0:/")){tmmode = 1;}

	vlfGuiCacheResource("system_plugin");
	vlfGuiCacheResource("system_plugin_fg");

	vlfGuiSetModelSystem();

	vlfGuiSystemSetup(1, 1, 1);
	SetTitle("sysconf_plugin", "tex_bar_init_icon", "%s v%i.%02i", module_info.modname, module_info.modversion[1], module_info.modversion[0]);
	
	LoadWave();
	SetBackground();
	vlfGuiAddEventHandler(PSP_CTRL_SQUARE, 0, SetBackground, NULL);

	int btn = GetKeyPress(0);
	if(!(btn & PSP_CTRL_RTRIGGER)){StartupCheck();}

	ResetScreen(1, 0, 0);

	while(1){vlfGuiDrawFrame();}

   	return 0;
}
/*
	IPL
*/
void ExtractIPL(char *outfile, int type) // use 0 as type for extraction based on size detection
{
	int i = 0, blankblocks = 0;
	u8 blank[4096];
	memset(blank, 0, sizeof(blank));

	sceIoRemove(outfile);

	GetMSInfo("msstor:", 0);

	SceUID in = sceIoOpen("msstor:", PSP_O_RDONLY, 0777); // open the input (usually a drive)
	SceUID out = sceIoOpen(outfile, PSP_O_WRONLY|PSP_O_CREAT, 0777); // open the output (usually a file)
	if(in < 1 || out < 1){ErrorReturn("Unable to open msstor: or %s", outfile);return;}
	if(out < 1){ErrorReturn("Unable to open %s", outfile);return;}

	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Extracting IPL from Memory Stick\n\nPlease wait...");

	sceIoLseek(in, 0x2000, 0); // go to the ipl
		
	if(MSInfo.IPLSize != -1 && type == 0) // ipl size is -1 when ipl cannot be detected
	{
		i = MSInfo.IPLSize;
		sceIoRead(in, big_buffer, (MSInfo.IPLSize + 512) & 0xFFFFFE00); // read the ipl (must be read in blocks of 512 bytes)
		sceIoWrite(out, big_buffer, MSInfo.IPLSize); // write the ipl (excluding extra read bytes)
	}
	else
	{
		while(1)
		{
			sceIoRead(in, big_buffer, 4096); // read a block
			
			if(!memcmp(big_buffer, blank, 4096) && !type)
			{
				blankblocks++;
			}
			else
			{
				if(blankblocks){
					blankblocks = 0;
					sceIoWrite(out, blank, 4096);
				}
			}
			if(blankblocks >= 2 || i >= MSInfo.IPLSpace) break; // stop dumping after there is 2 blank blocks or after reaching end of reserved space
			if(blankblocks == 0) i += sceIoWrite(out, big_buffer, 4096); // write a block and increase the block counter
		}
	}

	sceIoClose(out);
	sceIoClose(in);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Detected IPL: %s (%i bytes)\n\nIPL has been saved to %s", MSInfo.IPLName, i, outfile);
}
void EraseIPL()
{
	int cont = vlfGuiMessageDialog("Once the IPL has been erased from the Memory Stick it cannot be recovered without a backup of the IPL.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
	if(cont != 1){OnBackToMainMenu(0);return;}

	GetMSInfo("msstor:", 0);

	SceUID out = sceIoOpen("msstor:", PSP_O_WRONLY, 0777); // open the output (usually a drive)
	if(out < 1){ErrorReturn("Unable to open msstor:");return;}
	
	sceIoLseek(out, 0x2000, 0); // go to the ipl
	memset(big_buffer, 0, MSInfo.IPLSpace);
	sceIoWrite(out, big_buffer, MSInfo.IPLSpace);
	
	sceIoClose(out);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%i bytes (%i blocks) erased", MSInfo.IPLSpace, MSInfo.IPLSpace / 4096);
}
void InjectIPL(char *filename, int type)
{
	int size = 0;
	memset(big_buffer, 0, sizeof(big_buffer));

	if(type == 0){size = zipFileRead(ebootpath, EBOOT_PSAR, filename, big_buffer);}
	else if(type == 1){size = ReadFile(filename, 0, big_buffer, sizeof(big_buffer));}
	
	if(size < 0 && type == 1){ErrorReturn("Unable to open %s", filename);return;}
	
	if(size != (size / 4096) * 4096){size = (size + 4096) & 0xFFFFFE00;}
	GetMSInfo("msstor:", 0);
	if(MSInfo.IPLSpace < size){ErrorReturn("Insufficient Reserved Sector Space.\nRequired Space: %i bytes\nAvailable Space: %i bytes", size, MSInfo.IPLSpace);return;}

	SceUID out = sceIoOpen("msstor:", PSP_O_WRONLY, 0777); // open the output
	if(out < 1){ErrorReturn("Unable to open msstor:");return;}

	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Injecting IPL to Memory Stick\n\nPlease wait...");
	
	sceIoLseek(out, 0x2000, 0); // go to the ipl location
	sceIoWrite(out, big_buffer, size); // write the ipl
	sceIoClose(out);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%i bytes (%i blocks) written\n\nIPL has been injected to Memory Stick", size, size / 4096);
}
void CheckIPL(char *input, int type) // use 0 for injected ipl or 1 for file
{
	int error = GetMSInfo(input, type);
	if(error == -1){ErrorReturn("Unable to open %s", input);return;}

	ResetScreen(0, 1, 0);
	vlf_texts[0] = vlfGuiAddTextF(40, 120, "Detected IPL Name: %s", MSInfo.IPLName);
	if(MSInfo.IPLSize == -1){vlf_texts[1] = vlfGuiAddText(40, 140, "Detected IPL Size: Unknown");}
	else{vlf_texts[1] = vlfGuiAddTextF(40, 140, "Detected IPL Size: %i bytes", MSInfo.IPLSize);}

	if(MSInfo.IPLSize != -1){SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Detected IPL Name: %s\n\nDetected IPL Size: %i bytes", MSInfo.IPLName, MSInfo.IPLSize);}
	else{SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Unknown IPL Detected");}
}
void CreateCheckSum(char *input, char *outfile, int seek) // use 0x2000 for injected ipl or 0 for file
{
	SceUID in = sceIoOpen(input, PSP_O_RDONLY, 0777); // open the input (usually a drive)
	if(in < 1){ErrorReturn("Unable to open %s", input);return;}
	SceUID out = sceIoOpen(outfile, PSP_O_WRONLY|PSP_O_CREAT, 0777); // open the output (usually a file)
	if(out < 1){ErrorReturn("Unable to open %s", outfile);return;}

	sceIoLseek(in, seek, 0); // go to the ipl

	memset(big_buffer, 0, 4096);
	sceIoRead(in, big_buffer, 4096); // read a block

	u8 sha1[20];
	sceKernelUtilsSha1Digest(big_buffer, 4096, sha1);
	sceIoWrite(out, sha1, sizeof(sha1)); // write the checksum

	sceIoClose(out);
	sceIoClose(in);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Checksum has been saved to %s", outfile);
}
/*
	Battery
*/
void SetBatSer(u16 ser1, u16 ser2)
{
	if(GetBatSer() == (ser2 & 0xFFFF) + ((ser1 & 0xFFFF)*0x10000)){ErrorReturn("The current battery serial is already set to 0x%04X%04X. There is no need to change the battery serial.", ser1, ser2);return;}

	u16 BatSer[2];
	BatSer[0] = ser1;
	BatSer[1] = ser2;

	if((BatSer[0] == 0x5241)&&(BatSer[1] == 0x4E44)){ // generate a random serial if the serial passed is RAND
		BatSer[0] = Rand(0x0001, 0xFFFE);
		BatSer[1] = Rand(0x0001, 0xFFFE);
	}
	
	WriteSerial(BatSer);

	if(GetBatSer() != (BatSer[1] & 0xFFFF) + ((BatSer[0] & 0xFFFF)*0x10000)){ErrorReturn("Unable to write to the Battery EEPROM.");return;}

	ShowBatSer();
}
u32 GetBatSer()
{
	u16 BatSer[2];
	ReadSerial(BatSer);

	return (BatSer[1] & 0xFFFF) + ((BatSer[0] & 0xFFFF)*0x10000);
}
void ShowBatSer()
{
	u32 BatSer = GetBatSer();

	char *BatMode;
	if(BatSer == 0xFFFFFFFF){BatMode = "Service Mode Battery";}
	else if(BatSer == 0x00000000){BatMode = "AutoBoot Battery";}
	else{BatMode = "Normal Battery";}

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Battery Mode: %s\n\nBattery Serial: 0x%08X", BatMode, BatSer);
}
void RestoreBatSer(char *infile)
{
	int in, size;

	memset(big_buffer, 0xFF, 256);

	in = sceIoOpen(infile, PSP_O_RDONLY, 0777);
	if(in < 1){ErrorReturn("Unable to open %s", infile);return;}
	size = sceIoRead(in, big_buffer, 256); // read in the file and get the size
	sceIoClose(in);

	if(size != 256){ErrorReturn("Incorrect file size");return;}

	SetBatSer((big_buffer[14] & 0xFF) + ((big_buffer[15] & 0xFF)*0x100), (big_buffer[18] & 0xFF) + ((big_buffer[19] & 0xFF)*0x100));
}

void BackupBat(char *outfile)
{
	int address, err, errtotal = 0;

	sceIoRemove(outfile);

	memset(big_buffer, 0xFF, 256);
	SceUID out = sceIoOpen(outfile, PSP_O_WRONLY|PSP_O_CREAT, 0777);
	if(out < 1){ErrorReturn("Unable to open %s", outfile);return;}

	SetStatus(0, 2, 240, 110, VLF_ALIGNMENT_CENTER, "Saving EEPROM from Battery...");

	for(address = 0; address < 128; address++){
		SetProgress(address, 128);
		err = ReadProm(address, (u16*) big_buffer+address);
		if(err < 0){errtotal++;}
	}
	
	sceIoWrite(out, big_buffer, 256);
	sceIoClose(out);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "EEPROM has been saved to %s\n\n%i read errors occured", outfile , errtotal);
}
void RestoreBat(char* infile)
{
	if(pspGetBaryonVersion() >= 0x00234000){ErrorReturn("The current PSP hardware does not support writing to the Battery EEPROM");return;}

	int address, err, errtotal = 0;
	
	memset(big_buffer, 0xFF, 256);
	int size = ReadFile(infile, 0, big_buffer, 256);
	if(size < 0){ErrorReturn("Unable to open %s", infile);return;}

	SetStatus(0, 2, 240, 110, VLF_ALIGNMENT_CENTER, "Restoring EEPROM from Battery...");

	for(address = 0; address < 0x80; address++){
		SetProgress(address, 128);
		err = WriteProm(address, (u16*) big_buffer+address);
		if(err < 0){errtotal++;}
	};

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "EEPROM has been restored from %s\n\n%i write errors occured", infile, errtotal);	
}
/*
	IdStorage
*/
void BackupIdStorage(char *outfile)
{
	u32 idsscramble = pspNandGetScramble();
	int idskey[2], in, currkey;
	char filepath[32];

	sprintf(filepath, "%s_%08X.bin", outfile, idsscramble);

	if(FileExists(filepath)){
		int cont = vlfGuiMessageDialog("An existing IdStorage backup was found.\n\nDo you want to overwrite the current IdStorage backup?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
		if(cont != 1){OnBackToMainMenu(0);return;}
		sceIoRemove(filepath);
	}

	SceUID out = sceIoOpen(filepath, PSP_O_RDONLY|PSP_O_WRONLY|PSP_O_CREAT, 0777);
	if(out < 0){ErrorReturn("Unable to open %s", filepath);return;}

	for(currkey=0; currkey<0xffef; currkey++){
		memset(idskey, 0, sizeof(idskey));
		idskey[0] = currkey;

		in = pspIdStorageReadLeaf(currkey, big_buffer);
		if(in != 0){continue;} // can't read idstorage key

		SetStatus(0, 0, 240, 110, VLF_ALIGNMENT_CENTER, "Saving IdStorage Key: %04X", currkey);
		
		sceIoWrite(out, idskey, 2);
		sceIoWrite(out, big_buffer, 512);
	}

	sceIoLseek(out, 0, PSP_SEEK_SET);
	int size = sceIoLseek(out, 0, PSP_SEEK_END);
	sceIoLseek(out, 0, PSP_SEEK_SET);
	sceIoRead(out, big_buffer, size);

	u8 sha1[20];
	sceKernelUtilsSha1Digest(big_buffer, size, sha1);
	sceIoWrite(out, sha1, sizeof(sha1));

	sceIoClose(out);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "IdStorage keys saved to %s", filepath);
}
void RestoreIdStorage(char *infile)
{
	u32 idsscramble = pspNandGetScramble();
	int idskey[2], size, currkey;
	char filepath[32];

	sprintf(filepath, "%s_%08X.bin", infile, idsscramble);

	char message[1024];
	sprintf(message, "This operation will erase your current IdStorage and replace it with the file stored at %s on the Memory Stick. This process is capable of permenantely bricking the PSP console if the IdStorage did not originate from this console.\n\nDo you want to continue?", filepath);
	int cont = vlfGuiMessageDialog(message, VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
	if(cont != 1){OnBackToMainMenu(0);return;}

	if(!FileExists(filepath)){ErrorReturn("Unable to open %s", filepath);return;}

	SceIoStat info;
	sceIoGetstat(filepath, &info);
	size = info.st_size;

	if(size != (size / 514 * 514) + 20){ErrorReturn("%s isn't the correct size", filepath);return;};

	SceUID in = sceIoOpen(filepath, PSP_O_RDONLY, 0777);

	u8 sha1[20], sha1_file[20];
	sceIoRead(in, big_buffer, size-20);
	sceIoRead(in, sha1_file, 20);
	sceKernelUtilsSha1Digest(big_buffer, size-20, sha1);
	sceIoLseek(in, 0, PSP_SEEK_SET);

	if(memcmp(sha1_file, sha1, 20) != 0){
		cont = vlfGuiMessageDialog("The IdStorage backup file on the Memory Stick has been modified since creation and may cause the PSP to brick.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
		if(cont != 1){OnBackToMainMenu(0);sceIoClose(in);return;}
	}
		
	while(1){
		memset(idskey, 0, sizeof(idskey));
		sceIoRead(in, idskey, 2);
		size = sceIoRead(in, big_buffer, 512);

		currkey = (idskey[0]) + ((idskey[1])*0x100);
		SetStatus(0, 0, 240, 110, VLF_ALIGNMENT_CENTER, "Restoring IdStorage Key: %04X", currkey);

		if(size == 512){pspIdStorageWriteLeaf(currkey, big_buffer);}
		else{break;}
	}

	sceIoClose(in);

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "IdStorage keys restored from %s", filepath);
}
/*
	MMS Creation & PSAR Extraction
*/
static int FindTablePath(char *table, int table_size, char *number, char *szOut)
{
	int i, j, k;

	for(i = 0; i < table_size-5; i++){
		if(strncmp(number, table+i, 5) == 0){
			for(j = 0, k = 0; ; j++, k++){
				if(table[i+j+6] < 0x20){
					szOut[k] = 0;
					break;
				}

				if(table[i+5] == '|' && !strncmp(table+i+6, "flash", 5) && j == 6){
					szOut[6] = ':';
					szOut[7] = '/';
					k++;
				}
				else if(table[i+5] == '|' && !strncmp(table+i+6, "ipl", 3) && j == 3){
					szOut[3] = ':';
					szOut[4] = '/';
					k++;
				}
				else{szOut[k] = table[i+j+6];}
			}

			return 1;
		}
	}

	return 0;
}

static int FindReboot(u8 *input, u8 *output, int size)
{
	int i;

	for(i = 0; i < (size - 0x30); i++){
		if(memcmp(input+i, "~PSP", 4) == 0){
			size = *(u32 *)&input[i+0x2C];

			memcpy(output, input+i, size);
			return size;
		}
	}

	return -1;
}

static void ExtractReboot(int extractmode, char *loadexec, char *reboot, char *rebootname, char *outdir)
{
	sprintf(loadexec, "%s/%s", loadexec, outdir);
	sprintf(reboot, "%s/%s", reboot, outdir);

	int s = ReadFile(loadexec, 0, sm_buffer1, SMALL_BUFFER_SIZE);

	if(s <= 0){return;}
	
	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Extracting %s... ", rebootname);

	if(extractmode != MODE_DECRYPT){
		if(extractmode == MODE_ENCRYPT_SIGCHECK){
			memcpy(sm_buffer2, sm_buffer1, s);
			pspSignCheck(sm_buffer2);

			if(WriteFile(loadexec, 0, sm_buffer2, s) != s){
				vlfGuiMessageDialog("Unable to write Reboot File", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
			}
		}
			
		s = pspDecryptPRX(sm_buffer1, sm_buffer2, s);
		if(s <= 0){
			vlfGuiMessageDialog("Unable to decrypt Reboot File", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
		}

		s = pspDecompress(sm_buffer2, sm_buffer1, SMALL_BUFFER_SIZE);
		if(s <= 0){
			vlfGuiMessageDialog("Unable to decompress Reboot File", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
		}
	}

	s = FindReboot(sm_buffer1, sm_buffer2, s);
	if(s <= 0){
		vlfGuiMessageDialog("Unable to find Reboot File", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
	}

	s = pspDecryptPRX(sm_buffer2, sm_buffer1, s);
	if(s <= 0){
		vlfGuiMessageDialog("Unable to decrypt Reboot file", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
	}

	WriteFile(reboot, 0, sm_buffer1, s);

	s = pspDecompress(sm_buffer1, sm_buffer2, SMALL_BUFFER_SIZE);
	if(s <= 0){
		vlfGuiMessageDialog("Unable to decompress Reboot file", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
	}

	if(WriteFile(reboot, 0, sm_buffer2, s) != s){
		vlfGuiMessageDialog("Unable to write Reboot file", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;
	}

	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "done.\n");
}

static char *GetVersion(char *buf)
{
	char *p = strrchr(buf, ',');

	if(!p){return NULL;}

	return p+1;
}

static int is5Dnum(char *str)
{
	int len = strlen(str);

	if(len != 5){return 0;}

	int i;

	for(i = 0; i < len; i++){if(str[i] < '0' || str[i] > '9'){return 0;}}

	return 1;
}
int DumpPSAR(int extractmode, char *filepath, char *outdir, char *requiredver, u16 model, char *requiredfiles[])
{
	int s, res;
	u8 pbp_header[0x28];
	SceUID fd;
	int error = 0;
	int psar_pos = 0, psar_offs;
	int table_mode;

	static char com_table[0x4000];
	static int comtable_size;

	static char _1g_table[0x4000];
	static int _1gtable_size;

	static char _2g_table[0x4000];
	static int _2gtable_size;

	static char _3g_table[0x4000];
	static int _3gtable_size;

	static char _4g_table[0x4000];
	static int _4gtable_size;

	static char _5g_table[0x4000];
	static int _5gtable_size;

	static char _6g_table[0x4000];
	static int _6gtable_size;

	static char _7g_table[0x4000];
	static int _7gtable_size;

	static char _8g_table[0x4000];
	static int _8gtable_size;

	static char _9g_table[0x4000];
	static int _9gtable_size;

	static char _10g_table[0x4000];
	static int _10gtable_size;

	static char _11g_table[0x4000];
	static int _11gtable_size;

	static char _12g_table[0x4000];
	static int _12gtable_size;


	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Reading EBOOT contents...");

	if(ReadFile(filepath, 0, pbp_header, sizeof(pbp_header)) != sizeof(pbp_header)){ErrorReturn("Unable to read %s", filepath);return 1;}

	if(memcmp(pbp_header, "PSAR", 4) == 0){psar_offs = 0;}
	else{psar_offs = *(u32 *)&pbp_header[0x24];}

	fd = sceIoOpen(filepath, PSP_O_RDONLY, 0);
	
	int cbFile = sceIoLseek32(fd, 0, PSP_SEEK_END) - psar_offs;
	sceIoLseek32(fd, psar_offs, PSP_SEEK_SET);

	if(sceIoRead(fd, big_buffer, BIG_BUFFER_SIZE) <= 0){ErrorReturn("Error reading file");sceIoClose(fd);return 1;}
	if(memcmp(big_buffer, "PSAR", 4) != 0){ErrorReturn("Invalid PSAR file");sceIoClose(fd);return 1;}
   
	res = pspPSARInit(big_buffer, sm_buffer1, sm_buffer2);
	if(res < 0){ErrorReturn("pspPSARInit fialed (0x%08X)", res);sceIoClose(fd);return 1;}

	char *ver = GetVersion((char *)sm_buffer1+0x10);
	if(requiredver != NULL){
		if(strcmp(ver, requiredver)){ErrorReturn("Wrong EBOOT.PBP version\nRequired Version: %s\nSupplied Version: %s", requiredver, ver);sceIoClose(fd);return 1;}
	}

	int psarVersion = 0;
	int version = -1;
	psarVersion = big_buffer[4];
	if(!memcmp(ver, "3.8", 3) || !memcmp(ver, "3.9", 3)){table_mode = 1;}
	else if(!memcmp(ver, "4.0", 3)){table_mode = 2;}
	else if(!memcmp(ver, "5.0", 3)){table_mode = 3;}
	//else if(!memcmp(ver, "6.", 2) && (psarVersion == 5)){table_mode = 5; version = 661;}
	else if(!memcmp(ver, "6.", 2)){table_mode = 4; version = 661;}
	else{table_mode = 0;}

	while (1){
		char name[128];
		int cbExpanded;
		int pos;
		int signcheck;
		int i = 0,  filerequired = 1;

		int res = pspPSARGetNextFile(big_buffer, cbFile, sm_buffer1, sm_buffer2, name, &cbExpanded, &pos, &signcheck);

		if(res < 0){
			if(error){ErrorReturn("PSAR decode error at 0x%08X", pos);sceIoClose(fd);return 1;}

			int dpos = pos-psar_pos;
			psar_pos = pos;
			
			error = 1;
			memmove(big_buffer, big_buffer+dpos, BIG_BUFFER_SIZE-dpos);

			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Reading EBOOT contents...");
			if(sceIoRead(fd, big_buffer+(BIG_BUFFER_SIZE-dpos), dpos) <= 0){ErrorReturn("Unable to read file");sceIoClose(fd);return 1;}

			pspPSARSetBufferPosition(psar_pos);

			continue;
		}
		else if(res == 0){ /* no more files */
			break;
		}

		if(is5Dnum(name)){
			//if(strcmp(name, "00001") != 0 && strcmp(name, "00002") != 0 && strcmp(name, "00003") != 0){
			if (atoi(name) >= 100 || (atoi(name) >= 10)) {
			//if (atoi(name) >= 1 && atoi(name) <= 11) {
				int found = 0;
				
				if(_1gtable_size > 0){found = FindTablePath(_1g_table, _1gtable_size, name, name);}
				if(!found && _2gtable_size > 0){found = FindTablePath(_2g_table, _2gtable_size, name, name);}
				if(!found && _3gtable_size > 0){found = FindTablePath(_3g_table, _3gtable_size, name, name);}
				if(!found && _4gtable_size > 0){found = FindTablePath(_4g_table, _4gtable_size, name, name);}
				if(!found && _5gtable_size > 0){found = FindTablePath(_5g_table, _5gtable_size, name, name);}
				if(!found && _6gtable_size > 0){found = FindTablePath(_6g_table, _6gtable_size, name, name);}
				if(!found && _7gtable_size > 0){found = FindTablePath(_7g_table, _7gtable_size, name, name);}
				if(!found && _8gtable_size > 0){found = FindTablePath(_8g_table, _8gtable_size, name, name);}
				if(!found && _9gtable_size > 0){found = FindTablePath(_9g_table, _9gtable_size, name, name);}
				if(!found && _10gtable_size > 0){found = FindTablePath(_10g_table, _10gtable_size, name, name);}
				if(!found && _11gtable_size > 0){found = FindTablePath(_11g_table, _11gtable_size, name, name);}
				if(!found && _12gtable_size > 0){found = FindTablePath(_12g_table, _12gtable_size, name, name);}
				if(!found){
					SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Warning: cannot find table path of %s", name);
					error = 0;
					continue;
				}
			}
		}
		else if(!strncmp(name, "com:", 4) && comtable_size > 0){
			if(!FindTablePath(com_table, comtable_size, name+4, name)){
				SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Warning: cannot find com path of %s", name);
				error = 0;
				continue;
			}
		}

		else if(!strncmp(name, "01g:", 4) && _1gtable_size > 0){
			if(!FindTablePath(_1g_table, _1gtable_size, name+4, name)){
				ErrorReturn("Cannot find path of %s", name);sceIoClose(fd);return 1;
			}
		}

		else if(!strncmp(name, "02g:", 4) && _2gtable_size > 0){
			if(!FindTablePath(_2g_table, _2gtable_size, name+4, name)){ErrorReturn("Cannot find path of %s", name);sceIoClose(fd);return 1;}
			if(!strcmp(name, "flash0:/vsh/etc/index.dat") && memcmp(ver, "4.01", 4) && model != 0x020){sprintf(name, "flash0:/vsh/etc/yndex.dat");}
		}

		if(requiredfiles == NULL){filerequired = 1;}
		else{
			filerequired = 0;
			i = 0;
			while(i != -1){
				if(!strcmp(name, requiredfiles[i]) || !strcmp("*", requiredfiles[i])){filerequired = 1;i = -2;}
				i++;
				if(!strcmp(requiredfiles[i], "0")){i = -1;}
			}
		}
		if(filerequired == 0){continue;}

		char* szFileBase = strrchr(name, '/');
		
		if(szFileBase != NULL){szFileBase++;} // after slash
		else{szFileBase = "err.err";}

		if(cbExpanded > 0){
			char szDataPath[128];
			
			if(!strncmp(name, "flash0:/", 8)){sprintf(szDataPath, "%s/%s", outdir, name+8);}
			else if(!strncmp(name, "flash1:/", 8)){sprintf(szDataPath, "%s/%s", outdir, name+8);}
			else if(!strcmp(name, "com:00000")){
				comtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
				if(comtable_size <= 0){ErrorReturn("Unable to decrypt common table");sceIoClose(fd);return 1;}
				if(comtable_size > sizeof(com_table)){ErrorReturn("COM table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

				memcpy(com_table, sm_buffer2, comtable_size);
				continue;
			}	
			else if(!strcmp(name, "01g:00000") || !strcmp(name, "00001")){
				if(model == NULL || model == 0x000 || model == 0x100 || model == 0x120 || model == 0x103 || model == 0x123){
					_1gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
					if(_1gtable_size <= 0){ErrorReturn("Unable to decrypt 1g table");sceIoClose(fd);return 1;}
					if(_1gtable_size > sizeof(_1g_table)){ErrorReturn("1g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_1g_table, sm_buffer2, _1gtable_size);
				}
				continue;
			}	
			else if(!strcmp(name, "02g:00000") || !strcmp(name, "00002")){
				if(model == NULL || model == 0x000 || model == 0x020 || model == 0x120 || model == 0x023 || model == 0x123){
					_2gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_2gtable_size <= 0){ErrorReturn("Unable to decrypt 2g table");sceIoClose(fd);return 1;}
					if(_2gtable_size > sizeof(_2g_table)){ErrorReturn("2g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_2g_table, sm_buffer2, _2gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00003")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_3gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_3gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led03g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 3g table");
						error = 0;
						continue;
					}

					if(_3gtable_size > sizeof(_3g_table)){ErrorReturn("3g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_3g_table, sm_buffer2, _3gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00004")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_4gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_4gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led04g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 4g table");
						error = 0;
						continue;
					}

					if(_4gtable_size > sizeof(_4g_table)){ErrorReturn("4g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_4g_table, sm_buffer2, _4gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00005")){
				if(model == 0x1337){
					_5gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_5gtable_size <= 0){
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 5g table");
						error = 0;
						continue;
					}

					if(_5gtable_size > sizeof(_5g_table)){ErrorReturn("5g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_5g_table, sm_buffer2, _5gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00006")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_6gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_6gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led06g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 6g table");
						error = 0;
						continue;
					}

					if(_6gtable_size > sizeof(_6g_table)){ErrorReturn("6g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_6g_table, sm_buffer2, _6gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00007")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_7gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_7gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led07g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 7g table");
						error = 0;
						continue;
					}

					if(_7gtable_size > sizeof(_7g_table)){ErrorReturn("7g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_7g_table, sm_buffer2, _7gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00008")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_8gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_8gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led08g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 8g table");
						error = 0;
						continue;
					}

					if(_8gtable_size > sizeof(_8g_table)){ErrorReturn("8g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_8g_table, sm_buffer2, _8gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00009")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_9gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_9gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led09g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 9g table");
						error = 0;
						continue;
					}

					if(_9gtable_size > sizeof(_9g_table)){ErrorReturn("9g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_9g_table, sm_buffer2, _9gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00010")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_10gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_10gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led010g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 10g table");
						error = 0;
						continue;
					}

					if(_10gtable_size > sizeof(_10g_table)){ErrorReturn("10g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_10g_table, sm_buffer2, _10gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00011")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_11gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_11gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led011g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 11g table");
						error = 0;
						continue;
					}

					if(_11gtable_size > sizeof(_11g_table)){ErrorReturn("11g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_11g_table, sm_buffer2, _11gtable_size);
				}
				continue;
			}
			else if(!strcmp(name, "00012")){
				if(model == NULL || model == 0x000 || model == 0x003 || model == 0x103 || model == 0x023 || model == 0x123){
					_12gtable_size = pspDecryptTable(sm_buffer2, sm_buffer1, cbExpanded, table_mode);
							
					if(_12gtable_size <= 0){
						// We don't have yet the keys for table of 3000, they are only in mesg_led012g.prx
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Cannot decrypt 12g table");
						error = 0;
						continue;
					}

					if(_12gtable_size > sizeof(_12g_table)){ErrorReturn("12g table buffer is too small.\nRecompile application with a bigger buffer.");sceIoClose(fd);return 1;}

					memcpy(_12g_table, sm_buffer2, _12gtable_size);
				}
				continue;
			}
			else{continue;}

			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", szDataPath);

			if(signcheck && extractmode == MODE_ENCRYPT_SIGCHECK && (strcmp(name, "flash0:/kd/loadexec.prx") != 0) && (strcmp(name, "flash0:/kd/loadexec_01g.prx") != 0) && (strcmp(name, "flash0:/kd/loadexec_02g.prx") != 0)){
				pspSignCheck(sm_buffer2);
			}

			if((extractmode != MODE_DECRYPT) || (memcmp(sm_buffer2, "~PSP", 4) != 0)){
				if(filerequired == 1){
					if(WriteFile(szDataPath, 0, sm_buffer2, cbExpanded) != cbExpanded){ErrorReturn("Cannot write %s", szDataPath);sceIoClose(fd);return 1;break;}
				}
			}

			if((memcmp(sm_buffer2, "~PSP", 4) == 0) &&	(extractmode == MODE_DECRYPT)){
				int cbDecrypted = pspDecryptPRX(sm_buffer2, sm_buffer1, cbExpanded);

				// output goes back to main buffer
				// trashed 'sm_buffer2'
				if(cbDecrypted > 0){
					u8* pbToSave = sm_buffer1;
					int cbToSave = cbDecrypted;
                    
					if((sm_buffer1[0] == 0x1F && sm_buffer1[1] == 0x8B) || memcmp(sm_buffer1, "2RLZ", 4) == 0 || memcmp(sm_buffer1, "KL4E", 4) == 0){
						int cbExp = pspDecompress(sm_buffer1, sm_buffer2, SMALL_BUFFER_SIZE);
						
						if(cbExp > 0){
							pbToSave = sm_buffer2;
							cbToSave = cbExp;
						}
						else{
							if(filerequired == 1){SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Decompress error 0x%08X\n\nFile will be written compressed", cbExp);}
						}
					}
        			
					if(filerequired == 1){
						if(WriteFile(szDataPath, 0, pbToSave, cbToSave) != cbToSave){ErrorReturn("Cannot write %s", szDataPath);sceIoClose(fd);return 1;}
					}
				}
				else{ErrorReturn("Unable to decrypt %s", szDataPath);sceIoClose(fd);return 1;}
			}
		}

		error = 0;
		scePowerTick(0);	
	}

	sceIoClose(fd);

    return 0;
}
int ExtractUpdaterPRXs(int extractmode, char *filepath, char *outdir)
{
	int dpsp_o, dpsp_l, psar_o;
	int i2, i3, j, r;
	u32 tag;
	u8* modptr[16];
	int modlen[16];
	u8* buff_off;
	u8 i;

	u32 xorkeys[] = {0x00, 0x75, 0xF1, 0x25, 0x43, 0x34, 0x93, 0xDE, 0x55, 0x77};

	struct modules {
		char*	modname;
		char*	filename;
	} modules[] = {
		{"sceLFatFs_Updater_Driver", "lfatfs_updater.prx"},
		{"sceNAND_Updater_Driver",  extractmode == 0 ? "nand_updater.prx":"emc_sm_updater.prx"},
		{"sceLflashFatfmtUpdater", "lflash_fatfmt_updater.prx"}
	};

	memset(big_buffer, 0, BIG_BUFFER_SIZE);

	SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Saving Updater Modules...");
	//SetProgress(0, 0xFF);

	SceUID fd = sceIoOpen(filepath, PSP_O_RDONLY, 0777);
	if(fd < 0){ErrorReturn("Unable to read %s", filepath);return 1;}

	sceIoRead(fd, big_buffer, 40);

	dpsp_o = *(u32 *)&big_buffer[0x20];
	psar_o = *(u32 *)&big_buffer[0x24];
	dpsp_l = psar_o - dpsp_o;

	sceIoLseek(fd, dpsp_o, SEEK_SET);
	sceIoRead(fd, big_buffer, dpsp_l);
	sceIoClose(fd);

	buff_off = (u8*)(((int)big_buffer + dpsp_l + 1024) & 0xFFFFFF00);
	r = pspDecryptPRX(big_buffer, buff_off, dpsp_l);

	//for(i = 0; i < 0xFF; i += 1){
	for(i = 0; i < sizeof(xorkeys) / sizeof(xorkeys[0]); i += 1){
		//u32 xorkey = (i & 0xFF) + ((i & 0xFF)*0x100) + ((i & 0xFF)*0x10000) + ((i & 0xFF)*0x1000000);
		u32 xorkey = (xorkeys[i] & 0xFF) + ((xorkeys[i] & 0xFF)*0x100) + ((xorkeys[i] & 0xFF)*0x10000) + ((xorkeys[i] & 0xFF)*0x1000000);
		//SetProgress(i, 0xFF);

		tag = 0x5053507E ^ xorkey;
		for(i2=0; i2<dpsp_l; i2+=4){
			if(*(u32*)(buff_off+i2) == tag){
				modptr[0] = &buff_off[i2];
				modlen[0] = (*(u32*)(buff_off+i2+0x2C)) ^ xorkey;

				for(j=0; j<modlen[0]; j+=4){
					*(u32*)(modptr[0]+j) ^= xorkey;
				}

				for(i3 = 0; i3 < sizeof(modules) / sizeof(modules[0]); i3++){
					char outname[128];
					sprintf(outname, "%s/%s", outdir, modules[i3].filename);
					if(strcmp((char*)(modptr[0]+0x0A), modules[i3].modname) == 0){
						SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, outname);
						WriteFile(outname, 0, modptr[0], modlen[0]);
					}	
				}
			}
		}
	}

	return 0;
}
void BackupSettings(char *outdir)
{
	int size;

	int btn = GetKeyPress(0);
	if(btn & PSP_CTRL_LTRIGGER){
		size = ReadFile("flash1:/registry/system.dreg", 0, big_buffer, sizeof(big_buffer));
		if(size > 0){WriteFile(strcat(strdup(outdir), "/system.dreg"), 0, big_buffer, size);}

		size = ReadFile("flash1:/registry/system.ireg", 0, big_buffer, sizeof(big_buffer));
		if(size > 0){WriteFile(strcat(strdup(outdir), "/system.ireg"), 0, big_buffer, size);}
	}

	size = ReadFile("flash2:/act.dat", 0, big_buffer, sizeof(big_buffer));
	if(size > 0){WriteFile(strcat(strdup(outdir), "/act.dat"), 0, big_buffer, size);}
}
void CreateMMS(char *mmsver)
{
	int cont, err = 0, i, customtheme;

	GetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &customtheme, 4);
	if(customtheme != NULL){ErrorReturn("Your PSP has a custom theme set. Turn the theme off before running this program.");return;}

	if(strcmp(mmsver, "150 Update Flasher") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Team C+D\nRelease Date: 22nd of August 2007\nFirmware: 1.50 to 2.71 (OFW)\nCompatibility: All PSP-1000 units\n\nTeam C+D Members\n - Adrahil (VoidPointer)\n - Booster\n - Cswindle (Caretaker)\n - Dark_AleX (Malyot)\n - Ditlew\n - Fanjita (FullerMonty)\n - Joek2100 (CosmicOverSoul)\n - Jim\n - Mathieulh (WiseFellow)\n - Nem (h1ckeyph0rce)\n - Psp250\n - Skylark\n - TyRaNiD (bockscar)", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/UPDATE.PBP")){ErrorReturn("Please ensure UPDATE.PBP exists at the root of the Memory Stick. This must be a 1.50 Update EBOOT. However after creation it can be replace with any Update EBOOT between 1.50 and 2.71.");return;}
		
		if(GetHardwareRevision() > 0x010400){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}
		
		if(DirExists("ms0:/kd")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/kd and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/UPDATE.PBP", "ms0:", "1.50", 0x100, UpdateFlasherFiles);
		if(err == 0){BackupSettings("ms0:/registry");}else{return;}
		
		for(i = 0; i < sizeof(UpdateFlasher150files) / sizeof(UpdateFlasher150files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/%s", UpdateFlasher150files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, UpdateFlasher150files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/pandora.bin");
	}
	else if(strcmp(mmsver, "DC3") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Dark_AleX\nRelease Date: 4th of October 2007\nFirmware: 3.71 M33-2 (3.71 OFW)\nCompatibility: TA-088 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/150.PBP") || !FileExists("ms0:/340.PBP") || !FileExists("ms0:/371.PBP")){ErrorReturn("Please ensure 150.PBP, 340.PBP and 371.PBP exist at the root of the Memory Stick.");return;}
		
		if(GetHardwareRevision() > 0x020200){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		if(DirExists("ms0:/kd")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/kd and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/150.PBP", "ms0:", "1.50", 0x120, DC_150Files);
		if(err == 0){err = DumpPSAR(MODE_DECRYPT, "ms0:/340.PBP", "ms0:", "3.40", 0x120, DC_340Files);}else{return;}
		if(err == 0){err = ExtractUpdaterPRXs(0, "ms0:/371.PBP", "ms0:/kd");}else{return;}
		if(err == 0){BackupSettings("ms0:/registry");}else{return;}
		
		for(i = 0; i < sizeof(DC3files) / sizeof(DC3files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/%s", DC3files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, DC3files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/pandora.bin");
	}
	else if(strcmp(mmsver, "DC4") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Dark_AleX\nRelease Date: 21st of January 2008\nFirmware: 3.80 M33-5 (3.80 OFW)\nCompatibility: TA-088 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/150.PBP") || !FileExists("ms0:/340.PBP") || !FileExists("ms0:/380.PBP")){ErrorReturn("Please ensure 150.PBP, 340.PBP and 380.PBP exist at the root of the Memory Stick.");return;}
		
		if(GetHardwareRevision() > 0x020200){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		if(DirExists("ms0:/kd")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/kd and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/150.PBP", "ms0:", "1.50", 0x120, DC_150Files);
		if(err == 0){err = DumpPSAR(MODE_DECRYPT, "ms0:/340.PBP", "ms0:", "3.40", 0x120, DC_340Files);}else{return;}
		if(err == 0){err = ExtractUpdaterPRXs(0, "ms0:/380.PBP", "ms0:/kd");}else{return;}
		if(err == 0){BackupSettings("ms0:/registry");}else{return;}
		
		for(i = 0; i < sizeof(DC4files) / sizeof(DC4files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/%s", DC4files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, DC4files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/pandora.bin");
	}
	else if(strcmp(mmsver, "DC5") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Dark_AleX\nRelease Date: 30th of March 2008\nFirmware: 3.90 M33-3 (3.90 OFW)\nCompatibility: TA-088 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/150.PBP") || !FileExists("ms0:/340.PBP") || !FileExists("ms0:/390.PBP")){ErrorReturn("Please ensure 150.PBP, 340.PBP and 390.PBP exist at the root of the Memory Stick.");return;}
		
		if(GetHardwareRevision() > 0x020200){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		if(DirExists("ms0:/TM/DC5")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/TM/DC5 and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/150.PBP", "ms0:/TM/DC5", "1.50", 0x120, DC_150Files);
		if(err == 0){err = DumpPSAR(MODE_DECRYPT, "ms0:/340.PBP", "ms0:/TM/DC5", "3.40", 0x120, DC_340Files);}else{return;}
		if(err == 0){err = ExtractUpdaterPRXs(0, "ms0:/390.PBP", "ms0:/TM/DC5/kd");}else{return;}
		if(err == 0){BackupSettings("ms0:/TM/DC5/registry");}else{return;}
		
		for(i = 0; i < sizeof(DC5files) / sizeof(DC5files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/TM/DC5/%s", DC5files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, DC5files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/DC5/ipl.bin");
	}
	else if(strcmp(mmsver, "DC7") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Dark_AleX\nRelease Date: 19th of August 2008\nFirmware: 4.01 M33-2 (4.01 OFW)\nCompatibility: TA-088v2 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/401.PBP")){ErrorReturn("Please ensure 401.PBP exists at the root of the Memory Stick.");return;}
		
		if(GetHardwareRevision() > 0x020201 && GetHardwareRevision() != 0x020300){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		if(DirExists("ms0:/TM/DC7")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/TM/DC7 and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/401.PBP", "ms0:/TM/DC7", "4.01", 0x120, NULL);
		if(err == 0){err = ExtractUpdaterPRXs(1, "ms0:/401.PBP", "ms0:/TM/DC7/kd");}else{return;} 
		if(err == 0){BackupSettings("ms0:/TM/DC7/registry");}else{return;}
		
		for(i = 0; i < sizeof(DC7files) / sizeof(DC7files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/TM/DC7/%s", DC7files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, DC7files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/DC7/ipl.bin");
	}
	else if(strcmp(mmsver, "DC8") == 0){
		vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Dark_AleX\nRelease Date: 18th of December 2008\nFirmware: 5.00 M33-4 (5.00 OFW)\nCompatibility: TA-088v2 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(!FileExists("ms0:/500.PBP")){ErrorReturn("Please ensure 500.PBP exists at the root of the Memory Stick.");return;}
		
		if(GetHardwareRevision() > 0x020201 && GetHardwareRevision() != 0x020300){
			cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		if(DirExists("ms0:/TM/DC8")){
			cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/TM/DC8 and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
			if(cont != 1){OnBackToMainMenu(0);return;}
		}

		err = DumpPSAR(MODE_ENCRYPT, "ms0:/500.PBP", "ms0:/TM/DC8", "5.00", 0x120, NULL);
		if(err == 0){err = ExtractUpdaterPRXs(1, "ms0:/500.PBP", "ms0:/TM/DC8/kd");}else{return;} 
		if(err == 0){BackupSettings("ms0:/TM/DC8/registry");}else{return;}
		
		for(i = 0; i < sizeof(DC8files) / sizeof(DC8files[0]); i++){
			char outname[128];
			sprintf(outname, "ms0:/TM/DC8/%s", DC8files[i].outname);
			SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
			zipFileExtract(ebootpath, EBOOT_PSAR, DC8files[i].inname, outname, big_buffer);
		}
		FinalizeMMS("/TM/DC8/ipl.bin");
	}

	else if(strcmp(mmsver, "DC9") == 0){
        vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: Balika011\nRelease Date: 18th of November 2021\nFirmware: 5.02 M33-5\n5.02 TestingTool M33\n5.02 OFW\nCompatibility: TA-092 and older", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
        if(!FileExists("ms0:/502.PBP")){ErrorReturn("Please ensure 502.PBP exists at the root of the Memory Stick.");return;}

        if(GetHardwareRevision() > 0x030101){
            cont = vlfGuiMessageDialog("This Magic Memory Stick software can not be installed on your PSP unit.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
            if(cont != 1){OnBackToMainMenu(0);return;}
        }

        if(DirExists("ms0:/TM/DC9")){
            cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/TM/DC9 and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
            if(cont != 1){OnBackToMainMenu(0);return;}
        }

        err = DumpPSAR(MODE_ENCRYPT, "ms0:/502.PBP", "ms0:/TM/DC9", "5.02", 0x120, NULL);
        if(err == 0){err = ExtractUpdaterPRXs(1, "ms0:/502.PBP", "ms0:/TM/DC9/kd");}else{return;}
        if(err == 0){BackupSettings("ms0:/TM/DC9/registry");}else{return;}

        for(i = 0; i < sizeof(DC9files) / sizeof(DC9files[0]); i++){
            char outname[128];
            sprintf(outname, "ms0:/TM/DC9/%s", DC9files[i].outname);
            SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
            zipFileExtract(ebootpath, EBOOT_PSAR, DC9files[i].inname, outname, big_buffer);
        }
		InjectIPL("ms0:/TM/DC9/msipl.bin", 1);
    }
	else if(strcmp(mmsver, "DC10") == 0){
        vlfGuiMessageDialog("Magic Memory Stick Information\n\nCreator: ARK-4 Team\nRelease Date: 20th of April 2024\nFirmware: 6.61 ARK-4\n6.61 OFW\nCompatibility: All", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
		if(FileExists("ms0:/661GO.PBP")){go = 1;}
        if(!FileExists("ms0:/661.PBP") && !FileExists("ms0:/661GO.PBP")){ErrorReturn("Please ensure 661.PBP or 661GO.PBP exists at the root of the Memory Stick.");return;}

        if(DirExists("ms0:/TM/DCARK")){
            cont = vlfGuiMessageDialog("A Magic Memory Stick Software already exists at ms0:/TM/DCARK and will be overwritten by this installation.\n\nDo you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
            if(cont != 1){OnBackToMainMenu(0);return;}
        }

		if(go > 0) {
        	err = DumpPSAR(MODE_ENCRYPT, "ms0:/661GO.PBP", "ms0:/TM/DCARK", "6.61", 0x1337, NULL);
        	if(err == 0){err = ExtractUpdaterPRXs(1, "ms0:/661GO.PBP", "ms0:/TM/DCARK/kd");}else{return;}
		}
		else {
        	err = DumpPSAR(MODE_ENCRYPT, "ms0:/661.PBP", "ms0:/TM/DCARK", "6.61", NULL, NULL);
        	if(err == 0){err = ExtractUpdaterPRXs(1, "ms0:/661.PBP", "ms0:/TM/DCARK/kd");}else{return;}
		}
        if(err == 0){BackupSettings("ms0:/TM/DCARK/registry");}else{return;}

        for(i = 0; i < sizeof(DC10files) / sizeof(DC10files[0]); i++){
            char outname[128];
            sprintf(outname, "ms0:/TM/DCARK/%s", DC10files[i].outname);
            SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "%s", outname);
            zipFileExtract(ebootpath, EBOOT_PSAR, DC10files[i].inname, outname, big_buffer);
        }
		InjectIPL("ms0:/TM/DCARK/msipl.bin", 1);
    }

	SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Magic Memory Stick has been created");
}
void FinalizeMMS(char *iplpath)
{
	char key[128], data[1024], *write;
	int btn;

	ResetScreen(0, 0, 0);
	GetMSInfo("msstor:", 0);
	if(memcmp(MSInfo.IPLName, "Time Machine", 12)){
		int injectipl = vlfGuiMessageDialog("For the Magic Memory Stick to boot you must have the Time Machine IPL injected to the Memory Stick.\n\nDo you want to inject the Time Machine IPL now?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
		if(injectipl == 1){InjectIPL("IPLs/Time Machine.bin", 0);}
	}

	ResetScreen(0, 0, 0);
	if(GetBatSer() != 0xFFFFFFFF && pspGetBaryonVersion() < 0x00234000){
		int setbatser = vlfGuiMessageDialog("For the Magic Memory Stick to boot you must have the Battery Serial set to 0xFFFFFFFF.\n\nDo you want to set the Battery Serial to 0xFFFFFFFF now?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
		if(setbatser == 1){SetBatSer(0xFFFF, 0xFFFF);}
	}

	SetStatus(0, 0, 240, 100, VLF_ALIGNMENT_CENTER, "Please hold the key(s) which you want to use to\nboot from the Magic Memory Stick...");

	while(1){
		btn = GetKeyPressKernel(1);
		sceKernelDelayThread(1*1000*1000);
		if(btn == GetKeyPressKernel(1)){break;}
	}

	memset(key, 0, sizeof(key));
	if(btn & PSP_CTRL_SELECT){strcat(key, "SELECT+");}
	if(btn & PSP_CTRL_START){strcat(key, "START+");}
	if(btn & PSP_CTRL_UP){strcat(key, "UP+");}
	if(btn & PSP_CTRL_RIGHT){strcat(key, "RIGHT+");}
	if(btn & PSP_CTRL_DOWN){strcat(key, "DOWN+");}
	if(btn & PSP_CTRL_LEFT){strcat(key, "LEFT+");}
	if(btn & PSP_CTRL_LTRIGGER){strcat(key, "L+");}
	if(btn & PSP_CTRL_RTRIGGER){strcat(key, "R+");}
	if(btn & PSP_CTRL_TRIANGLE){strcat(key, "TRIANGLE+");}
	if(btn & PSP_CTRL_CIRCLE){strcat(key, "CIRCLE+");}
	if(btn & PSP_CTRL_CROSS){strcat(key, "CROSS+");}
	if(btn & PSP_CTRL_SQUARE){strcat(key, "SQUARE+");}
	if(btn & PSP_CTRL_HOME){strcat(key, "HOME+");}
	if(btn & PSP_CTRL_NOTE){strcat(key, "NOTE+");}
	if(btn & PSP_CTRL_SCREEN){strcat(key, "SCREEN+");}
	if(btn & PSP_CTRL_VOLUP){strcat(key, "VOLUP+");}
	if(btn & PSP_CTRL_VOLDOWN){strcat(key, "VOLDOWN+");}
	memset(key+strlen(key)-1, 0, sizeof(key)-(strlen(key)+1));

	if(key[0] != NULL){
		SetStatus(0, 1, 240, 160, VLF_ALIGNMENT_CENTER, "%s", key);

		memset(big_buffer, 0, sizeof(big_buffer));
		memset(data, 0, sizeof(data));

		ReadFile("ms0:/TM/config.txt", 0, big_buffer, sizeof(big_buffer));

		sprintf(data, "%s = \"%s\";\r\n%s", key, iplpath, big_buffer);
		write = strpbrk(data, 0);

		WriteFile("ms0:/TM/config.txt", 0, data, strlen(data));

		sceKernelDelayThread(2*1000*1000);
	}
}
/*
	General
*/
void StartupCheck()
{
	int i, err = 0;
	char msg[256];

	if(!err){
		if(sceKernelDevkitVersion() == 0x03070110){
			err = 1;
			sprintf(msg, "Your current firmware version is unsupported. Please update to a newer custom firmware.");
		}
	}

	if(!err){
		struct modnames {
			char*	name;
			int		allowunload;
		} modnames[] = {
			{"PSPLINK", 0},
			{"RemoteJoyLite", 0}
		};

		sprintf(msg, "Please disable the following plugins in GAME mode to reduce the risk of issues arising (hold R after exiting):\n");

		for(i = 0; i < sizeof(modnames) / sizeof(modnames[0]); i++){
			int modloaded = pspModuleLoaded(modnames[i].name);
			if(modloaded){
				if(modnames[i].allowunload){
					StopModule(modnames[i].name);
				}
				else{
					err = 2;
					strcat(msg, "\n   ");
					strcat(msg, modnames[i].name);
				}
			}
		}
	}

	if(err != 0){
		ErrorReturn(msg);
		if(err == 1){sceKernelExitGame();}
		else if(err == 2){scePowerRequestColdReset(0);}
	}
}
void About()
{
	memset(big_buffer, 0, sizeof(big_buffer));

	int btn = GetKeyPress(0);
	if(btn & PSP_CTRL_LTRIGGER){sprintf(big_buffer, "%s", Quotes[Rand(0, sizeof(Quotes) / sizeof(Quotes[0]))]);}
	else{zipFileRead(ebootpath, EBOOT_PSAR, "System/Credits.txt", big_buffer);}

	vlfGuiMessageDialog(big_buffer, VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
	OnBackToMainMenu(0);
}
void ConnectUSB(int enable, u32 device)
{
	if(enable == 0){
		if(pspModuleLoaded("pspUsbDev_Driver")){pspUsbDeviceFinishDevice();}
		sceUsbDeactivate(0x1c8);
		sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0);
	}
	else{
		SetTitle("sysconf_plugin", "tex_bar_usb_icon", "USB Connection");

		LoadStartModule("flash0:/kd/semawm.prx");
		LoadStartModule("flash0:/kd/usbstor.prx");
		LoadStartModule("flash0:/kd/usbstormgr.prx");
		LoadStartModule("flash0:/kd/usbstorms.prx");
		LoadStartModule("flash0:/kd/usbstorboot.prx");

		SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Please wait...");

		if(FileExists("flash0:/kd/usbdevice.prx")){LoadStartModule("flash0:/kd/usbdevice.prx");}
		else if(FileExists(strcat(strdup(path), "/usbdevice.prx"))){LoadStartModule(strcat(strdup(path), "/usbdevice.prx"));}
		if((!tmmode || device == -1 || device == PSP_USBDEVICE_UMD9660) && pspModuleLoaded("pspUsbDev_Driver")){pspUsbDeviceSetDevice(device, 0, 0);}

		sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbstorBootSetCapacity(0x800000);
	
		sceUsbActivate(0x1c8);

		SetStatus(1, 0, 240, 120, VLF_ALIGNMENT_CENTER, "USB Mode");
	}
}
void CheckMSInfo(int page)
{
	int error = GetMSInfo("msstor:", 0);
	if(error == -1){vlfGuiMessageDialog("Unable to open input File/Device", VLF_MD_TYPE_ERROR|VLF_MD_BUTTONS_NONE);OnBackToMainMenu(0);return;}

	ResetScreen(0, 1, 0);

	vlf_texts[0] = vlfGuiAddTextF(20, 80, "Boot Status: 0x%02X", MSInfo.BootStatus);
	vlf_texts[1] = vlfGuiAddTextF(20, 100, "Starting Head: 0x%02X", MSInfo.StartHead);
	vlf_texts[2] = vlfGuiAddTextF(20, 120, "Start Sec/Clu: 0x%04X", MSInfo.StartSector);
	vlf_texts[3] = vlfGuiAddTextF(20, 140, "Partition Type: 0x%02X", MSInfo.PartitionType);
	vlf_texts[4] = vlfGuiAddTextF(20, 160, "Last Head: 0x%02X", MSInfo.LastHead);

	vlf_texts[5] = vlfGuiAddTextF(220, 80, "Last Sec/Clu: 0x%04X", MSInfo.LastSector);
	vlf_texts[6] = vlfGuiAddTextF(220, 100, "Abs Sector: 0x%08X", MSInfo.AbsSector);
	vlf_texts[7] = vlfGuiAddTextF(220, 120, "Total Sectors: 0x%08X", MSInfo.TotalSectors);
	vlf_texts[8] = vlfGuiAddTextF(220, 140, "Signature: 0x%04X", MSInfo.Signature);
	vlf_texts[9] = vlfGuiAddTextF(220, 160, "Available IPL Space: %i KB", MSInfo.IPLSpace / 1024);
}

int GetMSInfo(char *input, int type) // use 0 for msstor:, 1 for ipl from file
{
	SceUID in;
	int i;
	char *buf = malloc(131072);

	memset(&MSInfo, 0, sizeof(MSInfo));
	
	in = sceIoOpen(input, PSP_O_RDONLY, 0777); // open the input
	if(in < 1){return -1;}
	if(type == 0){
		sceIoRead(in, buf, 512);

		MSInfo.BootStatus = buf[446]; // assign the data
		MSInfo.StartHead = buf[447];
		MSInfo.StartSector = *(u16*)&buf[448];
		MSInfo.PartitionType = buf[450];
		MSInfo.LastHead = buf[451];
		MSInfo.LastSector = *(u16*)&buf[452];
		MSInfo.AbsSector = (buf[454] & 0xFF) + ((buf[455] & 0xFF)*0x100) + ((buf[456] & 0xFF)*0x10000) + ((buf[457] & 0xFF)*0x1000000);
		MSInfo.TotalSectors = (buf[458] & 0xFF) + ((buf[459] & 0xFF)*0x100) + ((buf[460] & 0xFF)*0x10000) + ((buf[461] & 0xFF)*0x1000000);
		MSInfo.Signature = *(u16*)&buf[510];
		MSInfo.IPLSpace = (MSInfo.AbsSector - 16) * 512; // ipl space (bytes) = (partition start sector - ipl start sector) * sector size
	
		sceIoLseek(in, MSInfo.AbsSector * 512, 0);

		sceIoRead(in, buf, 512);

		MSInfo.SerialNum = (buf[67] & 0xFF) + ((buf[68] & 0xFF)*0x100) + ((buf[69] & 0xFF)*0x10000) + ((buf[70] & 0xFF)*0x1000000);
		memcpy(MSInfo.Label, buf+0x47, 11);
		memcpy(MSInfo.FileSystem, buf+0x52, 8);
	}

	if(type == 0){sceIoLseek(in, 0x2000, 0);} // go to the ipl (if type is 0)

	memset(buf, 0, 4096);
	sceIoRead(in, buf, 4096); // read a block
	sceIoClose(in); // close the input

	u8 sha1[20];
	sceKernelUtilsSha1Digest(buf, 4096, sha1);
	
	sprintf(MSInfo.IPLName, "Unknown IPL");
	MSInfo.IPLSize = -1;

	int size = zipFileRead(ebootpath, EBOOT_PSAR, "IPLs/Checksums.bin", buf);
	for(i = 0; i < (size / 100); i++){
		if(memcmp(sha1, buf+(i*100)+54, 20) == 0){
			sprintf(MSInfo.IPLName, "%s", buf+(i*100));
			MSInfo.IPLSize = (buf[(i*100)+53] & 0xFF) + ((buf[(i*100)+52] & 0xFF)*0x100) + ((buf[(i*100)+51] & 0xFF)*0x10000) + ((buf[(i*100)+50] & 0xFF)*0x1000000);
			break;
		}
	}

	free(buf);

	return 0;
}
int CheckSysInfo(int page)
{
	vlfGuiSetPageControlEnable(1);
	vlfGuiNextPageControl(CheckSysInfo);
	vlfGuiPreviousPageControl(CheckSysInfo);

	ResetScreen(0, 1, 0);

	if(page == 0){vlfGuiCancelPreviousPageControl();}
	if(page == 3){vlfGuiCancelNextPageControl();}

	if(page == 0){CheckSysInfoP1();}
	else if(page == 1){CheckSysInfoP2();}
	else if(page == 2){CheckSysInfoP3();}
	else if(page == 3){CheckSysInfoP4();}

	int i;
	for(i = 0; i < vlf_text_items; i++){if(vlf_texts[i] != NULL){vlfGuiSetTextFontSize(vlf_texts[i], 0.75);}}
}
void CheckSysInfoP1()
{
	char owner_name[32], password[5];

	memset(owner_name, 0, sizeof(owner_name));
	memset(password, 0, sizeof(password));

	vlf_texts[0] = vlfGuiAddTextF(30, 80, "Model: PSP-%04i", GetModel());
	vlf_texts[1] = vlfGuiAddTextF(30, 105, "Region: %s", GetRegion(big_buffer));
	vlf_texts[2] = vlfGuiAddTextF(30, 130, "Installed Firmware: %s", GetFWVersion(big_buffer));
	vlf_texts[3] = vlfGuiAddTextF(30, 155, "Original Firmware: %s", GetShippedFW(big_buffer));

	vlf_texts[4] = vlfGuiAddTextF(230, 80, "Name: %s", GetRegistryValue("/CONFIG/SYSTEM", "owner_name", &owner_name, sizeof(owner_name)));
	vlf_texts[5] = vlfGuiAddTextF(230, 105, "Password: %s", GetRegistryValue("/CONFIG/SYSTEM/LOCK", "password", &password, sizeof(password)));
	vlf_texts[6] = vlfGuiAddTextF(230, 130, "CPU Frequency: %i Mhz", scePowerGetCpuClockFrequency());
	vlf_texts[7] = vlfGuiAddTextF(230, 155, "BUS Frequency: %i Mhz", scePowerGetBusClockFrequency());
}
void CheckSysInfoP2()
{
	vlf_texts[0] = vlfGuiAddTextF(30, 80, "Battery Status: %s", scePowerIsBatteryCharging() == 1 ? "Charge":"In Use");
	vlf_texts[1] = vlfGuiAddTextF(30, 105, "Power Source: %s", scePowerIsBatteryExist() == 1 ? "Battery":"External");
	vlf_texts[2] = vlfGuiAddTextF(30, 130, "Battery Level: %i%%", scePowerGetBatteryLifePercent() < 0 ? 0:scePowerGetBatteryLifePercent());
	vlf_texts[3] = vlfGuiAddTextF(30, 155, "Hours Left: %i:%02i'", scePowerGetBatteryLifeTime() < 0 ? 0:(scePowerGetBatteryLifeTime() / 60), scePowerGetBatteryLifeTime() < 0 ? 0:(scePowerGetBatteryLifeTime() - (scePowerGetBatteryLifeTime() / 60 * 60)));

	vlf_texts[4] = vlfGuiAddTextF(230, 80, "Remain Capacity: %i mAh", scePowerGetBatteryRemainCapacity() < 0 ? 0:scePowerGetBatteryRemainCapacity());
	vlf_texts[5] = vlfGuiAddTextF(230, 105, "Total Capacity: %i mAh", scePowerGetBatteryFullCapacity() < 0 ? 0:scePowerGetBatteryFullCapacity());
	vlf_texts[6] = vlfGuiAddTextF(230, 130, "Battery Temp: %iºC", scePowerGetBatteryTemp() < 0 ? 0:scePowerGetBatteryTemp());
	vlf_texts[7] = vlfGuiAddTextF(230, 155, "Battery Voltage: %0.3fV", scePowerGetBatteryVolt() < 0 ? 0:(float)scePowerGetBatteryVolt() / 1000.0);
}
void CheckSysInfoP3()
{
	vlf_texts[0] = vlfGuiAddTextF(30, 80, "Motherboard: %s", GetMotherboard(big_buffer));
	vlf_texts[1] = vlfGuiAddTextF(30, 105, "Tachyon: 0x%08X", pspGetTachyonVersion());
	vlf_texts[2] = vlfGuiAddTextF(30, 130, "Baryon: 0x%08X", pspGetBaryonVersion());
	vlf_texts[3] = vlfGuiAddTextF(30, 155, "Pommel: 0x%08X", pspGetPommelVersion());

	vlf_texts[4] = vlfGuiAddTextF(230, 80, "UMD Drive Firmware: %s", GetUMDFW(big_buffer));
	vlf_texts[5] = vlfGuiAddTextF(230, 105, "Fuse Id: 0x%llX", pspGetFuseId());
	vlf_texts[6] = vlfGuiAddTextF(230, 130, "Fuse Config: 0x%08X", pspGetFuseConfig());
	vlf_texts[7] = vlfGuiAddTextF(230, 155, "NAND Seed: 0x%08X", pspNandGetScramble());
}
void CheckSysInfoP4()
{
	u8 kirk[4], spock[4], macaddr[512];

	GetMACAddress(macaddr);
	*(u32 *)kirk = pspGetKirkVersion();
	*(u32 *)spock = pspGetSpockVersion();

	vlf_texts[0] = vlfGuiAddTextF(30, 80, "Kirk Version: %c%c%c%c", kirk[3], kirk[2], kirk[1], kirk[0]);
	vlf_texts[1] = vlfGuiAddTextF(30, 105, "Spock Version: %c%c%c%c", spock[3], spock[2], spock[1], spock[0]);
	vlf_texts[2] = vlfGuiAddTextF(30, 130, "WLAN Status: %s", sceWlanGetSwitchState() == 0 ? "Off":"On");
	vlf_texts[3] = vlfGuiAddTextF(30, 155, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

	vlf_texts[4] = vlfGuiAddTextF(230, 80, "NAND Size: %i MB", (pspNandGetPageSize() * pspNandGetPagesPerBlock() * pspNandGetTotalBlocks()) / 1024 / 1024);
	vlf_texts[5] = vlfGuiAddTextF(230, 105, "Time Machine Running: %s", tmmode == 0 ? "No":"Yes");
	vlf_texts[6] = vlfGuiAddTextF(230, 130, GetHENVersion() == 0 ? "HEN Version: Not Running":"HEN Version: ChickHEN R%i", GetHENVersion());
}
int SetBackground()
{
	if(waiticon == NULL){
		int size = 0, btn = GetKeyPress(0);
		if(btn & PSP_CTRL_LTRIGGER){if(bguseflash){bguseflash = 0;}else{bguseflash = 1;}}
		if(btn & PSP_CTRL_RTRIGGER){vlfGuiSetBackgroundSystem(1);return;}

		if(bguseflash == 0){size = zipFileRead(ebootpath, EBOOT_PSAR, "System/Background.bmp", big_buffer);}

		if(size != 0){
			int rand = Rand(0, size / 6176);
			vlfGuiSetBackgroundFileBuffer(big_buffer+(rand*6176), 6176, 1);
			return 1;
		}
		else{
			bguseflash = 1;

			if(FileExists("flash0:/vsh/resource/01-12.bmp")){
				size = GetFileSize("flash0:/vsh/resource/01-12.bmp");
				int size2 = GetFileSize("flash0:/vsh/resource/13-27.bmp");
				int rand = Rand(0, (size + size2) / 6176);

				if(rand < size / 6176){ReadFile("flash0:/vsh/resource/01-12.bmp", (rand * 6176), big_buffer, 6176);}
				else{ReadFile("flash0:/vsh/resource/13-27.bmp", ((rand * 6176) - size), big_buffer, 6176);}

				vlfGuiSetBackgroundFileBuffer(big_buffer, 6176, 1);

				return 1;
			}
			else{
				vlfGuiSetBackgroundSystem(1);
				return 0;
			}
		}
	}
}
/*
	File Helpers
*/
void LoadWave()
{
	int size = zipFileRead(ebootpath, EBOOT_PSAR, "System/Wave.omg", big_buffer);
	
	if(size > 0)
	{
		ScePspFMatrix4 matrix;
		ScePspFVector3 scale;
		vlfGuiSetModel(big_buffer, size);

		scale.x = scale.y = scale.z = 8.5f;
		gumLoadIdentity(&matrix);
		gumScale(&matrix, &scale);
		vlfGuiSetModelWorldMatrix(&matrix);
	}
}
void FormatMS(char *msname, int iplreservedspace)
{
	/*
		MBR Layout
		446 bytes = Boot Code (commonly all 0x0)
		16 bytes = Partition 1 description
		16 bytes = Partition 2 description (filled with 0x0 if partition not needed)
		16 bytes = Partition 3 description (filled with 0x0 if partition not needed)
		16 bytes = Partition 4 description (filled with 0x0 if partition not needed)
		2 bytes = 0x55AA (use as a sanity check, must be 0x55AA if formatted correctly)

		Partition Description Layout
		1 byte = Boot Flag
		4 bytes = CHS Begin (should be ignored, should be able to be dummied)
		1 byte = Type Code (FAT32 = 0x0B or 0x0C) (should be ignored, should be able to be dummied)
		4 bytes = CHS End (this is where the file system is located on the disk) (should be ignored, should be able to be dummied)
		4 bytes = LBA Begin
		4 bytes = Number of Sectors

		Cluster Size = Number of sectors per cluster * Sector size
	*/
	int read, written, seeked, tmp;
	int old_partpreceed, old_partsize;
	int new_partpreceed, new_partsize, new_partclustersize, new_partstartsize = sizeof(big_buffer);
	char mbr[512], bootrecord[512];

	memset(big_buffer, 0, sizeof(big_buffer));
	zipFileRead(ebootpath, EBOOT_PSAR, "System/Partition Start.bin", big_buffer); // read the default partition start
	if(new_partstartsize < 0){ErrorReturn("Unable to read the Partition Start file from the EBOOT");return;}
	if(new_partstartsize != (new_partstartsize / 512) * 512){new_partstartsize = (new_partstartsize + 512) & 0xFFFFFE00;}

	int cont = vlfGuiMessageDialog("Do you want to format the Memory Stick?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
	if(cont != 1){OnBackToMainMenu(0);return;}

	cont = vlfGuiMessageDialog("All data on the Memory Stick will be deleted.\nAre you sure you want to continue?", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_YESNO|VLF_MD_INITIAL_CURSOR_NO);
	if(cont != 1){OnBackToMainMenu(0);return;}

	SetStatus(0, 0, 20, 120, VLF_ALIGNMENT_LEFT, "Formatting...\nDo not remove the Memory Stick or turn off the power.");

	if(strlen(msname) > 11){memcpy(big_buffer+0x2B, msname, 11);memcpy(big_buffer+0x40000, msname, 11);} // patch in the memory stick name
	else{memcpy(big_buffer+0x2B, msname, strlen(msname));memcpy(big_buffer+0x40000, msname, strlen(msname));}

	big_buffer[39] = Rand(0, 0xFF); // patch in a random id
	big_buffer[40] = Rand(0, 0xFF);
	big_buffer[41] = Rand(0, 0xFF);
	big_buffer[42] = Rand(0, 0xFF);

	read = ReadFile("msstor:", 0, mbr, 512); // read the current mbr
	if(read != 512){ErrorReturn("Unable to read the MBR from the Memory Stick");return;}
	old_partpreceed = (mbr[454] & 0xFF) + ((mbr[455] & 0xFF)*0x100) + ((mbr[456] & 0xFF)*0x10000) + ((mbr[457] & 0xFF)*0x1000000);

	read = ReadFile("msstor:", old_partpreceed * 512, bootrecord, 512);
	if(read < 0){ErrorReturn("Unable to read the Boot Record from the Memory Stick");return;}
	old_partsize = (bootrecord[32] & 0xFF) + ((bootrecord[33] & 0xFF)*0x100) + ((bootrecord[34] & 0xFF)*0x10000) + ((bootrecord[35] & 0xFF)*0x1000000);

	new_partpreceed = (iplreservedspace * 2) + 16;
	new_partsize = old_partsize + old_partpreceed - new_partpreceed - 64;

	tmp = (old_partsize * 512) / 1024 / 1024; // get a mew cluster size
	new_partclustersize = 0x28;
	if(tmp > 55){new_partclustersize = 0x10;}
	if(tmp > 110){new_partclustersize = 0x20;}
	if(tmp > 900){new_partclustersize = 0x40;}
	if(tmp > 3500){new_partclustersize = 0x80;}

	mbr[454] = new_partpreceed & 0xFF; // patch the partition preceed into the mbr
	mbr[455] = (new_partpreceed & 0xFF00) / 0x100;
	mbr[456] = (new_partpreceed & 0xFF0000) / 0x10000;
	mbr[457] = (new_partpreceed & 0xFF000000) / 0x1000000;

	mbr[458] = new_partsize & 0xFF; // patch the partition size into the mbr
	mbr[459] = (new_partsize & 0xFF00) / 0x100;
	mbr[460] = (new_partsize & 0xFF0000) / 0x10000;
	mbr[461] = (new_partsize & 0xFF000000) / 0x1000000;

	mbr[447] = mbr[448] = mbr[449] = mbr[451] = mbr[452] = mbr[453] = 0xFF; // dummy out chs
	mbr[450] = 0x0E;

	big_buffer[13] = new_partclustersize; // patch the partition start
	big_buffer[32] = new_partsize & 0xFF;
	big_buffer[33] = (new_partsize & 0xFF00) / 0x100;
	big_buffer[34] = (new_partsize & 0xFF0000) / 0x10000;
	big_buffer[35] = (new_partsize & 0xFF000000) / 0x1000000;

	int out = sceIoOpen("msstor:", PSP_O_RDONLY|PSP_O_WRONLY, 0777); // open the ms for writing

	written = sceIoWrite(out, mbr, 512); // write the new mbr
	if(written != 512){SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Failure writing the MBR to the Memory Stick");}

	seeked = sceIoLseek(out, new_partpreceed * 512, SEEK_SET);
	if(seeked != new_partpreceed * 512){SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Failure seeking to the new partition start");}

	//new_partstartsize
	written = sceIoWrite(out, big_buffer, new_partstartsize);
	if(written != new_partstartsize){SetStatus(0, 0, 240, 120, VLF_ALIGNMENT_CENTER, "Failure writing the Partition Start Blocks to the Memory Stick");}

	sceIoClose(out);

	vlfGuiMessageDialog("Format completed. The game will now exit to reduce the risk of system instability.", VLF_MD_TYPE_NORMAL|VLF_MD_BUTTONS_NONE);
	sceKernelExitGame();
}
