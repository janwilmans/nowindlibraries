		ifndef FILE_ASM
		define FILE_ASM
		
		
		MODULE File

		define	MAXFILES	50

; Function: Open	
; Purpose: Opens a library file
; Parameters: DE: pointer to filename (ASCIIZ)
; Result: A: error code
;	  IX: pointer to LIBFILE structure to use
; Modifies: all
InitLibrary:	ld	ix,Library
		ld	hl,Directory		
		call	Libfile.Open
		ret
		

KillLibrary:	ld	ix,Library
		call	Libfile.Close
		ret


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
		
		ld	l,(iy + Libfile.FILE_ENTRY.fileSize + 0)
		ld	h,(iy + Libfile.FILE_ENTRY.fileSize + 1)	;load size of file		
		call	FileRead	; read file
		ret	nz		; exit if error occurred
		
		call	FileClose	; close file
		
		ret			; done!

	
FileOpen:
; Function :   Open file
; input    :   DE = Pointer to file name
;          :    A = 
; output   :   A = Error Number
;          :   B = File Handle
;          :   STRING_BUFFER filled with error txt
;          :   nz=Error Loading
; Modifies :   All
		push	ix
		ld	ix,Library
		call	Libfile.Search
		pop	ix
		JP	NZ,ExplainError
                RET

FileRead:
; Function :   Read from file
; input    :   DE = Pointer to buffer
;          :   HL = Length
;          :    B = File Handle (not used in libfile, but necessary for loading separate files)
; output   :   A = Error Number
;          :   STRING_BUFFER filled with error txt
;          :   nz=Error Loading
;          :   HL=Bytes read
; Modifies :   AF,HL,DE
                PUSH    BC
                PUSH	IX
                ld	ix,Library
                call	Libfile.Read
		CALL	NZ,ExplainError
		POP	IX
                POP     BC
		RET

FileClose:
; Function :   close file
;          :   B = File Handle
; output   :   A = Error Number
;          :   STRING_BUFFER filled with error txt
; Modifies :   All
                RET

ExplainError:
; input     :  A = error number
; output    :  ERROR_STRING filled with error txt
; Modifies  :  DE,C
		PUSH	AF
                PUSH    HL
                PUSH    BC
		LD	B,A
                LD      DE,ERROR_STRING	       
                BdosCall _EXPLAIN
                POP     BC
                POP     HL
                POP     AF
                RET
                        
ERROR_STRING   	DS	64,0

Library:	ds	Libfile.LIBFILE
Directory:	ds	MAXFILES * Libfile.FILE_ENTRY
    
    		ENDMODULE
    
		endif
