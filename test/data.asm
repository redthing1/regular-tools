; test some basic functions

#entry :main

data:
    #d \00000022

main:
    set r2 ::data
    ldw r1 r2 ; load the data into r1
    hlt