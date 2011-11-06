10 CLS
20 SW=PEEK(&HF3B0): 'LINLIN (screen width)
30 XM=CINT((SW-13)/13)*13
40 PRINT "[Select file to start]:":LOCATE 0,4
50 ON ERROR GOTO 120
60 'list files
70 OFS=CSRLIN
80 FILES "*.bas"
90 FILES "*.ldr
100 FILES "*.prg"
110 GOTO 130
120 RESUME NEXT:' ignore errors
130 ' done listing files
140 ON ERROR GOTO:' remove handler
150 X=0:Y=OFS:YM=CSRLIN:GOSUB 250
160 'main loop
170 GOSUB 250
180 A$=INPUT$(1):A=ASC(A$)
190 IF A=28 THEN IF X<XMTHEN X=X+13
200 IF A=29 THEN IF X>0 THEN X=X-13
210 IF A=30 THEN IF Y>OFS THEN Y=Y-1
220 IF A=31 THEN IF Y<YM THEN Y=Y+1
230 IF A=13 GOTO 290
240 GOTO 160
250 ' update screen
260 'LOCATE 30,0:PRINTUSING "keycode: ###";A
270 LOCATE X,Y,1:'set position, enable cursor
280 RETURN
290 ' run program at cursor
300 'get the filename from the screen
310 'width 37 is actually width 40 with 3 unreachable chars
320 'some calculating is needed to get the right positions for all widths
330 LOCATE 0,0,0:'remove cursor
340 IF (SW>40) THEN YW=80 ELSE YW=40
350 YO=CINT((YW+1-SW)/2)
360 B$="":FOR A=0 TO 12:B$=B$+CHR$(VPEEK((Y*YW)+X+A+YO)):NEXT
370 LOCATE 0,2,0:PRINTUSING "[&]";B$
380 ON ERROR GOTO 430
390 RUN B$
400 'return from error
410 ON ERROR GOTO:'remove handler
420 GOTO 160
430 'load error
440 LOCATE 15,2:PRINT">> Load error!!":BEEP
450 RESUME 400
