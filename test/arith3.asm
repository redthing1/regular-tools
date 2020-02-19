; test basic add function with asmv2

#entry :main

main:
    set r1 .20
    set r2 .10

    ; push to the stack in reverse order
    psh r2 ; arg2
    psh r1 ; arg1

    set r4 ::func_add
    cal r4

func_add:
    ; | RET | R1 | R2 |
    set at $4
    add ad sp at
    ldw r1 ad
    
    pop r1 ; arg1
    pop r2 ; arg2
    ; put return value

    ret
