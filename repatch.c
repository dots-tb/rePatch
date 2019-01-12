/* 
rePatch v3.0 reDux0 -- PATCHING WITH FREEDOM
	Brought to you by SilicaTeam 2.0 --
	
		Dev and "reV ur engines" by @dots_tb @CelesteBlue123 (especially his """holy grail"""  and self_auth info)
		
	with support from @Nkekev @SilicaAndPina

Testing team:
	AlternativeZero	bopz
	@IcySon55		DuckySushi
	AnalogMan		Pingu (@mcdarkjedii) 
	amadeus			jeff7360
	Radziu (@AluProductions)
	@RealYoti		@froid_san
	@waterflame321
	
Special thanks to:
	VitaPiracy, especially Radziu for shilling it
	The translation community for being supportive of rePatch and its development
	Motoharu for his RE work on the wiki
	TheFlow for creating a need for this plugin
*/

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/syslimits.h>
#include <vitasdkkern.h>
#include <taihen.h>

#include "repatch.h"
#include "self.h"
#define printf ksceDebugPrintf
#define HOOKS_NUMBER 7

static int hooks_uid[HOOKS_NUMBER];
static tai_hook_ref_t ref_hooks[HOOKS_NUMBER];

static mount_point_overlay addcont_overlay;
static mount_point_overlay repatch_overlay;

int (*_sceFiosKernelOverlayRemoveForProcessForDriver)(SceUID pid, uint32_t id);

int checkFile(const char *filename) {
	SceIoStat k_stat;
	return !ksceIoGetstat(filename, &k_stat);
}

void stripDevice(const char *inPath, char *outPath) {
	char *old_path_file =  strchr(inPath, ':') + 1;
	old_path_file = (old_path_file[0] == '/') ? strchr(old_path_file + 1, '/') + 1 : strchr(old_path_file, '/') + 1;
	snprintf(outPath, PATH_MAX, rePatchFolder"/%s", old_path_file);
}

static char temp_path[PATH_MAX];
static int resolveFolder(char *filepath) {
	for(int i = 0; i < DEVICES_AMT; i++) {
		snprintf(temp_path, sizeof(temp_path), "%s/%s", DEVICES[i], filepath);
		if(checkFile(temp_path))
			return (strncpy(filepath, temp_path, 292) != NULL);
	}
	return checkFile(filepath);
}

static char manu_patch[PATH_MAX];
static int overlayHandler(uint32_t pid, mount_point_overlay *overlay_old, mount_point_overlay *overlay_new, int opt) {
	if(overlay_new->PID == pid && overlay_new->mountId > 0)
		_sceFiosKernelOverlayRemoveForProcessForDriver(pid, overlay_new->mountId);
	overlay_new->mountId = 0;
	overlay_new->PID = pid;
	overlay_new->order = 0x85;
	overlay_new->type = 1;
	if(opt & AIDS_PATH)
		strncpy(overlay_new->dst, "addcont0:", sizeof(overlay_new->dst));
	else
		strncpy(overlay_new->dst, overlay_old->src, sizeof(overlay_new->dst));
	char titleid[32];
	if(ksceKernelGetProcessTitleId(pid, titleid, sizeof(titleid))==0) {
		if(opt & APP_PATH)
			snprintf(overlay_new->src, sizeof(overlay_new->src), rePatchFolder"/%s", titleid);
		else if((opt & DLC_PATH) || (opt & AIDS_PATH))
			snprintf(overlay_new->src, sizeof(overlay_new->src), addcontFolder"/%s", titleid);
		else if((opt & MANU_PATH) && (strncmp("NPXS10027", titleid, sizeof("NPXS10027"))==0 || strncmp("main", titleid, sizeof("main"))==0)) 
			strncpy(overlay_new->src, manu_patch, sizeof(manu_patch));
	}
	int ret = resolveFolder(overlay_new->src);
	overlay_new->dst_len = strnlen(overlay_new->dst, sizeof(overlay_new->dst));
	overlay_new->src_len = strnlen(overlay_new->src, sizeof(overlay_new->src));
	return ret;
}

static int sceFiosKernelOverlayAddForProcessForDriver_patched(uint32_t pid, mount_point_overlay *overlay, uint32_t *outID) {
	int ret = -1, state;
	uint32_t repatch_outID =0;
	ENTER_SYSCALL(state);
	if(ksceSblACMgrIsGameProgram(pid)) {
		if(strncmp(overlay->dst, "app0:", sizeof("app0:")) == 0) {
			if(overlayHandler(pid, overlay, &repatch_overlay, APP_PATH))
			repatch_overlay.mountId = TAI_CONTINUE(int, ref_hooks[0], pid, &repatch_overlay, &repatch_outID);
				repatch_overlay.mountId =  repatch_outID;
			repatch_outID = 0;
			if(overlayHandler(pid, NULL, &addcont_overlay, AIDS_PATH))
				addcont_overlay.mountId =  TAI_CONTINUE(int, ref_hooks[0], pid, &addcont_overlay, &repatch_outID);
			addcont_overlay.mountId = repatch_outID;
		} else if(strncmp(overlay->dst, "addcont0:", sizeof("addcont0:")) == 0 && overlayHandler(pid, overlay, &addcont_overlay, DLC_PATH)) {
			addcont_overlay.mountId = TAI_CONTINUE(int, ref_hooks[0], pid, &addcont_overlay, &repatch_outID);
			addcont_overlay.mountId =  repatch_outID;
		}
	}				
	if(strncmp(overlay->dst, repatch_overlay.dst, sizeof(repatch_overlay.dst)) == 0 && overlayHandler(pid, overlay, &repatch_overlay, PATCH_PATH)) {
			repatch_overlay.mountId = TAI_CONTINUE(int, ref_hooks[0], pid, &repatch_overlay, &repatch_outID);
			repatch_overlay.mountId =  repatch_outID;	
	} else if(strncmp("gp", overlay->dst, sizeof("gp") - 1) == 0 && overlayHandler(pid, overlay, &repatch_overlay, MANU_PATH)) {
		TAI_CONTINUE(int, ref_hooks[0], pid, &repatch_overlay, &repatch_outID);
		repatch_overlay.mountId =  repatch_outID;
	}
	ret = TAI_CONTINUE(int, ref_hooks[0], pid, overlay, outID);
	EXIT_SYSCALL(state);
	return ret;
}

static char repatch_path[PATH_MAX];
static SceSelfAuthInfo self_auth_info;

static int ksceSblAuthMgrAuthHeaderForKernel_patched(int ctx, char *header, int header_size, SceSblSmCommContext130 *context_130){
	int ret = -1, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[6], ctx, header, header_size, context_130);
	SCE_header *shdr = (SCE_header *)header;
	SCE_appinfo *appinfo = (SCE_appinfo *)(header + shdr->appinfo_offset);
	if(context_130->self_auth_info_caller.program_authority_id  == self_auth_info.program_authority_id || appinfo->authid == self_auth_info.program_authority_id) {
		memcpy((char*)(context_130->self_auth_info.capability), (char*)&self_auth_info + 0x10, 0x40);
		if (context_130->self_auth_info.capability[0] == 0x10) 
			((char *)&context_130->self_auth_info.program_authority_id)[7];
	}
	EXIT_SYSCALL(state);
	return ret;
}
static char eboot_path[PATH_MAX];
static int ksceIoOpen_patched(const char *filename, int flag, SceIoMode mode) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	if ((flag & SCE_O_WRONLY) != SCE_O_WRONLY && ksceSblACMgrIsShell(0) && (strncmp(filename, "ux0:", sizeof("ux0:") -1) == 0) && strstr(filename, "/eboot.bin") != NULL){
			stripDevice(filename, eboot_path);
			resolveFolder(eboot_path);
			if((ret = TAI_CONTINUE(int, ref_hooks[1], eboot_path, flag, mode))>0) {
				strncpy(repatch_path, eboot_path, sizeof(repatch_path));
				char *end_path = strstr(repatch_path, "eboot.bin");
				*end_path = 0;
				snprintf(eboot_path, PATH_MAX, "%sself_auth.bin", repatch_path);
				SceUID fd = ksceIoOpen(eboot_path, SCE_O_RDONLY, 0);
				if (fd >= 0) {
					if (ksceIoRead(fd, &self_auth_info, 0x90) != 0x90)
						memset(&self_auth_info, 0, sizeof(self_auth_info));
					ksceIoClose(fd);
				}
				if (hooks_uid[6] <= 0)
					hooks_uid[6] = taiHookFunctionImportForKernel(KERNEL_PID, &ref_hooks[6], "SceKernelModulemgr", TAI_ANY_LIBRARY, 0xF3411881, ksceSblAuthMgrAuthHeaderForKernel_patched);
			}		
	}
	if(ret <= 0) ret = TAI_CONTINUE(int, ref_hooks[1], filename, flag, mode);
	EXIT_SYSCALL(state);
	return ret;
}

static int confirmDlc(char *filepath, const char *adcont_id) {
	snprintf(filepath, PATH_MAX, "%s/%s", addcont_overlay.src, adcont_id);
	return checkFile(filepath);
}

static char dlc_path[PATH_MAX];
static int sceAppMgrDrmOpenForDriver_patched(drm_opts *drmOpt, int r2) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[2], drmOpt, r2);
	if(ret < 0 && !ksceSblACMgrIsShell(0)) 
		ret = confirmDlc(dlc_path, drmOpt->adcont_id) ? 0 : ret;
	EXIT_SYSCALL(state);
	return ret;
}

static char dlc_path2[PATH_MAX];
static int sceAppMgrDrmCloseForDriver_patched(drm_opts *drmOpt, int r2) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	ret = TAI_CONTINUE(int, ref_hooks[3], drmOpt, r2);
	if(ret < 0 && !ksceSblACMgrIsShell(0))
		ret = confirmDlc(dlc_path2, drmOpt->adcont_id) ? 0 : ret;
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

static int ksceAppMgrGameDataMount_patched(char *input, int r2, int r3, char *outpath) {
	int ret = -1, state;
	ENTER_SYSCALL(state);
	stripDevice(input, manu_patch);
	ret = TAI_CONTINUE(int, ref_hooks[4], input, r2, r3, outpath);
	EXIT_SYSCALL(state);
	return ret;
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {
	module_get_export_func(KERNEL_PID, "SceFios2Kernel", TAI_ANY_LIBRARY, 0x23247EFB, &_sceFiosKernelOverlayRemoveForProcessForDriver);
	
	hooks_uid[0] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[0], "SceFios2Kernel", TAI_ANY_LIBRARY, 0x17E65A1C, sceFiosKernelOverlayAddForProcessForDriver_patched);
	hooks_uid[1] = taiHookFunctionImportForKernel(KERNEL_PID, &ref_hooks[1], "SceKernelModulemgr", TAI_ANY_LIBRARY, 0x75192972, ksceIoOpen_patched);
	hooks_uid[2] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[2], "SceAppMgr", TAI_ANY_LIBRARY, 0xEA75D157, sceAppMgrDrmOpenForDriver_patched);
	hooks_uid[3] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[3], "SceAppMgr", TAI_ANY_LIBRARY, 0x088670A6, sceAppMgrDrmCloseForDriver_patched);
	hooks_uid[4] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hooks[4], "SceAppMgr", TAI_ANY_LIBRARY, 0xCE356B2D, ksceAppMgrGameDataMount_patched);

	tai_module_info_t tai_info;
	
	memset(&tai_info,0,sizeof(tai_module_info_t));
	tai_info.size = sizeof(tai_module_info_t);
	taiGetModuleInfoForKernel(KERNEL_PID, "SceIofilemgr", &tai_info);

	switch(tai_info.module_nid) {
		case 0xA96ACE9D://3.65
		case 0x90DA33DE://3.68
			hooks_uid[5] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[5], tai_info.modid, 0, 0xb3d8, 1,  io_item_thing_patched);
			break;
		case 0x9642948C://3.60
			hooks_uid[5] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[5], tai_info.modid, 0, 0xd400, 1, io_item_thing_patched);
			break;
		default:
			hooks_uid[5] =  taiHookFunctionOffsetForKernel(KERNEL_PID, &ref_hooks[5], tai_info.modid, 0, 0xb3d8, 1,  io_item_thing_patched);
			break;
	}
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	for (int i=0; i < HOOKS_NUMBER; i++)
		if (hooks_uid[i] >= 0) taiHookReleaseForKernel(hooks_uid[i], ref_hooks[i]);   
	return SCE_KERNEL_STOP_SUCCESS;
}
