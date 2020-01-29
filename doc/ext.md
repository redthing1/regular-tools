
# REGULAR_ad extension documentation

## system instructions

system instructions are extensions to the architecture specification and require hardware support.

| Name  | Encoding   | Description                        |
|-------|------------|------------------------------------|
| `hlt` | 0x70       | Halt execution.                    |
| `int` | 0x71 rA    | Interrupt with the value in rA.    |
| `brx` | 0x72 rA rB | Branch to address in rB if rA > 0. |

## pseudo instructions

pseudo instructions are extensions to the instruction set implemented by expansion to equivalent hardware instructions by the assembler.

| Name  | Encoding   | Implementation             | Description                                            |
|-------|------------|----------------------------|--------------------------------------------------------|
| `jmp` | 0xa0 rA    | Copy register to `sp`.     | Jump to address specified in rA.                       |
| `jmi` | 0xa1 imm   | Setting `sp` value.        | Jump to address specified in unsigned 16-bit constant. |
| `swp` | 0xb0 rA rB | Use `at` to swap registers | Swap the values in rA and rB.                          |
