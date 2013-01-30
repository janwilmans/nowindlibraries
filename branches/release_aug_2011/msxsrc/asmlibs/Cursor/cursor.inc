

		IFNDEF	CURSOR_INC
		DEFINE	CURSOR_INC
		
		
		
		
		STRUCT	Cursor
Width		word
Height		word
x		word
y		word
A		byte
B		byte
xOffset		word    ; X-position correction in overscan mode
yOffset		word	; Y-position correction in overscan mode
		ENDS
		
		
		
		ENDIF
		
