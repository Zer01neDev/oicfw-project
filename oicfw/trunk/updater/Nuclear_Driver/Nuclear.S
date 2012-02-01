	.set noreorder

#include "pspstub.s"

	STUB_START "Nuclear",0x40090000,0x00010005
	STUB_FUNC  0x795F2053,NuclearStartUpdater
	STUB_END
