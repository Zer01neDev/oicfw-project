	.set noreorder

#include "pspstub.s"

	STUB_START "sceVshBridge",0x00090000,0x00010005
	STUB_FUNC  0x2EB0812A,vshKernelGetModel
	STUB_END

