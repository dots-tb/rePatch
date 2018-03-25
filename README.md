# rePatch reDux0:

This plugin serves as a substitute for the patch folder when an existing update is present on official cartridges or other encrypted formats. It also has the benefit of loading decrypted content on 3.65.

**To clarify: this plugin works on both 3.65 and 3.60 on NoNpDrm, official cartridges, and officially downloaded games.**

Note: Be wary of using a Vitamin dumped(or similar) eboot, it will be effected by save issues. 

More indepth information on modding by Radziu: https://github.com/TheRadziu/NoNpDRM-modding/wiki

3.65 Confirmed working: https://twitter.com/dots_tb/status/972922611536576512

Eboot.bin patching added based off: https://github.com/GabyPCgeeK/rePatch

# How to use:

	1. Install the plugin as any other kernel plugin, being sure to add it to the *KERNEL section of your config.txt.
	2. Create the folder ux0:rePatch
	3. Create a folder within the rePatch folder with the TITLE ID of the game you wish to patch. 
	4. Place decrypted content within this TITLE ID folder. Exclude the sce_sys folder.
	5. You may delete overlapping files within the original patch folder or app folder to save space.


Developed by dots-tb & SilicaAndPina
