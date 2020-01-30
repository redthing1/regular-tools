
# assembler documentation

```asm
#entry :main ; specify the entry point

test1: ; labels are used as jump points
    nop
    mov r2 r1

main:
    nop
    ; test some basic math
    set r1 $10 ; $ denotes a hex constant
    set r2 .10 ; . denotes a dec constant
    add r1 r1 r2
    ; test jump
    jmp ::test1 ; double colon is used to reference label
```
