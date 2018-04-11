/* 
rePatch 1.1 reDux0 -- PATCHING WITH FREEDOM
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

#define rePatchFolder "ux0:/rePatch"
#define addcontFolder rePatchFolder"/%s/addcont"

//https://wiki.henkaku.xyz/vita/SceIofilemgr
typedef struct io_scheduler_item //size is 0x14 - allocated from SceIoScheduler heap
{
   uint32_t* unk_0; // parent
   uint32_t unk_4; // 0
   uint32_t unk_8; // 0
   uint32_t unk_C; // 0
   uint32_t unk_10; // pointer to unknown module data section
} io_scheduler_item;

typedef struct drm_opts {
	uint32_t size;
	char adcont_id[20];
	char mount_point[10];
} drm_opts;

typedef struct mount_point_overlay{
  uint32_t unk0;
  uint32_t unk1;
  uint32_t PID;
  uint32_t mountId;
  char path[292];
  char mount_point[16];
} mount_point_overlay;

//File resolver options
#define TRUNC_PATH 0x01
#define TITLE_PATH 0x02
#define DLC_PATH   0x04
#define NO_DEVICE  0x08
#define NO_DUMBY   0x10
#define IS_DIR     0x20

#define ADDCONT_THRES 12