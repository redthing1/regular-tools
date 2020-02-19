; test nested macros

#entry :main

increment@ rA : ; rA = rA + 1

::

add2@ rA : ; rA = rA + 2
    increment rA
    increment rA
::

main:
    set r1 $4
    add2 r1
    
    hlt
