; test file for g9klib
		OUTPUT  "g9klib.lib"
		org	0100h
		
		DEFINE  BITBUSTER_OPTIMIZE_SPEED
	
		include "bdos.inc"
		include "g9klib.inc"
		include "macros.inc"
		
		
		// Insert your code here			
	
		include "file.asm"
		include "g9klib.asm"
		include "string.asm"
		include "bdos.asm"
		
		IFNDEF G9K_DISABLE_BITBUST
		include "bitbuster.asm"
		ENDIF
		
		
		