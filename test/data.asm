; test some basic functions

#entry :main

data:
    #d \ffddaa22

main:
    set r2 ::data
    ldw r1 r2 ; load the data into r1
    hlt