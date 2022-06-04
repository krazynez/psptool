#include <pspsdk.h>
#include <pspkernel.h>
#include <stdio.h>
#include <vlf.h>
#include <PRXs.h>
#include <common.h>

extern int app_main(SceSize args, void *argp);
extern char *path;

int start_thread(SceSize args, void *argp)
{
    path = (char *)argp;
    int last_trail = -1;
    int i;

    if(path){
       for (i = 0; path[i]; i++){
          if (path[i] == '/')
             last_trail = i;
       }
    }

	if(last_trail >= 0){path[last_trail] = 0;}

	sceIoChdir(path);

	if(GetFileSize("kprx.prx") == size_kprx){LoadStartModule("kprx.prx");}
	else{LoadStartModuleBuffer("kprx.prx", kprx, size_kprx);}

	if(sceKernelDevkitVersion() > 0x02070110){
		if(GetFileSize("iop.prx") == size_iop){LoadStartModule("iop.prx");}
		else{LoadStartModuleBuffer("iop.prx", iop, size_iop);}
	}

	if(GetFileSize("intraFont.prx") == size_intraFont){LoadStartModule("intraFont.prx");}
	else{LoadStartModuleBuffer("intraFont.prx", intraFont, size_intraFont);}

	if(GetFileSize("vlf.prx") == size_vlf){LoadStartModule("vlf.prx");}
	else{LoadStartModuleBuffer("vlf.prx", vlf, size_vlf);}

	LoadStartModuleDirectory("modules");

	vlfGuiInit(-1, app_main);

	return sceKernelExitDeleteThread(0);
}
int module_start(SceSize args, void *argp)
{
	SceUID thid = sceKernelCreateThread("start_thread", start_thread, 0x10, 0x4000, 0, NULL);
	if (thid < 0)
		return thid;

	sceKernelStartThread(thid, args, argp);
	
	return 0;
}