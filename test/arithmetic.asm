; test more features

#entry :main

test1:
    hlt

main:
    set r1 .10
    set r2 .10
    add r1 r1 r2
    jmi ::test1
