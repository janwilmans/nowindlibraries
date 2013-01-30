		ifndef FILE_ASM
		define FILE_ASM
		
		
		
		MODULE File

FileLoad:	
; Function :   Load a file
; input    :   HL = Pointer to file name
;          :   DE = Address to load to
; output   :   A = Error Number
;          :   B = File Handle
;          :   STRING_BUFFER filled with error txt
;          :   nz=Error Loading
; Modifies :   All
		push	de		; store address to load to
		ex	de,hl		; put pointer to filename in DE
		call	FileOpen	; open the file
		pop	de		; retrieve address to load to
		ret	nz		; exit if error occurred
		LD	HL,256
	;	ld	l,(iy + Libfile.FILE_ENTRY.fileSize + 0)
	;	ld	h,(iy + Libfile.FILE_ENTRY.fileSize + 1)	;load size of file		
		call	FileRead	; read file
		ret	nz		; exit if error occurred
		
		call	FileClose	; close file
		
		ret			; done!
		
FileOpen
		JP	Bdos.FileOpen
FileRead
		JP	Bdos.FileRead
FileClose
		JP	Bdos.FileClose

		ENDMODULE
    
		endif