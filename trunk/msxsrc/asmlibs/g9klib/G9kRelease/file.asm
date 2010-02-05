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