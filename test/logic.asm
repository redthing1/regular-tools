; test more features

#entry :main

cond1:
    set r2 $00ff
    int r2 ; raise r2
    jmi ::test2 ; resume the test

main:
test1:
    ; test conditional jumping
    set r1 .10
    set r2 .10
    ; compare values
    tcu r3 r1 r2
test2:
    
    hlt
