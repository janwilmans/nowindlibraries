; Product      : Libfile library
; Version      : 0.1 
; Main code by : Arjan

;----------------------------------------------------------------------------;
; General Functions overview                                                 ;
; ---------------------------------------------------------------------------;
; Open		- Opens a libfile
; Close		- Closes a libfile
; Search	- Search a file in a libfile and sets filepointer accordingly
; Read		- Read data from a libfile

		ifndef LIBFILE_ASM
		define LIBFILE_ASM

			
		MODULE	Libfile
		
		
		STRUCT	FILE_ENTRY
fileName:	BLOCK	13		; filename, 8.3 zero terminated format (no padding needed)
fileSize:	BLOCK	3		; filesize, 16MB should be more than enough :P
fileOffset:	BLOCK	3		; offset in libfile, 16MB should be more than enough :P
		ENDS
		
		STRUCT	LIBFILE
fileCount:	WORD		; number of files in library
fileHandle:	BYTE		; filehandle associated with this library
directoryAddress:	WORD            ; address of library directory
		ENDS

; Function: Open	
; Purpose: Opens a library file
; Parameters: IX: pointer to LIBFILE structure to use
;	      DE: pointer to filename (ASCIIZ)
;	      HL: address to save library directory to
; Result: A: error code
;	  IX: pointer to LIBFILE structure to use
; Modifies: all
Open:		push	ix		; store pointer to LIBFILE structure

		ld	(ix + LIBFILE.directoryAddress + 0),l
		ld	(ix + LIBFILE.directoryAddress + 1),h	; store directory addres
				
		ld	a,1                    
		call	Bdos.FileOpen	; open file for reading only		                    
                                                      		
		pop	ix              ; get pointer to LIBFILE structure
		ret	nz		; exit if error occurred
		                                                          		
		ld	(ix + LIBFILE.fileHandle),b
		
		push	ix              
		
		push	ix
		pop	de		; put pointer to LIBFILE structure in DE
		
		ld	hl,2
		call	Bdos.FileRead	; read FileCount

		pop	ix              ; get pointer to LIBFILE structure
		ret	nz		; exit if error occurred
		
		ld	e,(ix + LIBFILE.fileCount + 0)
		ld	d,(ix + LIBFILE.fileCount + 1)	; put number of files in HL
		
		ld	a,FILE_ENTRY		; file entry size (change in the future	
		call	Math.MultiplyDEA     	; calculate directory length
		
		ld	e,(ix + LIBFILE.directoryAddress + 0)
		ld	d,(ix + LIBFILE.directoryAddress + 1)   ; get directory address
		
		push	ix		; store pointer to LIBFILE structure
		
		call	Bdos.FileRead	; read directory 
		
		pop	ix		; restore pointer to LIBFILE structure 		
		ret                     ; done!


; Function: Close	
; Purpose: Closes a library file
; Parameters: IX: pointer to LIBFILE structure to use
; Result: A: error code
; Modifies: all
Close:		ld	b,(ix + LIBFILE.fileHandle)	; get file handle of file to close
		call	Bdos.FileClose	; close library file
		ret                     ; done!



; Function: Search	
; Purpose: Searches a file in the library
; Parameters: IX: pointer to LIBFILE structure to use
;	      DE: pointer to ASCIIZ filename to find
; Result: A: error code
;	  IY: address of matching FILE_ENTRY 
; Modifies: all
Search:		ld	c,(ix + LIBFILE.fileCount + 0)
		ld	b,(ix + LIBFILE.fileCount + 1)	; get number of files in libfile

		ld	l,(ix + LIBFILE.directoryAddress + 0)   
		ld	h,(ix + LIBFILE.directoryAddress + 1)	; get address of directory

.loop:		push	de		; store address filename on stack
		push	hl              ; store current file entry adddress on stack
                
        	call	String.StringCompareZ   ; compare filenames
        	jr	z,.found        ; zero flag set if filenames are the same

		pop	hl              ; retrieve current file entry address
		ld	de,FILE_ENTRY
		add	hl,de           ; move to next file entry
		pop	de              ; retrieve filename to find

		dec	bc              ; decrease number of files to find
		ld	a,b
		or	c
		jr	nz,.loop        ; loop until last file in directory checked
		
		ld	a,ERROR.NOFIL	; error code of file not found
		OR	A,A
		ret


.found:		pop	iy		; retrieve current file entry address
		pop	de              ; store address filename on stack
		
		ld	l,(iy + FILE_ENTRY.fileOffset + 0)                       
		ld	h,(iy + FILE_ENTRY.fileOffset + 1)
		ld	e,(iy + FILE_ENTRY.fileOffset + 2)   
		ld	d,0		; retrieve file offset (24 bits)                              	
		
		push	iy              ; store address file entry on stack
		
		ld	b,(ix + LIBFILE.fileHandle)	; get file handle of library 
		ld	c,_SEEK         ; seek function number
		ld	a,0             ; seek from beginning
		call	BDOS
		
		pop	iy              ; retrieve address file entry from stack
		ret                     ; done!
		
		
		
; Function: Read
; Purpose: Read data from a libfile
; Parameters: IX: pointer to LIBFILE
;	      DE: buffer address
;     	      HL: number of bytes to read
; Result: A: error code
;	  IX: pointer to LIBRARY
; Modifies: all 
Read:		push	ix
		ld	b,(ix + LIBFILE.fileHandle)
		call	Bdos.FileRead
		pop	ix
		ret

				
		ENDMODULE
		
		endif	;ifndef libfile
		
