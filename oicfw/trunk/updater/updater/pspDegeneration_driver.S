	.set noreorder

#include "pspstub.s"

	STUB_START	"pspDegeneration_driver",0x00010000,0x00020005
	STUB_FUNC	0x843F274A,pspDegeneration_driver_843F274A
	STUB_FUNC	0x9FA0B53E,pspDegeneration_driver_9FA0B53E
	STUB_END
