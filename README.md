# rePatch reDux0:

This plugin serves as a substitute for the patch folder when an existing update is present on official cartridges or other encrypted formats. It also has the benefit of loading decrypted content on 3.65.

**To clarify: this plugin works on both 3.65 and 3.60 on NoNpDrm, official cartridges, and officially downloaded games.**

Note: Be wary of using a Vitamin dumped(or similar) eboot, it will be affected by save issues. 


**More in-depth information on modding by Radziu, including newly added DLC:** https://github.com/TheRadziu/NoNpDRM-modding/wiki

3.65 Confirmed working: https://twitter.com/dots_tb/status/972922611536576512

Eboot.bin patching added based off: https://github.com/GabyPCgeeK/rePatch

# How to use (GENERAL USE):

	1. Install the plugin as any other kernel plugin, being sure to add it to the *KERNEL section of your config.txt.
	2. Create the folder ux0:rePatch
	3. Create a folder within the rePatch folder with the TITLE ID of the game you wish to patch. 
	4. Place decrypted content within this TITLE ID folder. Exclude the sce_sys folder.
	5. You may delete overlapping files within the original patch folder or app folder to save space.

**You should have 3 seperate folders:**

	1. An app folder for the original app. (Minus the ones that are being patched by rePatch)
	2. A patch folder to contain the official update/patch files. (Minus the ones that are being patched by rePatch)
	3. A rePatch folder which contains modified versions of files **that were formally contained in app and patch**

# How to use (DLC USE):	
	
	1. Within a game's rePatch folder, create a folder named "addcont". Ex: ux0:rePatch/<TITLE ID>/addcont
	2. Place DECRYPTED content within this folder relating to DLC. This may include: DLC mods (similar to normal rePatch mods) or DLC decrypted/dumped by a pre-taihen dumper or Motoharu's tools.
	3. Please continue to the following sections pertaining to the subject you want.

**NOTE: rePatch does not act as a patch directory and will not work with decrypted official updates. It modifies files that are or were in encrypted folders.**

# DLC Decrypted Only

	1. Make sure that ux0:addcont/<TITLE ID> does not exist. This will make the game load from the rePatch addcont folder instead.
	2. Place completely decrypted content in: ux0:rePatch/<TITLE ID>/addcont/<DLC ID>
	NOTE: You now may mod the DLC directly as you could with MAIDUMP or VITAMIN.
	
# DLC Encrypted Only (Modding)

	1. Make sure that ux0:addcont/<TITLE ID> does exist and that you have encrypted DLC. This includes officially downloaded DLC along with NoNpDRM DLC.
	2. Place completely decrypted content in corresponding folder: ux0:rePatch/<TITLE ID>/addcont/<DLC ID>
	3. You may delete overlapping files within the original addcont folder to save space. This works exactly like a rePatch patch.
	
# DLC Encrypted + Decrypted

	1. Make sure that ux0:addcont/<TITLE ID> does exist and that you have encrypted DLC. This includes officially downloaded DLC along with NoNpDRM DLC.
	2. Place completely decrypted content in corresponding folder: ux0:rePatch/<TITLE ID>/addcont/<DLC ID>
	3. Run the rePatch Addcont Injection Deficiency Supplement program.
	NOTE: Modding still works in this mode.

# rePatch Addcont Injection Deficiency Supplement:	

This program is for the maintenance of rePatch. It will allow the DECRYPTED + ENCRYPTED mode to work and will help delete no longer used rePatch game directories.
	
Developed by dots-tb & SilicaAndPina
