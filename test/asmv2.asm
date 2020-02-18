; test of assembly v2
; v2 REGULAR_ad asm is incompatible with v1
; it is a revision rewritten for better assembler features
; the v1 assembler was rushed and a lot of features were bolted on

#entry :main
; this time, spec compatibility is improved.
; entry point is stored in a WORD-size slot
; if the emulator does not support our header
; we simply insert a jmp

; data must appear before code
data0:
    #d \x 22000000 ; $22 in little endian
data1:
    #d \' hello ; data string support
    #d \x 00 ; null terminator

; use BIND ("@") instead of MARK (":") to define a macro
addi@ rA rB v0 ; this macro sets rA = rB + v0
    add rA rB v0

main:
    set r1 $0
    set r2 $4
    addi r1 r2 $8
    ; labels can be ahead-referenced
    ; any relative offsets will be resolved later
    cal ::func1

    ; labels can be accessed with positive offsets
    set r3 ::data1.$4 ; pointer to "hello" string

func1:
    set r7 $0
