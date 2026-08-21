/* CheatDevice Remastered Loader
 * Copyright (C) 2026-, daniemun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <pspmoduleinfo.h>
#include <pspkernel.h>
#include <systemctrl.h>
#include <pspsysmem_kernel.h>
#include <string.h>
#include <kubridge.h>
#include "versioning.h"

PSP_MODULE_INFO(PLUGIN_NAME, PSP_MODULE_USER, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(0);

/* https://github.com/uofw/uofw/blob/master/include/modulemgr_options.h#L12 */
/* for SceKernelLMOption.position */
#define SCE_KERNEL_LM_POS_LOW       (PSP_SMEM_Low)  /* Place module at lowest possible address */
#define SCE_KERNEL_LM_POS_HIGH      (PSP_SMEM_High) /* Place module at highest possible address */

/* https://github.com/uofw/uofw/blob/master/include/modulemgr_options.h#L17 */
/* for SceKernelLMOption.access */
#define SCE_KERNEL_LM_ACCESS_NOSEEK (1)

/* Lowest address in which we could detect that some sort of High Memory Layout is being used */
#define LOWEST_ADDR_HIGH_MEM_LAYOUT 0x0AC00000

static int _main(SceSize args, void *argp)
{
  (void)args;

  /* Wait until GTA is loaded...
   * If in 5 tries (5 sec), 'GTA3' module wasn't found
   * most likely we aren't playing GTA so we're unloading :D */
  int tries = 0;
  SceModule gta3Module = {0};
  while( kuKernelFindModuleByName("GTA3", &gta3Module) < 0 || !gta3Module.text_addr )
  {
    if ( tries == 5 ) goto UNLOAD;
    sceKernelDelayThread(1000 * 1000); /* 1 sec */
    tries++;
  }

  const char *plugin_filename;

  /* Load lite / full version depending on memory available            */
  const SceUID high_addr_block = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 0x100, NULL);
  if ( high_addr_block < 0 ) goto UNLOAD;

  /* Highest address is not calculated precisely this way but it gives a rough idea */
  const int highest_addr = (int)sceKernelGetBlockHeadAddr(high_addr_block) + 0x100;
  sceKernelFreePartitionMemory(high_addr_block);

  /* Higher highest addr -> Load Full version */
  /* Lower highest addr -> Load Lite version  */
  if ( highest_addr >= LOWEST_ADDR_HIGH_MEM_LAYOUT ) plugin_filename = "cheatdevice_remastered.prx";
  else plugin_filename = "cheatdevice_remastered_lite.prx";

  /* Get current plugin directory from argp (this is where we'll try to find CDR) */
  char *directory = (char *)argp;
  char *slash = directory ? strrchr(directory, '/') : NULL;
  if ( !slash ) goto UNLOAD;
  slash++; *slash = '\0'; /* Cut argp string to get plugin directory */

  char full_path[PATH_MAX];
  strcpy(full_path, directory); /* Copy plugin directory */
  strcpy(full_path + (slash - directory), plugin_filename);

  /* Load module options */
  const SceKernelLMOption CDR_module_options =
  {
    .size     = sizeof(SceKernelLMOption),
    .mpidtext = PSP_MEMORY_PARTITION_USER,
    .mpiddata = PSP_MEMORY_PARTITION_USER,
    .position = SCE_KERNEL_LM_POS_HIGH,
    .access   = SCE_KERNEL_LM_ACCESS_NOSEEK,
  };

  /* Load and start CheatDevice */
  SceUID modid = kuKernelLoadModule(full_path, 0, (SceKernelLMOption *)&CDR_module_options);
  if ( modid >= 0 ) sceKernelStartModule(modid, 0, NULL, NULL, NULL);

  /* Unload this module after all work */
  UNLOAD:
  sceKernelSelfStopUnloadModule(1, 0, NULL);
  
  return 0;
}

int module_start(SceSize args, void *argp)
{
  /* Create and start main thread */
  SceUID thid = sceKernelCreateThread("CDRLoaderThread", _main, 32, 0x1000, 0, 0);
  if ( thid < 0 ) return 1;

  sceKernelStartThread(thid, args, argp);
  return 0;
}