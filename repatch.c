/* 
rePatch 2.68 reDux0 -- PATCHING WITH FREEDOM
	Brought to you by SilicaTeam 2.0 --
	
		Dev and "reV ur engines" by @dots_tb
		
	with support from @Nkekev @SilicaAndPina and @CelesteBlue123 (especially his """holy grail""")

Testing team:
    AlternativeZero	bopz
	@IcySon55		DuckySushi
	AnalogMan		Pingu (@mcdarkjedii) 
	amadeus			jeff7360
	Radziu (@AluProductions)
	
Special thanks to:
	VitaPiracy, especially Radziu for shilling it
	The translation community for being supportive of rePatch and its development
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

#include "repatch.h"
//#define printf ksceDebugPrintf
#define HOOKS_NUMBER 6

static int hooks_uid[HOOKS_NUMBER];
static tai_hook_ref_t ref_hooks[HOOKS_NUMBER];
static char newPath[PATH_MAX];

static int getNewPath(const char *old_path, char *new_path, SceUID pid, size_t maxPath, int opt) {
	char *old_path_file = (char *)old_path;
	SceIoStat k_stat;
	if(!(opt & NO_DEVICE)) {
		old_path_file = strchr(old_path_file, ':') + 1;
		if(old_path_file[0] == '/')
			old_path_file++;
	}
	if(opt & TRUNC_PATH) {
		if((old_path_file = strchr(old_path_file, '/'))==NULL) 
			return 0;
		snprintf(new_path, maxPath, rePatchFolder"%s", old_path_file);
	} else if(opt & TITLE_PATH) {
		char titleid[32];
		if(ksceKernelGetProcessTitleId(pid, titleid, sizeof(titleid))<0)
			return 0;
		if(opt & DLC_PATH) {
			if((opt & IS_DIR) && new_path[0] != 0 && memcmp("PD", new_path, sizeof("PD") - 1) == 0) {
				if(old_path_file[0] == 0 || (opt & NO_DUMBY))
					return 0;
				int dfd = ksceIoDopen(new_path);
				SceIoDirent dir;
				memset(&dir, 0, sizeof(dir));
				if(dfd >= 0) {
					int ret = ksceIoDread(dfd, &dir);
					ksceIoDclose(dfd);
					if(ret)
						return 0;
				}										
			}
			snprintf(new_path, maxPath, addcontFolder"/%s", titleid, old_path_file);			
		} else 
			snprintf(new_path, maxPath, rePatchFolder"/%s/%s", titleid, old_path_file);
		int i = 0;
		while(new_path[i] != 0) {
			if(new_path[i++]=='\\')
				new_path[i-1]='/';
		}
		
	}
	if(ksceIoGetstat(new_path, &k_stat)<0) 
		return 0;
	return 1;
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
	if(!ksceSblACMgrIsShell(0)) {
		if (memcmp("app0:", pInPath, sizeof("app0:") - 1)==0 && getNewPath(pInPath, pOutPath, pid, maxPath, (TITLE_PATH)))
			ret = 0;
		else if (memcmp("addco", pInPath, sizeof("addco") - 1)==0) {
			if(strchr(pInPath + sizeof("addcont0:/") + ADDCONT_THRES, '/') == NULL) {
				ret = TAI_CONTINUE(int, ref_hooks[0], pid, resolveFlag, pInPath, pOutPath, maxPath);
				if(getNewPath(pInPath, pOutPath, pid, maxPath, (DLC_PATH|TITLE_PATH|IS_DIR|NO_DUMBY)))
					ret = 0;
			} else if(getNewPath(pInPath, pOutPath, pid, maxPath, (DLC_PATH|TITLE_PATH)))
				ret = 0;
		}
	}
	if(ret < 0) ret = TAI_CONTINUE(int, ref_hooks[0], pid, resolveFlag, pInPath, pOutPath, maxPath);
	
	EXIT_SYSCALL(state);
	return ret;
}

static int sceFiosKernelOverlayResolveFolder_patched(SceUID pid, const char *pInPath, mount_point_overlay *mount_info, int r3, int r4, char *pOutPath) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	if(!ksceSblACMgrIsShell(0)) {
		if (memcmp("addco", pInPath, sizeof("addco") - 1)==0) {
			if(memcmp("addco", mount_info->path, sizeof("addco") - 1) == 0 || memcmp("app0", mount_info->path, sizeof("app0") - 1) == 0) {
				ret = TAI_CONTINUE(int, ref_hooks[4], pid, pInPath, mount_info, r3, r4, pOutPath);	
				if(getNewPath(pInPath, pOutPath, pid, PATH_MAX, (DLC_PATH|TITLE_PATH|IS_DIR)))
					ret = 1;
			}
		}
	}
	if(ret < 0) ret = TAI_CONTINUE(int, ref_hooks[4], pid, pInPath, mount_info, r3, r4, pOutPath); 
	EXIT_SYSCALL(state);
	return ret;
}

static int ksceIoOpen_patched(const char *filename, int flag, SceIoMode mode) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	if((flag & SCE_O_WRONLY) != SCE_O_WRONLY && ksceSblACMgrIsShell(0)) {
		if(confirmPatch(filename) && strstr(filename, "/eboot.bin")!=NULL) {
			if(getNewPath(filename, newPath, 0, sizeof(newPath), (TRUNC_PATH)))
				ret = TAI_CONTINUE(int, ref_hooks[1], newPath, flag, mode);
		}
	} 
	if(ret <= 0) ret = TAI_CONTINUE(int, ref_hooks[1], filename, flag, mode);
	EXIT_SYSCALL(state);
	return ret;
}

static int sceAppMgrDrmOpenForDriver_patched(drm_opts *drmOpt) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[2], drmOpt);
	if(ret < 0 && !ksceSblACMgrIsShell(0)) {
		if(getNewPath(drmOpt->adcont_id, newPath, ksceKernelGetProcessId(), sizeof(newPath), (DLC_PATH|TITLE_PATH|NO_DEVICE))) 
			ret = 0;
	}
	EXIT_SYSCALL(state);
	return ret;
}

static int sceAppMgrDrmCloseForDriver_patched(drm_opts *drmOpt) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[3], drmOpt);
	if(ret < 0 && !ksceSblACMgrIsShell(0)) 
		ret = 0;
	EXIT_SYSCALL(state);
	return ret;
}


static int io_item_thing_patched(io_scheduler_item *item, int r1) {
	int ret, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[5], item, r1);
	if(ret == 0x80010013 &&item->unk_10 == 0x800) 
		item->unk_10 = 1;
	EXIT_SYSCALL(state);
	return ret;
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {
	hooks_uid[0] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[0], "SceFios2Kernel", TAI_ANY_LIBRARY, 0x0F456345, sceFiosKernelOverlayResolveSyncForDriver_patched);
	hooks_uid[1] = taiHookFunctionImportForKernel(KERNEL_PID, &ref_hooks[1], "SceKernelModulemgr", TAI_ANY_LIBRARY, 0x75192972, ksceIoOpen_patched);
	hooks_uid[2] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[2], "SceAppMgr", TAI_ANY_LIBRARY, 0xEA75D157, sceAppMgrDrmOpenForDriver_patched);
	hooks_uid[3] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[3], "SceAppMgr", TAI_ANY_LIBRARY, 0x088670A6, sceAppMgrDrmCloseForDriver_patched);

	tai_module_info_t tai_info;
	tai_info.size = sizeof(tai_module_info_t);
	taiGetModuleInfoForKernel(KERNEL_PID, "SceFios2Kernel", &tai_info);
	
	hooks_uid[4] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[4], tai_info.modid, 0, 0x1920, 1,  sceFiosKernelOverlayResolveFolder_patched);
	
	memset(&tai_info,0,sizeof(tai_module_info_t));
	tai_info.size = sizeof(tai_module_info_t);
	taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &tai_info);

	switch(tai_info.module_nid) {
		case 0xA96ACE9D://3.65
			hooks_uid[5] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[5], tai_info.modid, 0, 0xb3d8, 1,  io_item_thing_patched);
			break;
		case 0x9642948C://3.60
			hooks_uid[5] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[5], tai_info.modid, 0, 0xd400, 1, io_item_thing_patched);
			break;
	}
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	for (int i=0; i < HOOKS_NUMBER; i++)
		if (hooks_uid[i] >= 0) taiHookReleaseForKernel(hooks_uid[i], ref_hooks[i]);   
	return SCE_KERNEL_STOP_SUCCESS;
}
