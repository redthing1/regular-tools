
# programming with `REGULAR_ad`

## stack convention

the stack pointer is initialized to `mem_sz - 4`, reserving one cell of memory at the very top of the stack. subsequent stack operations will move the stack pointer. the stack grows downward as elements are pushed onto it.

## calling convention

`REGULAR_ad`'s calling convention is based on that of `REGULAR`, except that registers should be saved by the caller rather than the callee.
