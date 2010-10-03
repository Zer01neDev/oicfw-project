/*
	Open Idea Custom Firmware
	Copyright (C) OpenIdea Project Team - Black Dev's Team 2010

	main.c: SystemControl Main Code
	This file are based from a reverse of M33/GEN VshControl
	Much part of this code are taken from OE/Wildcard Source Code

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspinit.h>
#include <psputilsforkernel.h>
#include <string.h>

#include "../vshctrl/main.h"
#include "sysmodpatches.h"
#include "malloc.h"
#include "umd9660_driver.h"
#include "systemctrl.h"
#include "systemctrl_se.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 4);
PSP_MAIN_THREAD_ATTR(0);

//sub_000061D8// 0x0000A2DC
//sub_000060F4// 0x0000A2E4
//sub_00003468// 0x0000A2F8

//sub_000078F4// 0x0000A2E0

//sub_000000E0// 0x0000A2F4


int (* ProbeExec1)(void *buf, u32 *check);
int (* PartitionCheck)(void *st0, void *check);
int (* ProbeExec2)(u8 *buf, u32 *check);//0x0000A2E4
//int (* sceMesgLed_driver_DFF0F308)(u8 *buf, int size, int x) = (void *)0x8801c718;
//int (* sceMesgLed_driver_55E4F665)(u8 *buf, int size, int x) = (void *)0x8801c6f0;

int sceKernelCheckExecFile(void *buf, int *check);
int sceKernelDcacheWBinvAll();
int sceKernelLoadExecutableObject(char *buf, void *check);
int sceKernelApplyPspRelSection(u32 *a0, void *a1, void *a2, void *a3, void *t0, void *t1);

int debug = 0;
u32 buf[256/4];
STMOD_HANDLER onpsprelsection = NULL;
char g_file[256];
int w;

/* ELF file header */
typedef struct
{
	u32		e_magic;
	u8		e_class;
	u8		e_data;
	u8		e_idver;
	u8		e_pad[9];
	u16		e_type; 
	u16		e_machine; 
	u32		e_version; 
	u32		e_entry; 
	u32		e_phoff; 
	u32		e_shoff; 
	u32		e_flags; 
	u16		e_ehsize; 
	u16		e_phentsize; 
	u16		e_phnum;
	u16		e_shentsize; 
	u16		e_shnum; 
	u16		e_shstrndx; 
} __attribute__((packed)) Elf32_Ehdr;

/* ELF section header */
typedef struct
{ 
	u32		sh_name; 
	u32		sh_type; 
	u32		sh_flags; 
	u32		sh_addr; 
	u32		sh_offset; 
	u32		sh_size; 
	u32		sh_link;
	u32		sh_info;
	u32		sh_addralign;
	u32		sh_entsize;
} __attribute__((packed)) Elf32_Shdr;

//0x00000000
void ClearCaches()
{
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
}

//0x0000001C
u32 sctrlHENFindFunction(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		return 0;
	}

	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	while(i < entLen)
    {
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

        if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];					
				}
			}
		}
		i += (entry->len * 4);
	}
	return 0;
}

//0x00000164 - need to confirm vs ASM
int IsAddress(void *addr)
{
	u32 u = (u32)addr;

	if (u >= 0x88000000 && u <= 0x883f0000)
		return 1;

	if (u >= 0x08840000 && u <= 0x09FFFFFFF)
		return 1;

	if (u >= 0x08800000 && u <= 0x0883FFFF)
		return 1;

	if (u >= 0x88800000 && u <= 0x8883FFFF)
		return 1;

	return 0;
}

//0x000001CC
int IsStaticElf(void *buf)
{
	Elf32_Ehdr *header = (Elf32_Ehdr *)buf;

	if (header->e_magic == 0x464C457F && header->e_type == 2)
	{
		return 1;
	}
	return 0;
}

//0x00000214
int PatchExec2(void *buf, int *check)
{
	int index = check[0x4C/4];

	if (index < 0)
	{
		index += 3;
	}

	u32 addr = (u32)(buf + index);

	if (addr >= 0x88400000 && addr <= 0x88800000)
	{
		return 0;
	}

	check[0x58/4] = ((u32 *)buf)[index/4] & 0xFFFF;
	return ((u32 *)buf)[index/4];
}

//0x00000274 Old
int PatchExec3(void *buf, int *check, int isPlain, int res)
{
	if (!isPlain)
	{
		return res;
	}

	if ((u32)check[8/4] >= 0x52)
	{
		if (check[0x20/4] == -1);
		{
			if (IsStaticElf(buf))
			{
				check[0x20/4] = 3;
			}
		}
		return res;
	}

	if (!(PatchExec2(buf, check) & 0x0000FF00))
	{
		return res;
	}

	check[0x44/4] = 1;
	return 0;
}

// 0x00000094 New
int PatchExec2(void *buf, int *check)
{
	u32 var3;
	int index = check[0x4C/4];
	int ret = 0;
	if (index < 0)
	{
		index += 3;
	}
	if ((0x00400000 < ((buf+index) + 0x77C00000)))
	{
		var3 = buf + ((index/4) << 0x2);
		check[0x58/4] = ((unsigned short *) var3)[0];
		ret = ((u32)var3)[0];
	}
	return ret;
}

//0x00001590 New
void PatchExec3(void *buf, int *check, int isPlain, int res)
{
	if (!isPlain)
	{
		return res;
	}
	else
	{
		if ((((u32)check[8/4] < 0x52) != 0)
		{
			if (!(PatchExec2(buf, check) & 0x0000FF00))
			{
				return res;
			}
			else
			{
				check[0x44/4] = 1;
				return 0;
			}
		}
		else
		{
			if (((u32 *)buf)[0] != 0x464C457F) // ELF
			{
			}
			else
			{
				if (((((unsigned char *) buf)[17] << 0x8) | ((unsigned char *) buf)[16]) != 0x2)
				{
				}
				else
				{
					check[0x20/4] = 3;
					return res;
				}
			}
		}
	}
	return 0;
}

//0x00000300
int PatchExec1(void *buf, int *check)
{
	if (((u32 *)buf)[0] != 0x464C457F) // ELF
	{
		return -1;
	}

	if (check[8/4] >= 0x120)
	{
		if (check[8/4] != 0x120 && check[8/4] != 0x141
			&& check[8/4] != 0x142 && check[8/4] != 0x143
			&& check[8/4] != 0x140)
		{
			return -1;
		}

		if (check[0x10/4] == 0)
		{
			if (check[0x44/4] != 0) 
			{
				check[0x48/4] = 1; 
				return 0; 
			} 

			return -1;
		}

		check[0x48/4] = 1;
		check[0x44/4] = 1;
		PatchExec2(buf, check);

		return 0;
	}
	else if ((u32)check[8/4] >= 0x52)
	{
		return -1;
	}

	if (check[0x44/4] != 0) 
	{ 
		check[0x48/4] = 1; 
		return 0; 
	} 

	return -2;
}

//0x00000514
int PartitionCheckPatched(u32 *st0, u32 *check)
{
	SceUID fd = (SceUID)st0[0x34/4];
	u32 pos;
	u16 attributes;

	if (fd < 0)
		return PartitionCheck(st0, check);

	pos = sceIoLseek(fd, 0, PSP_SEEK_CUR);

	if (pos < 0)
		return PartitionCheck(st0, check);

	/* rewind to beginning */
	sceIoLseek(fd, 0, PSP_SEEK_SET);
	if (sceIoRead(fd, buf, 256) < 256)
	{
		sceIoLseek(fd, pos, PSP_SEEK_SET);
		return PartitionCheck(st0, check);
	}

	/* go to module info offset */
	if (buf[0] == 0x50425000) /* PBP */
	{
		sceIoLseek(fd, buf[0x20/4], PSP_SEEK_SET);
		sceIoRead(fd, buf, 0x14);

		if (buf[0] != 0x464C457F) /* ELF */
		{
			/* Encrypted module */
			sceIoLseek(fd, pos, PSP_SEEK_SET);
			return PartitionCheck(st0, check);
		}

		sceIoLseek(fd, buf[0x20/4]+check[0x4C/4], PSP_SEEK_SET);

		if (!IsStaticElf(buf))
		{
			check[0x10/4] = buf[0x24/4]-buf[0x20/4]; // Allow psar's in decrypted pbp's
		}
	}
	else if (buf[0] == 0x464C457F) /* ELF */
	{
		sceIoLseek(fd, check[0x4C/4], PSP_SEEK_SET);
	}
	else /* encrypted module */
	{
		sceIoLseek(fd, pos, PSP_SEEK_SET);
		return PartitionCheck(st0, check);
	}

	sceIoRead(fd, &attributes, 2);

	if (IsStaticElf(buf))
	{
		check[0x44/4] = 0;
	}
	else
	{
		if (attributes & 0x1000)
		{
			check[0x44/4] = 1;
		}
		else
		{
			check[0x44/4] = 0;
		}
	}

	sceIoLseek(fd, pos, PSP_SEEK_SET);
	return PartitionCheck(st0, check);
}

char *GetStrTab(u8 *buf)
{
	Elf32_Ehdr *header = (Elf32_Ehdr *)buf;
	int i;

	if (header->e_magic != 0x464C457F)
		return NULL;

	u8 *pData = buf+header->e_shoff;

	for (i = 0; i < header->e_shnum; i++)
	{
		if (header->e_shstrndx == i)
		{
			Elf32_Shdr *section = (Elf32_Shdr *)pData;

			if (section->sh_type == 3)
				return (char *)buf+section->sh_offset;
		}
		pData += header->e_shentsize;
	}
	return NULL;
}

//0x000007B8
int ProbeExec2Patched(u8 *buf, u32 *check)
{
	int res;

	res = ProbeExec2(buf, check);

	if (((u32 *)buf)[0] != 0x464C457F) // ELF
		return res;

	if (IsStaticElf(buf))
	{
		// Fake apitype to avoid reject
		check[8/4] = 0x120;
	}

	if (check[0x4C/4] == 0)
	{
		if (IsStaticElf(buf))
		{
			char *strtab = GetStrTab(buf);

			if (strtab)
			{
				Elf32_Ehdr *header = (Elf32_Ehdr *)buf;
				int i;				

				u8 *pData = buf+header->e_shoff;

				for (i = 0; i < header->e_shnum; i++)
				{
					Elf32_Shdr *section = (Elf32_Shdr *)pData;

					if (strcmp(strtab+section->sh_name, ".rodata.sceModuleInfo") == 0)
					{
						check[0x4C/4] = section->sh_offset;
						check[0x58/4] = 0;
					}

					pData += header->e_shentsize;
				}
			}
		}
	}	

	return res;
}

// OK
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler)
{
	STMOD_HANDLER res = onpsprelsection;
	onpsprelsection = (STMOD_HANDLER)((u32)handler | 0x80000000);
	return res;
}

int unk_A2D8 = 0; //0x0000A2D8

void OnPspReloc(SceModule2 *mod)
{
	s0 = arg0 + 8;
	v0 = a0;

	s1 = Mem[arg0 + 108]; // text_addr

	if(strcmp(mod->modname, "sceIsofs_driver") == 0)
	{
		if(sceKernelInitApitype() == 0x120)
		{
			PatchIsofsDriver(mod->text_addr);
		}
	}
	else if(strcmp(mod->modname, "sceLowIO_Driver") == 0)
	{
		PatchLowIO(mod->text_addr);
		if(oe_malloc_init() < 0)
		{
			while(1) _sw(0,0);
		}
	}
	else if(strcmp(mod->modname, "sceUmdMan_driver") == 0)
	{
		PatchUmdMan(mod->text_addr);
	}
	else if(strcmp(mod->modname, "sceMediaSync") == 0)
	{
		PatchInitLoadExecAndMediaSync(mod->text_addr);
	}
	else if(strcmp(mod->modname, "sceImpose_Driver") == 0)
	{
		OnImposeLoad();
		s0 = 0x1;
		Mem[s0 + 0x0000A2D8];
		loc_0000118C();
	}
	else if(strcmp(mod->modname, "sceWlan_Driver") == 0)
	{
		PatchWlan(mod->text_addr);
	}
	else if(strcmp(mod->modname, "scePower_Service") == 0)
	{
		PatchPower(mod->text_addr);
		loc_00001188();
	}
	else if(strcmp(mod->modname, "sceVshNpSignin_Module") == 0)
	{
		_sw(0x3C041000, mod->text_addr+0x8668);
		//_sw(0x3C041000, mod->text_addr+0x9034); 5.50
		ClearCaches();
	}
	else if(strcmp(mod->modname, "npsignup_plugin_module") == 0)
	{
		_sw(0x3C041000, mod->text_addr+0x33DF8);
		// _sw(0x3C041000, mod->text_addr+0x3B7F8); 5.50
		s0 = 0x1;
		v0 = ClearCaches();
		/*v0 = _lw(Mem[0x1+0xA2D8];
		Mem[v0 + 0x0000A2D8] = v0;*/
	}

	if(unk_A2D8 == 0)
	{
		if(SysMemForKernel_599EE936() == 0x2){
			unk_A2D8 = PatchIoDrv();
		}
	}

	return 0;
}

void sctrlHENPatchSyscall(u32 funcaddr, void* newfunc)
{
	u32* vectors = NULL;
	__asm("cfc0   $v0, $12\nmove	%0, $v0\n" : "=r"(vectors));
	u32* end = vectors + 0x4000;

	for(; vectors != end; vectors++)
	{
		if(*vectors == funcaddr)
		{
			*vectors = (u32)newfunc;
		}
	}
}

//0x00001388
void UndoSuperNoPlainModuleCheckPatch()
{

}

// START sceMesgLed Patch
int unknown = 0; //0x0000A2C0
int (*sub_000000E0)(u32, u32, u32, u32*, u32, u32*, u32, u32);

int sub_00000404(u32 arg0, u32 arg1, u32 arg2, u32* pbIn, u32 arg4, u32* pbOut, u32 arg6, u32 arg7)
{
	int (*call)(u32, u32, u32, u32*, u32, u32*, u32, u32) = NULL;
	int call_return = -1;

	// Save some special values given by caller, to restore them later to the SCE function call.
	__asm__ volatile (
		"lw		$s2, 72($sp)\n"
		"lw		$s3, 76($sp)\n"
		"lw		$s4, 80($sp)\n"
		"lw		$s5, 84($sp)\n"
	);

	if(unknown == 0)
	{
try_all:
		if(pbIn == NULL)
		{
set_call:
			call = sub_000000E0;
call_func:
			__asm__ volatile (
				"sw		$s2, 0($sp)\n"
				"sw		$s3, 4($sp)\n"
				"sw		$s4, 8($sp)\n"
				"sw		$s5, 12($sp)\n"
			);
			call_return = call(arg0, arg1, arg2, pbIn, arg4, pbOut, arg6, arg7);
			if(call_return >= 0)
			{
				return call_return;
			}
			else
			{
				if(sub_00348((u8*)pbIn, arg4, arg6) < 0){
					return call_return;
				}else{
					__asm__ volatile (
						"sw		$s2, 72($sp)\n"
						"sw		$s3, 76($sp)\n"
						"sw		$s4, 80($sp)\n"
						"sw		$s5, 84($sp)\n"
					);
					return call(arg0, arg1, arg2, pbIn, arg4, pbOut, arg6, arg7);
				}
			}
		}
		else
		{
			call = sub_000000E0;
			if(!arg0)
			{
				goto call_func;
			}
			if(!pbOut)
			{
				goto set_call;
			}
			if(pbIn[76]==0x28796DAA || pbIn[76]==0x7316308C || pbIn[76]==0x3EAD0AEE)
			{
label18:
				if(pbIn[0x150] != 0x1F || pbIn[0x151] != 0x8B)
				{
					goto set_call;
				}
				else
				{
					((int*)pbOut)[0] = ((int*)pbIn)[44];
					memmove(pbIn, (u32*)((u32)pbIn+0x150), pbIn[44]);
					return 0;
				}
			}
			else
			{
				if(pbIn[76] != 0x8555ABF2)
				{
					goto set_call;
				}
				else
				{
					goto label18;
				}
			}
		}
	}
	else
	{
		__asm__ volatile (
			"sw		$s2, 0($sp)\n"
			"sw		$s3, 4($sp)\n"
			"sw		$s4, 8($sp)\n"
			"sw		$s5, 12($sp)\n"
		);
		if(call(arg0, arg1, arg2, pbIn, arg4, pbOut, arg6, arg7) >= 0)
		{
			return 0;
		}
		goto try_all;
	}
	return 0;
}

// OK
void PatchMesgLed()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceMesgLed");
	MAKE_CALL(mod->text_addr+0x1878, sub_00404);
	sub_000000E0 = (void*)mod->text_addr+0xE0;
	MAKE_CALL(mod->text_addr+0x1DAC, sub_00404);
	MAKE_CALL(mod->text_addr+0x3418, sub_00404);
	MAKE_CALL(mod->text_addr+0x37D4, sub_00404);
	MAKE_CALL(mod->text_addr+0x1E7C, sub_00404);
}
// END sceMesgLed Patch

// START sceMemlmd Patch
int (*sub_00000F10)(u8 *buf); // 0x0000A2E8
int (*MemLmdDecrypt)(u8 *dataIn, int cbFile, u8 *dataOut, int decompress); // 0x0000A2EC
int (*memlmd_FD379991)(u8 *data, void* addr); // 0x0000A2F0

int sub_00348(u8* buf)
{
	if(((u32*)buf)[0] != 0x5053507E) //~PSP
	{
		return 0;
	}
	else
	{
		int i = 0;
		u8* ptr = buf;

		for(i = 0; i <= 0x58; i++)
		{
			if(ptr[212] != 0)
			{
				if(((u32*)buf)[46] == 0)
				{
					if(((u32*)buf)[47] == 0)
					{
						return 1;
					}
					return sub_00000F10(buf);
				}
				else
				{
					return sub_00000F10(buf);
				}
			}
			else
			{
				ptr = buf + i;
			}
		}
		return 0;
	}
}

int (*unknown_func)(void*, void*, void*, void*, void*, void*, void*, void*);

void SystemCtrlForKernel_AC0E84D1(int arg1)
{
	int (*func) = unknown_func;
	unknown_func = (int ( *))arg1;
	return;
}

void MemlmdDecryptPatched(u8 *dataIn, int cbFile,  u8 *dataOut, int decompress)
{
	if (unknown_func == 0x00000000)
	{
		label6:
		if (dataIn == NULL)
		{
			label16:
			var24 = MemLmdDecrypt;

			label17:
			var27 = MemLmdDecrypt(dataIn, cbFile, dataOut, decompress);
			if (var27 >= 0)
			{
				label30:
				return 0;
			}
			else
			{
				var31 = sub_00348(dataIn);
				if (var31 < 0)
				{
					goto label30;
				}
				else
				{
					var35 = memlmd_FD379991(NULL, (void*)0xBFC00200);
					var37 = MemLmdDecrypt;
					return 0;
				}
			}
		}
		else
		{
			if (dataOut == NULL)
				goto label16;
			if (dataIn[0x130] == 0xC6BA41D3)
			{
				label13:
				if (dataIn[0x150] != 0x1F)
				{
					goto label16;
				}
				else
				{
					if (dataIn[0x151] != 0x8B)
						goto label16;
					dataOut[0] = dataIn[44];
					memmove(dataIn, (dataIn + 0x150));
					var27 = 0x00000000;
					goto label30;
				}
			}
			else
			{
				if (dataIn[0x130] != 0x55668D96)
				{
					var24 = MemLmdDecrypt;
					goto label17;
				}
				else
				{
					var12 = MemLmdDecrypt;
					goto label13;
				}
			}
		}
	}
	else
	{
		var8 = unknown_func(dataIn, cbFile, dataOut, decompress, arg5, arg6, arg7, arg8);
		var27 = 0x00000000;
		if (var8 >= 0)
			goto label30;
		goto label6;
	}
	return;
}

// OK
void PatchMemLmd()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceMemlmd");
	sub_00000F10 = (void*)mod->text_addr+0x0F10;
	MemLmdDecrypt = (void*)mod->text_addr+0x0134;
	MAKE_CALL(mod->text_addr+0x10D8, sub_00348);
	memlmd_FD379991 = (void*)mod->text_addr+0x1158;
	MAKE_CALL(mod->text_addr+0x112C, sub_00348);
	MAKE_CALL(mod->text_addr+0x0E10, MemlmdDecryptPatched);
	MAKE_CALL(mod->text_addr+0x0E74, MemlmdDecryptPatched);
}

void PatchMemLmd_02g()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceMemlmd");
	sub_00000FA8 = (void*)mod->text_addr+0x0FA8;
	MemLmdDecrypt = (void*)mod->text_addr+0x0134;
	MAKE_CALL(mod->text_addr+0x1170, sub_00348);
	memlmd_FD379991 = (void*)mod->text_addr+0x11F0;
	MAKE_CALL(mod->text_addr+0x11C4, sub_00348);
	MAKE_CALL(mod->text_addr+0x0EA8, MemlmdDecryptPatched);
	MAKE_CALL(mod->text_addr+0x0F0C, MemlmdDecryptPatched);
}

// END sceMemlmd Patch

u32 unk_modentry; //0x0000A2D4
SceUID Unk_SceUIDThread = 0;//0x0000A2D0

int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void *argp)
{
	if (thid == Unk_SceUIDCreateThread)
	{
		Unk_SceUIDThread = -1;
		if(onpsprelsection)
		{
			onpsprelsection(unk_modentry, arglen, argp, arg4, arg5, arg6, arg7, arg8);
		}
	}
	return sceKernelStartThread(thid, arglen, argp);
}

SceUID sceKernelCreateThreadPatched(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam *option)
{
	SceUID var3 = sceKernelCreateThread(name, entry, initPriority, stackSize, attr, option);
	if (var3 > 0)
	{
		if (strcmp(name, "SceModmgrStart") == 0)
		{
			Unk_SceUIDThread = var3;
			unk_modentry = LoadCoreForKernel_DD303D79(entry);//entry
		}
	}
	return var3;
}

//0x00001630
int sceKernelCheckExecFilePatched(int *buf, int *check)
{
	int res = PatchExec1(buf, check);
	if (res == 0)
	{
		return res;
	}
	else
	{
		int isPlain = ((int *) buf)[0];
		res = sceKernelCheckExecFile(buf, check);
	}
	return PatchExec3(buf, check, (((isPlain + 0xB9B3BA81) < 0x1)), res);//0x00001590
}

// OK
void PatchModuleMgr()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceModuleManager");
	MAKE_JUMP(mod->text_addr+0x8024, sceKernelCheckExecFilePatched);
	sub_000078F4 = (void*)mod->text_addr+0x78F4;
	_sw(NOP, mod->text_addr+0x760);
	_sw(0x24020000, mod->text_addr+0x7BC);
	_sw(NOP, mod->text_addr+0x2BE4);
	_sw(NOP, mod->text_addr+0x2C3C);
	_sw(0x10000009, mod->text_addr+0x2C68);
	_sw(NOP, mod->text_addr+0x2F90);
	_sw(NOP, mod->text_addr+0x2FE4);
	_sw(0x10000010, mod->text_addr+0x3010);
	MAKE_CALL(mod->text_addr+0x5F3C, PartitionCheckPatched);
	MAKE_CALL(mod->text_addr+0x62B8, PartitionCheckPatched);
	_sw(NOP, mod->text_addr+0x3F6C);
	_sw(NOP, mod->text_addr+0x3FB4);
	_sw(NOP, mod->text_addr+0x3FCC);
	MAKE_JUMP(mod->text_addr+0x817C, sceKernelCreateThreadPatched);
	MAKE_JUMP(mod->text_addr+0x81C4, sceKernelStartThreadPatched);
}

// START sceLoaderCore Patch
int ProbeExec1Patched(void *buf, u32 *check)
{
	int res;
	u16 attr;
	u16 *modinfo;
	u16 realattr;

	res = ProbeExec1(buf, check);

	if (((u32 *)buf)[0] != 0x464C457F)
		return res;

	modinfo = ((u16 *)buf) + (check[0x4C/4] / 2);

	realattr = *modinfo;
	attr = realattr & 0x1E00;

	if (attr != 0)
	{
		u16 attr2 = ((u16 *)check)[0x58/2];
		attr2 &= 0x1e00;

		if (attr2 != attr)
		{
			((u16 *)check)[0x58/2] = realattr;
		}
	}

	if (check[0x48/4] == 0)
		check[0x48/4] = 1;

	return res;
}

// 0x00006A68
void sub_06A68 (int arg1, int arg2)
{
	//Not Reversed Actualy
	ClearCaches();
	return;
}

// Patch Updated
void PatchLoadCore()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceLoaderCore");
	_sw((u32)sceKernelCheckExecFilePatched, mod->text_addr+0x84A4);// OK
	_sw(LUI, mod->text_addr+0x40DC);// OK
	MAKE_CALL(mod->text_addr+0x1628, sceKernelCheckExecFilePatched);// OK
	MAKE_CALL(mod->text_addr+0x1678, sceKernelCheckExecFilePatched);// OK
	MAKE_CALL(mod->text_addr+0x49D8, sceKernelCheckExecFilePatched);// OK
	_sw(mod->text_addr+0x8988, mod->text_addr+0x89A4);// OK
	MAKE_CALL(mod->text_addr+0x46D0, ProbeExec1Patched);// OK
	MAKE_CALL(mod->text_addr+0x486C, ProbeExec2Patched);// OK
	_sw(LUI, mod->text_addr+0x40DC);// OK
	_sw(LUI, mod->text_addr+0x40C4);// OK
	_sw(MOVE_3, mod->text_addr+0x7CD8);// OK
	_sw(NOP, mod->text_addr+0x6888);// OK
	_sw(NOP, mod->text_addr+0x688C);// OK
	_sw(NOP, mod->text_addr+0x6998);// OK
	_sw(NOP, mod->text_addr+0x699C);// OK
	MAKE_CALL(mod->text_addr+0x1298, sub_00A14);// OK
	MAKE_CALL(mod->text_addr+0x1E5C, sub_06A68);//LoadCoreForKernel_23C81B66// OK
	_sw(MOVE_4, mod->text_addr+0x1E60);// OK
	ProbeExec1 = (void*)mod->text_addr+0x61D8;// OK
	ProbeExec2 = (void*)mod->text_addr+0x60F4;// OK
	sub_00003468 = (void*)mod->text_addr+0x3468;// OK
	memlmd_2159F4BB = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0x2159F4BB);
	MAKE_CALL(mod->text_addr+0x691C, memlmd_2159F4BB);// OK
	MAKE_CALL(mod->text_addr+0x694C, memlmd_2159F4BB);// OK
	MAKE_CALL(mod->text_addr+0x69E4, memlmd_2159F4BB);// OK
	memlmd_B05E960B = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0xB05E960B);
	MAKE_CALL(mod->text_addr+0x41D0, memlmd_B05E960B);// OK
	MAKE_CALL(mod->text_addr+0x68F8, memlmd_B05E960B);// OK
}
// END sceLoaderCore Patch

// Patch Updated
void PatchInterruptMgr()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceInterruptManager");
	_sw(NOP, mod->text_addr+0x145C);// OK
	_sw(NOP, mod->text_addr+0x150C);// OK
	_sw(NOP, mod->text_addr+0x1510);// OK
}

//0x000016B4
int module_start(SceSize args, void *argp)
{
	PatchLoadCore();
	PatchModuleMgr();
	PatchMemLmd();
	PatchInterruptMgr();
	ClearCaches();
	onpsprelsection = 0x80000000 | OnPspReloc;
	sctrlSESetUmdFile((char *)0x88FB0000);// Real SetUmdFile
	sctrlSESetConfig((SEConfig *)0x88FB0050);// Real SetConfig
	sctrlSESetBootConfFileIndex(*((int *) 0x88FB00C0));
	Mem[0x0000A6F8] = Mem[0x88FB0000 | 0x00CC0000];// 88FF0000
	ClearCaches();
	return 0;
}

