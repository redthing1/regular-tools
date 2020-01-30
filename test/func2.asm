; test function calls

#entry :main

func_add: ; function to add two numbers
    ; get arg1
    ; get arg2
    ; return (arg1 + arg2)
    ret

main:
    call func_add
    hlt ; r1 = $41, r2 = $69
