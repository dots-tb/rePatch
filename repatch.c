/* 
rePatch 1.0 reDux0 -- PATCHING WITH FREEDOM
   Brought to you by SilicaTeam 2.0 --
      Dev and "reV ur engines" by @dots_tb
	  
       with support from @SilicaAndPina and @CelesteBlue123 (especially his """holy grail""")

   Testing team:
    AlternateZero
    bopz
	@IcySon55
	DuckySushi
	Analog Man
	Pingu (@mcdarkjedii) 
	Radziu
	
   Special thanks to:
    VitaPiracy, especially Radziu for shilling it
    Motoharu for his RE work on the wiki
	TheFlow for creating a need for this plugin
	
   No thanks to:
	Coderx3(Banana man)

  Based off ioPlus by @dots_tb: https://github.com/CelesteBlue-dev/PSVita-RE-tools/tree/master/ioPlus/
  */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/syslimits.h>
#include <vitasdkkern.h>
#include <taihen.h>

#define printf ksceDebugPrintf

static int hooks_uid[3];
static tai_hook_ref_t ref_hooks[3];

#define rePatchFolder "ux0:rePatch"

//https://wiki.henkaku.xyz/vita/SceIofilemgr
typedef struct io_scheduler_item //size is 0x14 - allocated from SceIoScheduler heap
{
   uint32_t* unk_0; // parent
   uint32_t unk_4; // 0
   uint32_t unk_8; // 0
   uint32_t unk_C; // 0
   uint32_t unk_10; // pointer to unknown module data section
} io_scheduler_item;


static int getNewPath(const char *old_path, char *new_path, SceUID pid, size_t maxPath, int opt) {
	char *old_path_file = strchr(old_path, ':') + 1;
	if(old_path_file[0] == '/')
		old_path_file++;
	if(opt) {
		char titleid[32];
		if(ksceKernelGetProcessTitleId(pid, titleid, sizeof(titleid))<0)
			return 0;
		snprintf(new_path, maxPath, rePatchFolder"/%s/%s",titleid,old_path_file);
	} else {
		if((old_path_file = strchr(old_path_file, '/'))==NULL) 
			return 0;
		snprintf(new_path, maxPath, rePatchFolder"%s",old_path_file);
	}
	SceIoStat k_stat;
	if(ksceIoGetstat(new_path, &k_stat)<0)
		return 0;
	int len = strnlen(new_path, maxPath);
	if(len < maxPath)
		return len;
	return 0;
}

static char *confirmPatch(const char *filename) { //For future support of future things.
	char *old_path_file = strchr(filename, ':') + 1;
	if(old_path_file[0] == '/')
		old_path_file++;
	if(memcmp(old_path_file,"patch",sizeof("patch")-1)==0
		||memcmp(old_path_file,"app",sizeof("app")-1)==0) 
			return old_path_file;
	return 0;
}

static int sceFiosKernelOverlayResolveSyncForDriver_patched(SceUID pid, int resolveFlag, const char *pInPath, char *pOutPath, size_t maxPath) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	if (memcmp("app0:", pInPath, sizeof("app0:") - 1)==0 && getNewPath(pInPath, pOutPath, pid, maxPath, 1))
		ret = 0;
	if(ret < 0)ret = TAI_CONTINUE(int, ref_hooks[0], pid, resolveFlag, pInPath, pOutPath, maxPath);
	EXIT_SYSCALL(state);
	return ret;
}

static char newPath[PATH_MAX];
static int ksceIoOpen_patched(const char *filename, int flag, SceIoMode mode) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	if((flag&SCE_O_WRONLY) != SCE_O_WRONLY && ksceSblACMgrIsShell(0)) {
		if(confirmPatch(filename) && strstr(filename, "/eboot.bin")!=NULL) {
			if(getNewPath(filename, newPath, 0, sizeof(newPath), 0))
				ret = TAI_CONTINUE(int, ref_hooks[1], newPath, flag, mode);
		}
	} 
	if(ret <= 0) ret = TAI_CONTINUE(int, ref_hooks[1], filename, flag, mode);
	EXIT_SYSCALL(state);
	return ret;
}


static int io_item_thing_patched(io_scheduler_item *item, int r1) {
	int ret, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[2], item, r1);
	if(ret == 0x80010013 &&item->unk_10 == 0x800) {
		item->unk_10 = 1;
		//ret = TAI_CONTINUE(int, ref_hooks[1], item, r1);
	}	
	EXIT_SYSCALL(state);
	return ret;
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {
	hooks_uid[0] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[0], "SceFios2Kernel", TAI_ANY_LIBRARY, 0x0F456345, sceFiosKernelOverlayResolveSyncForDriver_patched);
	hooks_uid[1] = taiHookFunctionImportForKernel(KERNEL_PID, &ref_hooks[1], "SceKernelModulemgr", TAI_ANY_LIBRARY, 0x75192972, ksceIoOpen_patched);

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &tai_info);

	switch(tai_info.module_nid) {
		case 0xA96ACE9D://3.65
			hooks_uid[2] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[2], tai_info.modid, 0, 0xb3d8, 1,  io_item_thing_patched);
			break;
		case 0x9642948C://3.60
			hooks_uid[2] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[2], tai_info.modid, 0, 0xd400, 1, io_item_thing_patched);
			break;
	}	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	if (hooks_uid[0] >= 0) taiHookReleaseForKernel(hooks_uid[0], ref_hooks[0]);   
	if (hooks_uid[1] >= 0) taiHookReleaseForKernel(hooks_uid[1], ref_hooks[1]);   
	if (hooks_uid[2] >= 0) taiHookReleaseForKernel(hooks_uid[2], ref_hooks[2]);   
	return SCE_KERNEL_STOP_SUCCESS;
}