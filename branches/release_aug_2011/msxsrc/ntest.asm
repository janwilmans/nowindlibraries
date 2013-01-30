BDOS:   equ $0005

        output "ntest.com"

        defpage 0,$0100

        page 0

        code @ $0100
entry:
        ld de,welcomemessage
        ld c,$09
        call BDOS

        ld de,readbuffer
        ld c,$1a
        call BDOS

loop:
        ld a,r          ; 'random' startsector
        and %00000011
        ld d,a
        ld a,r
        ld e,a
        
        ld a,r          ; 'random' number of sectors
        and %00111111
        add 40
        ld h,a
        
        ld l,0          ; drive number
        ld c,$2f
        call BDOS
        
        jp loop

        
welcomemessage:
        db "Nowind Interface test running...$"

readbuffer:
        ; data will be tranfered to this address
