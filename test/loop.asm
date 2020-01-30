#entry :main

main:
    nop
    set r4 $6 ; loop bound
    set r2 $0 ; loop counter
loop:
    ; loop iter
    tcu r3 r4 r2 ; r3 = SIGN[r4 - r2]
    adi r2 $1
    set r1 ::loop
    brx r1 r3 ; branch to ::loop, if r3

end:
    hlt