#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

/**
 * These functions are only available in SE-C and later, 
 * and they are not in HEN 
*/

/** enum for Fake Region **/
enum 
{
	FAKE_REGION_DISABLED = 0, //Fake région désactivé
	FAKE_REGION_JAPAN = 1, //Fake région Japon
	FAKE_REGION_AMERICA = 2, //Fake région Amérique
	FAKE_REGION_EUROPE = 3, //Fake région Europe
	FAKE_REGION_KOREA = 4, //Fake région Korea /* do not use, may cause brick on restore default settings */
	FAKE_REGION_UNK = 5, //Fake région United Kingdom
	FAKE_REGION_UNK2 = 6, //Fake région Mexique
	FAKE_REGION_AUSTRALIA = 7, //Fake région Australie
	FAKE_REGION_HONGKONG = 8, //Fake région Hongkong /* do not use, may cause brick on restore default settings */
	FAKE_REGION_TAIWAN = 9, //Fake région Taiwan /* do not use, may cause brick on restore default settings */
	FAKE_REGION_RUSSIA = 10, //Fake région Russie
	FAKE_REGION_CHINA = 11, //Fake région Chine /* do not use, may cause brick on restore default settings */
	FAKE_REGION_DEBUG_TYPE_I = 12, //Fake région Debug type 1(Dark_alex Custom région)
	FAKE_REGION_DEBUG_TYPE_II = 13, //Fake région Debug type 2(Dark_alex Custom région)
};

/** enum for UMD Modes **/
enum SEUmdModes
{
	MODE_UMD = 0,//Mode Normal UMD necessaire
	MODE_OE_LEGACY = 1,//Mode OE legacy isofs driver NO UMD
	MODE_MARCH33 = 2,//Mode March33 Driver NO UMD
	MODE_NP9660 = 3,//Mode Sony NP9660 NO UMD
};

/** enum for USB Devices **/
enum SEUsbDevices
{
	USBDEVICE_MEMORYSTICK = 0, //Périphérique USB Memory Stick
	USBDEVICE_FLASH0 = 1, //Périphérique USB flash0
	USBDEVICE_FLASH1 = 2, //Périphérique USB flash1
	USBDEVICE_FLASH2 = 3, //Périphérique USB flash2
	USBDEVICE_FLASH3 = 4, //Périphérique USB flash3
	USBDEVICE_UMD9660 = 5, //Périphérique USB UMD
};

/** enum for SpeedUpMS **/
enum SESpeedUpMS
{
	SPEED_UP_NEVER = 0, //Ne jamais utiliser le hack du driver MS
	SPEED_UP_VSH = 1, //Utiliser le hack du driver MS uniquement pour le VSH (XMB)
	SPEED_UP_GAME = 2, //Utiliser le hack du driver MS uniquement pour les JEUX
	SPEED_UP_VSH_AND_GAME = 3, //Utiliser le hack du driver MS pour le VSH (XMB) et les JEUX
	SPEED_UP_POPS = 4, //Utiliser le hack du driver MS uniquement pour les POPS (JEUX PS1)
	SPEED_UP_VSH_AND_POPS = 5, //Utiliser le hack du driver MS pour le VSH (XMB) et les POPS (JEUX PS1)
	SPEED_UP_GAME_AND_POPS = 6, //Utiliser le hack du driver MS pour les JEUX et les POPS (JEUX PS1)
	SPEED_UP_ALWAYS = 7, //Toujours utiliser le hack du driver MS
};

/** SEConfig structure **/
typedef struct
{
	int magic; /* 0x47434553 */
	int hidecorrupt; //Cacher les icones corrompus
	int	skiplogo; //Sauter le logo Sony Computer Entertainment
	int umdactivatedplaincheck; //Activer l’utilisation de tous les modules pour les jeux UMD/ISO
	int gamekernel150; //Dossier GAME
	int executebootbin; //Exécuter le fichier boot.bin (eboot.bin non crypter) dans les jeux UMD/ISO
	int startupprog; //Lancement automatique d'un homebrew ce situent a ms0:/PSP/GAME/BOOT/EBOOT.PBP au démarrage de la console
	int umdmode; //Mode UMD
	int useisofsonumdinserted; //Utiliser le driver isofs si un UMD est insérer Non disponible dans les nouveau Custom Firmware
	int	vshcpuspeed; //Vitesse du CPU dans le XMB
	int	vshbusspeed; //Vitesse du BUS dans le XMB
	int	umdisocpuspeed; //Vitesse du CPU dans les jeux UMD/ISO
	int	umdisobusspeed; //Vitesse du CPU dans les jeux UMD/ISO
	int fakeregion; //Fausse région
	int freeumdregion; //Région UMD libre (non existante) Plus disponible dans les nouveau Custom Firmware
	int	hardresetHB; //Reset brutal dans les homebrew Plus disponible dans les nouveau Custom Firmware
	int usbdevice; //Périphérique USB par default
	int novshmenu; //Utilisation du vshmenu
	int usbcharge; //Charge USB si le cable est connecter (uniquement sur PSP Slim)
	int notusedaxupd; //Mise a jour reseaux M33
	int hidepics; //Cacher les PIC0/PIC1.PNG des jeux, homebrew dans le menu Jeu->Memory Stick
	int xmbplugins; //Plugins pour le XMB Non disponible pour le moment
	int gameplugins; //Plugins pour les jeux, homebrew Non disponible pour le moment
	int popsplugins; //Plugins pour les pops (jeux PS1) Non disponible pour le moment
	int useversiontxt; //Utilisation du fichier flash0:/vsh/etc/version.txt pour contourner le system de verification du PSStore de Sony
	int speedupmsaccess; //Hack MS Driver
	int reserved[2]; // ???
} SEConfig;

enum VshMenu
{
	VSHMENU_VSHMENU = 0,
	VSHMENU_RECOVERY = 1,
	VSHMENU_DISABLED = 2,
};

//structure de configuration du recovery du GEN
typedef struct
{
	int magic; /* 0x47434553 */
	int hidecorrupt;//Good
	int	skiplogo;//Good
	int umdactivatedplaincheck;//Good
	int gamekernel150;//Good
	int executebootbin;//Good
	int startupprog;//Good
	int umdmode;//Good
	int useisofsonumdinserted;
	int	vshcpuspeed;//Good
	int	vshbusspeed;//Good
	int	umdisocpuspeed;//Good
	int	umdisobusspeed;//Good
	int fakeregion;//Good
	int freeumdregion;
	int	hardresetHB;
	int usbdevice;//Good
	int vshmenu;//Not Good
	int usbcharge;
	int notusedaxupd;//Good
	int hidepics;//Good
	int xmbplugins;
	int gameplugins;
	int popsplugins;
	int useversiontxt;//Not Good
	int hidemac;
	int slimcolors;
	int gamecategorie;
	int reserved[2];
} SEGENConfig;

/**
 * Gets the SE/OE version
 *
 * @returns the SE version
 *
 * 3.03 OE-A: 0x00000500
*/
int sctrlSEGetVersion();

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @returns 0 on success
*/
int sctrlSEGetConfig(SEConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(SEConfig *config, int size);

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(SEConfig *config);

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(SEConfig *config, int size);

/**
 * Initiates the emulation of a disc from an ISO9660/CSO file.
 *
 * @param file - The path of the 
 * @param noumd - Wether use noumd or not
 * @param isofs - Wether use the custom SE isofs driver or not
 * 
 * @returns 0 on success
 *
 * @Note - When setting noumd to 1, isofs should also be set to 1,
 * otherwise the umd would be still required.
 *
 * @Note 2 - The function doesn't check if the file is valid or even if it exists
 * and it may return success on those cases
 *
 * @Note 3 - This function is not available in SE for devhook
 * @Example:
 *
 * SEConfig config;
 *
 * sctrlSEGetConfig(&config);
 *
 * if (config.usenoumd)
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * }
 * else
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
 * }
*/
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);

/**
 * Umounts an iso.
 *
 * @returns 0 on success
*/
int sctrlSEUmountUmd(void);

/**
 * Forces the umd disc out state
 *
 * @param out - non-zero for disc out, 0 otherwise
 *
*/
void sctrlSESetDiscOut(int out);

/**
 * Sets the disctype.
 *
 * @param type - the disctype (0x10=game, 0x20=video, 0x40=audio)
*/
void sctrlSESetDiscType(int type);

/**
 * Gets the current umd file (kernel only)
*/
char *sctrlSEGetUmdFile();

/**
 * Sets the current umd file (kernel only)
 *
 * @param file - The umd file
*/
void sctrlSESetUmdFile(char *file);

/** 
 * Sets the boot config file for next reboot (kernel only)
 *
 * @param index - The index identifying the file (0 -> normal bootconf, 1 -> march33 driver bootconf, 2 -> np9660 bootcnf)
*/
void sctrlSESetBootConfFileIndex(int index);

#endif
