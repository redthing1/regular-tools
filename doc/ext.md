
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

| Name  | Encoding    | Implementation                | Description                                            |
|-------|-------------|-------------------------------|--------------------------------------------------------|
| `jmp` | 0xa0 rA     | Copy register to `sp`.        | Jump to address specified in rA.                       |
| `jmi` | 0xa1 imm    | Set `sp` value.               | Jump to address specified in unsigned 16-bit constant. |
| `psh` | 0xa2 rA     | Stack manipulation with `at`. | Save a register to the stack.                          |
| `pop` | 0xa3 rA     | Stack manipulation with `at`. | Load a register from the stack.                        |
| `cal` | 0xa4 rA     | Push return address and jump. | Call a subprocedure.                                   |
| `ret` | 0xa5 imm    | Pop return address and jump.  | Return from a subprocedure.                            |
| `swp` | 0xb0 rA rB  | Use `at` to swap registers.   | Swap the values in rA and rB.                          |
| `adi` | 0xb1 rA imm | Expand expression with `at`   | Unsigned constant addition to rA.                      |
| `sbi` | 0xb2 rA imm | Expand expression with `at`.  | Unsigned constant subtraction from rA.                 |
