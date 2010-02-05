			ifndef Window_INC
			define Window_INC
		
LEFT_UP_COLOR		EQU	18* G9K_RED + 20 * G9K_GREEN + 23 * G9K_BLUE
RIGHT_DOWN_COLOR	EQU	10* G9K_RED + 20 * G9K_GREEN + 23 * G9K_BLUE ; 0x0808
INNER_COLOR		EQU	3 * G9K_RED + 11 * G9K_GREEN  + 14 * G9K_BLUE ;0x0404


                        STRUCT  WINDOW
left:                   WORD  
right:                  WORD
top:                    WORD
bottom:                 WORD
onMouseClick:           WORD            
onMouseOver:            WORD            
onMouseOut:             WORD
onMousePressed		WORD
onMouseReleased		WORD
data:                   WORD            ; User data			
                        ENDS


			endif
		
		
