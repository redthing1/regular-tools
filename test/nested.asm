; test nested macros

#entry :main

increment@ rA : ; rA = rA + 1
    set at $1
    add rA rA at
::

; macros can be nested, with caveats:
; when calling another macro, no variables may be used
; because we can't resolve a variable in another macro to one bound to this one
add2@ rA : ; rA = rA + 2
    mov ad rA
    increment ad
    increment ad
    mov rA ad
::

main:
    set r1 $4
    add2 r1
    
    hlt
