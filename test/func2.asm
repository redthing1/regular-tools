; test function calls

#entry :main

func_add: ; function to add two numbers
    ; get arg1
    ; get arg2
    ; return (arg1 + arg2)
    int r7
    ret

main:
    set r4 ::func_add
    set r7 $4
    int r7
    cal r4
    int r7
    hlt ; r1 = $41, r2 = $69
