; flash01.bin is the bload'able version of the flashWriter
; it can be loaded from a real disk in case the firmware is somehow flashed incorrectly

		output "flash01.bin"
        include "labels.asm"
	
		db $fe
		dw $c000							; begin address code
		dw $c000+flash_end-flash_begin-1	; end address code
		dw flash_init						; start address

		org $c000	
flash_begin:	
		include "flashWriter.asm"

flash_init:	
		di
		in a,($a8)
		and $f0
		or  %00000101		; enable slot 1 in page 0/1
		out ($a8),a
		jp waitForFlashCommand
flash_end: