	.set noreorder

#include "pspstub.s"

	STUB_START	"pspDegeneration",0x40090000,0x00030005
	STUB_FUNC	0x843F274A,pspDegeneration_843F274A
	STUB_FUNC	0x9FA0B53E,pspDegeneration_9FA0B53E
	STUB_END
