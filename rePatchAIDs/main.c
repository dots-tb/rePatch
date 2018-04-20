/* 
rePatch Addcont Deficiency Injection Supplement -- AIDS WITH FREEDOM
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
	
No thanks to:
	Coderx3(Banana man)

Uses code from TheFlow's vitashell (you don't need a link for that)
*/
#include <stdio.h>
#include <sys/syslimits.h>
#include <stdlib.h>
#include <string.h>
#include <vitasdk.h>
#include <taihen.h>
#include <vitashell_user.h>

#include "graphics.h"
#include "../repatch.h"

#define TITLE_ID "REPATCH01"

char g_currentMount[16];
char g_currentPath[PATH_MAX];

int pfsMount(const char *path) {
	char klicensee[0x10];
	ShellMountIdArgs args;

	memset(klicensee, 0, sizeof(klicensee));

	args.process_titleid = TITLE_ID;
	args.path = path;
	args.desired_mount_point = NULL;
	args.klicensee = klicensee;
	args.mount_point = g_currentMount;

	args.id = 0x3EA;
	return shellUserMountById(&args);
}

int pfsUmount(const char *path) {
	if (path[0] == 0)
	return -1;
	int res = sceAppMgrUmount(path);
	return res;
}

int checkIfDummy(const char *titleid, const char *dlcid) {
	char currentPath[PATH_MAX];
	snprintf(currentPath, PATH_MAX, "ux0:addcont/%s/%s", titleid, dlcid);
	int dfd = sceIoDopen(currentPath);
	if(dfd < 0)
	return 0;
	SceIoDirent dir_stat;
	int ret=sceIoDread(dfd, &dir_stat);
	sceIoDclose(dfd);
	if(ret)
		return 0;
	return 1;
}
int hasEndSlash(const char *path) {
	return path[strlen(path) - 1] == '/';
}

int removePath(const char *path) {
	psvDebugScreenPrintf("Deleting: %s\n", path);
	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				char *new_path = malloc(strlen(path) + strlen(dir.d_name) + 2);
				snprintf(new_path, PATH_MAX - 1, "%s%s%s", path, hasEndSlash(path) ? "" : "/", dir.d_name);

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					int ret = removePath(new_path);
					if (ret <= 0) {
						free(new_path);
						sceIoDclose(dfd);
						return ret;
					}
				} else {
					int ret = sceIoRemove(new_path);
					if (ret < 0) {
						free(new_path);
						sceIoDclose(dfd);
						return ret;
					}
				}
				free(new_path);
			}
		} while (res > 0);

		sceIoDclose(dfd);

		int ret = sceIoRmdir(path);
		if (ret < 0)
			return ret;
	} else {
		int ret = sceIoRemove(path);
		if (ret < 0)
			return ret;
	}

	return 1;
}

int checkExists(const char *path) {
	SceIoStat stat;
	if(sceIoGetstat(path, &stat)<0) 
	return 0;
	return SCE_S_ISDIR(stat.st_mode);
}

int checkIfRepatched(const char *titleid, const char *dlcid) {
	char currentPath[PATH_MAX];
	if(!dlcid) 
	snprintf(currentPath, PATH_MAX, rePatchFolder"/%s", titleid);
	else
	snprintf(currentPath, PATH_MAX, addcontFolder"/%s", titleid, dlcid);
	return checkExists(currentPath);
}

int checkIfEncrypted(const char *titleid) {
	char currentPath[PATH_MAX];
	snprintf(currentPath, PATH_MAX, "%s%s", g_currentMount, titleid);
	return checkExists(currentPath);
}

int checkIfFolder(const char *titleid, const char *dlcid) {
	char currentPath[PATH_MAX];
	snprintf(currentPath, PATH_MAX, addcontFolder"/%s", titleid, dlcid);
	return checkExists(currentPath);
}

void cleanUpDLC(const char *titleid) {
	int dfd = sceIoDopen(g_currentMount);
	if(dfd < 0)
	return;
	SceIoDirent dir_stat;
	while(sceIoDread(dfd, &dir_stat)==1) {
		if(strcmp(dir_stat.d_name,"sce_sys")==0)
		continue;
		psvDebugScreenPrintf("Current DLC: %s\n", dir_stat.d_name);
		if(!checkIfDummy(titleid, dir_stat.d_name)) {
			psvDebugScreenPrintf("Not dummy file... skipping.\n");
			continue;
		}
		if(!checkIfRepatched(titleid, dir_stat.d_name)) {
			
			char currentDlc[PATH_MAX];
			snprintf(currentDlc, PATH_MAX, "%s%s", g_currentPath, dir_stat.d_name);
			psvDebugScreenPrintf("No rePatch detected... deleting: %x\n", sceIoRmdir(currentDlc));
		}		

	}
	sceIoDclose(dfd);
}

void generateDummyFolders(const char *titleid) {
	if(checkIfEncrypted(titleid))
	return;
	char currentPath[PATH_MAX];
	snprintf(currentPath, PATH_MAX, addcontFolder, titleid);
	int dfd = sceIoDopen(currentPath);
	if(dfd < 0)
	return;
	SceIoDirent dir_stat;
	char currentDlc[PATH_MAX];
	while(sceIoDread(dfd, &dir_stat)==1) {
		if(strcmp(titleid,"sce_sys")==0)
		continue;
		psvDebugScreenPrintf("Current Decrypted DLC: %s\n", dir_stat.d_name);
		snprintf(currentDlc, PATH_MAX, "%s%s", g_currentPath, dir_stat.d_name);
		if(checkExists(currentDlc))
			continue;
		if(!checkIfFolder(titleid, dir_stat.d_name))
			continue;
		psvDebugScreenPrintf("Creating dummy folder: %x\n", sceIoMkdir(currentDlc, 0777));
	}
	sceIoDclose(dfd);
}



int drawPrompt() {
	sceKernelDelayThread(1500000); 
	psvDebugScreenPrintf("\n.....................................\n");
	psvDebugScreenPrintf(" X means YES, other buttons NO\n");
	psvDebugScreenPrintf(".....................................\n");
	SceCtrlData ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	sceCtrlPeekBufferPositive(0, &ctrl, 1);
	while(ctrl.buttons == 0) {
		sceCtrlPeekBufferPositive(0, &ctrl, 1);
	}
	if((ctrl.buttons & SCE_CTRL_CROSS) == (SCE_CTRL_CROSS))
		return 1;
	return 0;
}

enum {
	DLC_HOUSEWORK,
	APP_HOUSEWORK
};

void handleGame(const char *titleid, int opt) {
	psvDebugScreenClear(0xFF000000);
	psvDebugScreenPrintf(".....................................\n");
	psvDebugScreenPrintf("Reading game: %s\n", titleid);
	psvDebugScreenPrintf(".....................................\n");
	if(opt == DLC_HOUSEWORK) {
		if(strcmp(titleid,"sce_sys")==0)
			return;
		snprintf(g_currentPath, PATH_MAX, "ux0:addcont/%s/", titleid);
		int res;
		psvDebugScreenPrintf("Mounting PFS drive: %x\n", res = pfsMount(g_currentPath));
		if(res >= 0) {
			psvDebugScreenPrintf("....................................\n");
			psvDebugScreenPrintf("Cleaning up old DLC dummy folders.\n");
			psvDebugScreenPrintf("....................................\n");
			cleanUpDLC(titleid);
			psvDebugScreenPrintf("...............................\n");
			psvDebugScreenPrintf("Generating DLC dummy folders.\n");
			psvDebugScreenPrintf("...............................\n");
			generateDummyFolders(titleid);
			psvDebugScreenPrintf("Unmounting PFS drive: %x\n", pfsUmount(g_currentMount));
		}
	} else if(opt == APP_HOUSEWORK){
		if(!checkIfEncrypted(titleid)) {
			psvDebugScreenPrintf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			psvDebugScreenPrintf("! No app folder detected... REMOVE? !\n");
			psvDebugScreenPrintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			if(drawPrompt()) {
				snprintf(g_currentPath, PATH_MAX, rePatchFolder"/%s", titleid);
				removePath(g_currentPath);
				snprintf(g_currentPath, PATH_MAX, addcontFolder, titleid);
				removePath(g_currentPath);
			}
			sceKernelDelayThread(1500000); 
			psvDebugScreenClear(0x00000000);
		}
		
	}
}

int main(int argc, char **argv) {
	psvDebugScreenInit();

	psvDebugScreenPrintf(".....................................\n");
	psvDebugScreenPrintf("rePatch Addcont Injection Deficiency \n");
	psvDebugScreenPrintf(".....................................\n");
	psvDebugScreenPrintf("Starting VitaShell Modules....\n");
	int patch_modid, kernel_modid, user_modid;
	psvDebugScreenPrintf("Starting unload patch...: %x\n", patch_modid = taiLoadStartKernelModule("ux0:VitaShell/module/patch.skprx", 0, NULL, 0));
	psvDebugScreenPrintf("Starting VitaShell Kernel Module...: %x\n", kernel_modid = taiLoadStartKernelModule("ux0:VitaShell/module/kernel.skprx", 0, NULL, 0));
	psvDebugScreenPrintf("Starting VitaShell User Module...: %x\n", user_modid = sceKernelLoadStartModule("ux0:VitaShell/module/user.suprx", 0, NULL, 0, NULL, NULL));
	
	int dfd = sceIoDopen("ux0:addcont");
	psvDebugScreenPrintf("Reading addcont: %x\n", dfd);
	if(dfd >= 0) {
		SceIoDirent dir_stat;
		while(sceIoDread(dfd, &dir_stat)==1) {
			if(checkIfRepatched(dir_stat.d_name, NULL))
				handleGame(dir_stat.d_name, DLC_HOUSEWORK);
		}
	}
	sceIoDclose(dfd);
	psvDebugScreenPrintf("Stopping VitaShell User Module...: %x\n", sceKernelStopUnloadModule(user_modid, 0, NULL, 0, NULL, NULL));
	psvDebugScreenPrintf("Stopping VitaShell Kernel Module...: %x\n", taiStopUnloadKernelModule(kernel_modid, 0, NULL, 0, NULL, NULL));
	psvDebugScreenPrintf("Stopping unload patch...: %x\n\n", 	taiStopKernelModule(patch_modid, 0, NULL, 0, NULL, NULL));

	psvDebugScreenPrintf("......................................................................\n");
	psvDebugScreenPrintf("DO YOU WANT TO CLEAN UP REPATCH FOLDER?\n");
	psvDebugScreenPrintf("This will remove rePatch directories for games that are not installed.\n");
	psvDebugScreenPrintf("You will be prompted for each detected entry.\n");
	psvDebugScreenPrintf("......................................................................\n");	
	if(drawPrompt()){
		dfd = sceIoDopen(rePatchFolder);
		strcpy(g_currentMount, "ur0:appmeta/");
		if(dfd >= 0) {
			SceIoDirent dir_stat;
			while(sceIoDread(dfd, &dir_stat)==1) 
				handleGame(dir_stat.d_name, APP_HOUSEWORK);
		}
		sceIoDclose(dfd);
		
		dfd = sceIoDopen("ux0:reAddcont");
		if(dfd >= 0) {
			SceIoDirent dir_stat;
			while(sceIoDread(dfd, &dir_stat)==1) 
				handleGame(dir_stat.d_name, APP_HOUSEWORK);
		}
		sceIoDclose(dfd);
	}
	psvDebugScreenPrintf(".................................\n");
	psvDebugScreenPrintf("Done... You may now exit the app!\n");
	psvDebugScreenPrintf(".................................\n");
	psvDebugScreenPrintf("Exiting in 5 seconds...\n");
	sceKernelDelayThread(5000000); 
	sceKernelExitProcess(0);
	return 0;
}