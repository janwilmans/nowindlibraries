		ifndef FILE_ASM
		define FILE_ASM
		
		
		
		MODULE File
		
FileOpen
		LD	C, _OPEN
		JP	Thread.MutexedBdosCall	
FileRead
		PUSH	BC
		LD	C,_READ
		CALL	Thread.MutexedBdosCall	
		POP	BC
		RET
FileClose
		LD	C, _CLOSE
		JP	Thread.MutexedBdosCall	

		ENDMODULE
    
		endif