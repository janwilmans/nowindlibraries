; test file for g9klib
		OUTPUT  "g9klib.lib"
		org	0100h
		
		DEFINE  BITBUSTER_OPTIMIZE_SPEED

		include "../macros.inc"	
		include "../bdos/bdos.inc"
		include "g9klib.inc"
		
		
		; Insert your code here			
	
		include "file.asm"
		include "g9klib.asm"
		include "../string/string.asm"
		include "../bdos/bdos.asm"
		
		IFNDEF G9K_DISABLE_BITBUST
		include "../bitbuster/bitbuster.asm"
		ENDIF
		
		
		