		ifndef BDOS_ASM
		define BDOS_ASM
		
		include bdos.inc
		
		MODULE Bdos

FileCreate:
; Function :   create/open file
; input    :   DE = Pointer to file name
;          :    A = File open flags
; 	   :    B = b0..b6 = Required attributes
;                       b7 = Create new flag
; output   :    A = Error Number
;          :    B = File Handle
;          :    ERROR_STRING filled with error txt
;          :    nz = error
; Modifies :    All
		BdosCall _CREATE
		JP	NZ,ExplainError
                RET
		
FileOpen:
; Function :   Open file
; input    :   DE = Pointer to file name
;          :    A = File open flags
; output   :   A = Error Number
;          :   B = File Handle
;          :   ERROR_STRING filled with error txt
;          :   nz = error
; Modifies :   All
		BdosCall _OPEN
		JP	NZ,ExplainError
                RET

FileRead:
; Function :   Read from file
; input    :   DE = Pointer to buffer
;          :   HL = Length
;          :    B = File Handle
; output   :   A = Error Number
;          :   ERROR_STRING filled with error txt
;          :   nz = Error 
;          :   HL=Bytes read
; Modifies :   AF,HL,DE
		PUSH	DE
                PUSH    BC
                BdosCall _READ
		CALL	NZ,ExplainError
                POP     BC
                POP	DE
		RET

FileClose:
; Function :   close file
;          :   B = File Handle
; output   :   A = Error Number
;          :   ERROR_STRING filled with error txt
; Modifies :   All
		BdosCall _CLOSE
		JP	NZ,ExplainError
                RET


FileWrite:
; Function :   Write to file
; input    :   DE = Pointer to buffer
;          :   HL = Number of bytes to write
;          :    B = File Handle
; output   :   A = Error Number
;          :   ERROR_STRING filled with error txt
;          :   nz = Error 
;          :   HL=Bytes read
; Modifies :   AF,HL,DE
                PUSH    BC
                BdosCall _WRITE
		CALL	NZ,ExplainError
                POP     BC
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
                        
ERROR_STRING:   	DS	64,0
    
 
SetDrivePath:
 ; Input     :  DE = Pointer to ASCIIZ string containing drive\path
 ; Output    :  none
 ; Modifies  :  AF,BC,DE,HL
    
   		PUSH	  DE
		BdosCall  _CHDIR
		POP	  HL
		LD	  A,(HL)
		SUB	  A,'A'
		LD	  E,A
		BdosCall  _SELDSK 
		RET

FindFirst:
; Input  :   DE = Drive/path/file ASCIIZ string or fileinfo block pointer
;            HL = filename ASCIIZ string (only if DE = fileinfo pointer)
;             B = Search attributes
;            IX = Pointer to new fileinfo block
; Output :    A = Error code
;        :   NZ = Error
;        : (IX) = Filled in with matching entry 
;        : ERROR_STRING, string explaining error
		BdosCall _FFIRST
		JP	NZ,ExplainError
		RET	
FindNext:	
; Input  :   IX = Pointer to fileinfo block from  previous find first function	
; Output :    A = Error code
;        :   NZ = Error
;        : (IX) = Filled in with next matching entry 
;        : ERROR_STRING, string explaining error
		BdosCall _FNEXT
		JP	NZ,ExplainError
		RET	
			
		ENDMODULE
		endif