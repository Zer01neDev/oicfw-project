	.set noreorder

#include "pspstub.s"

	STUB_START	"Plutonium",0x40090000,0x00030005
	STUB_FUNC	0x340AC1E4,Plutonium_340AC1E4
	STUB_FUNC	0xF8547F11,Plutonium_F8547F11
	STUB_FUNC	0xE30ED0F0,Plutonium_E30ED0F0
	STUB_END
