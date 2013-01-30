; This file specifies where it can find the Open,Read and close functions.
; Replace these jumps to support an other file system.  

		ifndef FILE_ASM
		define FILE_ASM
		
		
		
		MODULE File
		
FileOpen
		JP	Bdos.FileOpen
FileRead
		JP	Bdos.FileRead
FileClose
		JP	Bdos.FileClose

		ENDMODULE
    
		endif