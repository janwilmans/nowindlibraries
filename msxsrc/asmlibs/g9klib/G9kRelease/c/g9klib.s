
_Reset::

; Function :    Resets v9990, Deletes Palette, Sprites off,stops current blit operation, puts V9990 in correct RAM config and disables display
; Input    :    None
; Output   :    None
; Notes    :    Doesn't change current adjust
; Modifies : 	A,B

                G9kReadReg G9K_DISPLAY_ADJUST + G9K_DIS_INC_READ
                PUSH    AF      ; Save adjust value

                ; Set reset state
                LD      A,G9K_SYS_CTRL_SRS
                OUT     (G9K_SYS_CTRL),A
                ; Clear reset state
                XOR     A,A
                OUT     (G9K_SYS_CTRL),A

                POP     AF
                OUT     (G9K_REG_DATA),A        	; Restore adjust value

                G9kWriteReg G9K_OPCODE,G9K_OPCODE_STOP
                G9kWriteReg G9K_CTRL,G9K_CTRL_DIS_SPD+G9K_CTRL_VRAM512

                ; Clear current palette
                G9kWriteReg G9K_PALETTE_PTR,0       ; A becomes 0
                LD      B,192
                      OUT     (G9K_PALETTE),A
                      DJNZ    $-2
                OUT (G9K_OUTPUT_CTRL),A    	   ; Set output GFX9000
                RET