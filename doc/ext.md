
# REGULAR_ad extension documentation

## system instructions

system instructions are extensions to the architecture specification and require hardware support.

| Name  | Encoding | Description     |
|-------|----------|-----------------|
| `hlt` | 0x70     | Halt execution. |

## pseudo instructions

pseudo instructions are extensions to the instruction set implemented by expansion to equivalent hardware instructions by the assembler.

| Name  | Encoding | Implementation      | Description                                            |
|-------|----------|---------------------|--------------------------------------------------------|
| `jmp` | 0xa0 rA  | Setting `sp` value. | Jump to address specified in rA.                       |
| `jmi` | 0xa0 imm | Setting `sp` value. | Jump to address specified in unsigned 16-bit constant. |
