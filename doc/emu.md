
# emulator documentation

## options

`--step` will pause after each instruction and prompt for commands in the `dbg>` shell
`--nodbg` will disable debug mode.

## dbg commands

`s` - continue execution
`cpu` - raise the DUMPCPU interrupt
`mem` - raise the DUMPMEM interrupt
`stk` - raise the DUMPSTK interrupt
