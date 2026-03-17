#define LAUNCHER_VERSION "1.1.2"

#include <kernel.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libcdvd.h>
#include <debug.h>
#include <malloc.h>
#include <string.h>
#include <osd_config.h>
#include <ctype.h>  // For toupper used to convert to uppercase
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>

// elf loading
#include <iopcontrol.h>
#include <iopheap.h>
#include <sbv_patches.h>
#include <elf-loader.h>
#include <dirent.h>

#define NEWLIB_PORT_AWARE
#include <fileio.h>
#include <fileXio_rpc.h>
#include <io_common.h>

#include "cnf_lite.h"

extern unsigned char ps2dev9_irx[];
extern unsigned int size_ps2dev9_irx;

extern unsigned char ps2atad_irx[];
extern unsigned int size_ps2atad_irx;

extern unsigned char ps2hdd_irx[];
extern unsigned int size_ps2hdd_irx;

extern unsigned char iomanx_irx[];
extern unsigned int size_iomanx_irx;

extern unsigned char filexio_irx[];
extern unsigned int size_filexio_irx;

extern unsigned char ps2fs_irx[];
extern unsigned int size_ps2fs_irx;

extern unsigned char usbd_irx[];
extern unsigned int size_usbd_irx;

extern unsigned char usbmass_bd_irx[];
extern unsigned int size_usbmass_bd_irx;

enum CONSOLE_REGIONS {
    CONSOLE_REGION_INVALID = -1,
    CONSOLE_REGION_JAPAN   = 0,
    CONSOLE_REGION_USA, // USA and HK/SG.
    CONSOLE_REGION_EUROPE,
    CONSOLE_REGION_CHINA,

    CONSOLE_REGION_COUNT
};

enum DISC_REGIONS {
    DISC_REGION_INVALID = -1,
    DISC_REGION_JAPAN   = 0,
    DISC_REGION_USA,
    DISC_REGION_EUROPE,

    DISC_REGION_COUNT
};

static char ver[64], cdboot_path[256], romver[16];

static char ConsoleRegion = CONSOLE_REGION_INVALID, DiscRegion = DISC_REGION_INVALID;

// Define a constant for the maximum length of the game ID
#define MAX_GAME_ID_LENGTH 12
char game_id[MAX_GAME_ID_LENGTH];

char cwd[256];
const char *targetElf = "PS1VModeNeg.elf";
char fullFilePath[512];

static int GetDiscRegion(const char *path) {
    int region = DISC_REGION_INVALID;

    // Find the last occurrence of '/', '\\', or ':'
    const char *path_end = strrchr(path, '/');
    if (!path_end) path_end = strrchr(path, '\\');
    if (!path_end) path_end = strrchr(path, ':');

    // If we found a separator, move past it to get the filename
    if (path_end) {
        path_end++;  // Move past the separator

        // Copy the filename to `game_id` and adjust length
        strncpy(game_id, path_end, MAX_GAME_ID_LENGTH - 1);
        game_id[MAX_GAME_ID_LENGTH - 1] = '\0'; // Ensure null-termination

        // Convert `game_id` to uppercase
        for (char *p = game_id; *p; ++p) {
            *p = toupper((unsigned char)*p);
        }

        // Determine the region based on the third character of the filename
        if (strlen(game_id) >= 3) {
            switch (game_id[2]) {
                case 'P':
                    region = DISC_REGION_JAPAN;
                    break;
                case 'U':
                    region = DISC_REGION_USA;
                    break;
                case 'E':
                    region = DISC_REGION_EUROPE;
                    break;
            }
        }
    }

    return region;
}

static unsigned short int GetBootROMVersion(void)
{
    int fd;
    char VerStr[5];

    fd = open("rom0:ROMVER", O_RDONLY);
    read(fd, romver, sizeof(romver));
    close(fd);
    VerStr[0] = romver[0];
    VerStr[1] = romver[1];
    VerStr[2] = romver[2];
    VerStr[3] = romver[3];
    VerStr[4] = '\0';

    return ((unsigned short int)strtoul(VerStr, NULL, 16));
}

static int GetConsoleRegion(void)
{
    int result;

    if ((result = ConsoleRegion) < 0) {
        switch (romver[4]) {
            case 'C':
                ConsoleRegion = CONSOLE_REGION_CHINA;
                break;
            case 'E':
                ConsoleRegion = CONSOLE_REGION_EUROPE;
                break;
            case 'H':
            case 'A':
                ConsoleRegion = CONSOLE_REGION_USA;
                break;
            case 'J':
                ConsoleRegion = CONSOLE_REGION_JAPAN;
        }

        result = ConsoleRegion;
    }

    return result;
}

static int HasValidDiscInserted(int mode) {
    int DiscType, result;

    if (sceCdDiskReady(mode) == SCECdComplete) {
        switch ((DiscType = sceCdGetDiskType())) {
            case SCECdDETCT:
            case SCECdDETCTCD:
            case SCECdDETCTDVDS:
            case SCECdDETCTDVDD:
                result = 0;
                break;
            case SCECdUNKNOWN:
            case SCECdPSCD:
            case SCECdPSCDDA:
            case SCECdPS2CD:
            case SCECdPS2CDDA:
            case SCECdPS2DVD:
            case SCECdCDDA:
            case SCECdDVDV:
            case SCECdIllegalMedia:
                result = DiscType;
                break;
            default:
                result = -1;
        }
    } else {
        result = -1;
    }

    return result;
}

static void DelayIO(void) {
    int i;

    for (i = 0; i < 24; i++)
        nopdelay();
}

static void LoadModules()
{
    SifInitIopHeap();
    SifLoadFileInit();
    sbv_patch_enable_lmb();
    SifExecModuleBuffer(ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    SifExecModuleBuffer(iomanx_irx, size_iomanx_irx, 0, NULL, NULL);
    SifExecModuleBuffer(filexio_irx, size_filexio_irx, 0, NULL, NULL);
    fileXioInit();
    SifExecModuleBuffer(ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
    SifExecModuleBuffer(ps2hdd_irx, size_ps2hdd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(ps2fs_irx, size_ps2fs_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(&usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, NULL);
    SifLoadFileExit();
    SifExitIopHeap();      
}

// Function to check if PS1VModeNeg.elf exists in the current directory
static int FindElfFile() {
    DIR *d;
    struct dirent *dir;
    d = opendir(".");  // Open the current directory
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcasecmp(dir->d_name, targetElf) == 0) {
                closedir(d);
                return 0; // Success: File found
            }
        }
        closedir(d);
    }

    // Next, check the pfs0:/ directory
    d = opendir("pfs0:/");  // Open the pfs0:/ directory
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcasecmp(dir->d_name, targetElf) == 0) {
                closedir(d);
                return 0; // Success: File found in the pfs0:/ directory
            }
        }
        closedir(d);
    }

    return -1; // Failure: File not found
}

static void PrintLogo() {
    scr_clear();
    scr_printf("\n"
               "#######################              ################  #######################\n"
               "#######################              ################  #######################\n"
               "                     ##              ##                                     ##\n"
               "                     ##              ##                                     ##\n"
               "#######################              ##                #######################\n"
               "##                                   ##                ##\n"
               "##                                   ##                ##\n"
               "##                      ###############                #######################\n"
               "##                      ###############                #######################\n"
               "Enhanced PS2 Disc Launcher v%s\n\n", LAUNCHER_VERSION);
}

static unsigned char padbuf[256] __attribute__((aligned(64)));

// Waits for user to press Cross on gamepad
static void WaitForX() {
    struct padButtonStatus buttons;
    u32 paddata;

    padInit(0);
    padPortOpen(0, 0, padbuf);

    while (1) {
        if (padRead(0, 0, &buttons) != 0) {
            paddata = 0xffff ^ buttons.btns;
            if (paddata & PAD_CROSS)
                return;
        }
        DelayIO();
    }
}

static void LaunchDisc(const char *filename, int num_args, char *args[]) {
    ConfigParam config;
    GetOsdConfigParam(&config);
    int language = config.language;
    bool autolaunch = 1;
    Read_Launcher_CNF("disc-launcher.cnf", &language, &autolaunch);
    config.language = language;

    // Overrides OSD language from cnf file for import games (if disk's region is different than the console's region)
    // Falls back to console's default language if no cnf file is found or if the value is invalid.
    if (ConsoleRegion != DiscRegion) {
        SetOsdConfigParam(&config);
    }
    
    // Ask for confirmation if auto launch is disabled
    if (!autolaunch) {
        const char *region_str;
        switch (DiscRegion) {
            case DISC_REGION_JAPAN:   region_str = "JAP"; break;
            case DISC_REGION_USA:     region_str = "USA"; break;
            case DISC_REGION_EUROPE:  region_str = "EUR"; break;
            default:                  region_str = "UNK"; break;
        }

        PrintLogo();
        scr_printf("Ready to launch %s (%s)\n\n", game_id, region_str);
        scr_printf("Press X to confirm.");
        WaitForX();
    }

    LoadExecPS2(filename, num_args, args);
}

int main(int argc, char *argv[]) {
    int DiscType, done;
    SifInitRpc(0);
    fioInit();
    sceCdInit(SCECdINoD);
    init_scr();

    done = 0;
    do {
        if ((DiscType = HasValidDiscInserted(1)) >= 0) {
            if (DiscType == 0) {
                while (HasValidDiscInserted(1) == 0) {
                    DelayIO();
                }
            } else {
                switch (DiscType) {
                    case SCECdPSCD:
                    case SCECdPSCDDA:
                    case SCECdPS2CD:
                    case SCECdPS2CDDA:
                    case SCECdPS2DVD:
                        done = 1;
                        break;
                    default:
                        scr_clear();
                        PrintLogo();
                        scr_printf("Unsupported disc. Please insert a PlayStation or PlayStation 2 game disc.");
                        sceCdStop();
                        sceCdSync(0);
                        while (HasValidDiscInserted(1) > 0) {
                            DelayIO();
                        }
                }
            }
        } else {
            scr_clear();
            PrintLogo();
            scr_printf("Please insert a PlayStation or PlayStation 2 game disc.");
            while (HasValidDiscInserted(1) < 0) {
                DelayIO();
            }
        }
    } while (!done);
    scr_clear();
    sceCdDiskReady(0);
    Read_SYSTEM_CNF(cdboot_path, ver);
    DiscType = sceCdGetDiskType();
    DiscRegion = GetDiscRegion(cdboot_path);
    GetBootROMVersion();
    GetConsoleRegion();

    if ((DiscType == SCECdPSCD) || (DiscType == SCECdPSCDDA)) { // If PS1 game disc
        char *args[2] = {cdboot_path, ver};

        // If PS1 game has a different video mode to the console
        if (((DiscRegion == 2) && (ConsoleRegion != 2)) ||
            ((DiscRegion == 0 || DiscRegion == 1) && (ConsoleRegion == 2))) {

            // Initialize and configure environment
            if (argc > 1) {
                while (!SifIopReset(NULL, 0)) {}
                while (!SifIopSync()) {}
                SifInitRpc(0);
            }

            LoadModules();

            // Get the current working directory
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                LaunchDisc("rom0:PS1DRV", 2, args);
                return 1;
            }

            // Check if the current working directory is on the HDD (pfs)
            if (strstr(cwd, "pfs") != NULL) {
                char *pfs_pos = strstr(cwd, ":pfs:");
                if (pfs_pos) *pfs_pos = '\0'; // Terminate string before ":pfs:"

                if (fileXioMount("pfs0:", cwd, FIO_MT_RDONLY) != 0) {
                    LaunchDisc("rom0:PS1DRV", 2, args);
                    return 1;
                }
                strcpy(cwd, "pfs0:/");
            }

            // Find and load the ELF file
            if (FindElfFile() == 0) {
                // Ensure that cwd ends with a '/'
                size_t cwd_len = strlen(cwd);
                if (cwd[cwd_len - 1] != '/') {
                // Append '/' if it's not already there
                strncat(cwd, "/", sizeof(cwd) - cwd_len - 1);
                }

                // Construct the full file path
                snprintf(fullFilePath, sizeof(fullFilePath), "%s%s", cwd, targetElf);
                LoadELFFromFile(fullFilePath, 0, NULL);
                return 0;
            } else {
                LaunchDisc("rom0:PS1DRV", 2, args);
                return 1;
        }

        } else {
            // Run the normal PS1DRV command
            LaunchDisc("rom0:PS1DRV", 2, args);
            return 0;
        }
    } else {
        // If PS2 game disc
        LaunchDisc(cdboot_path, 0, NULL);
        return 0;
    }

    fioExit();
    sceCdInit(SCECdEXIT);
    SifExitRpc();

    return 0;
}