			MODULE	WaveDriverRelease

Release:
			DI
			CallSegment WAVESEG,WAVE_DRV_JUMP.Stop
			CallSegment WAVESEG,WAVE_DRV_JUMP.FreeSongSegments
			
			; Copy old int
			LD	HL,0fb04h+WaveDriverLoader.old_int-WaveDriverLoader.opl4_int_han
			LD	DE,0fd9ah
			LD	BC,5
			LDIR
							
			LD	A,(WAVESEG)
			CALL	mapper.FreeSegment
			LD	BC,26	;opl4_int_end-opl4_int_han
			LD	HL,0fb04h
			LD	DE,0fb04h+1	
			LD	(HL),0
			LDIR	
			EI			
			RET

			ENDMODULE