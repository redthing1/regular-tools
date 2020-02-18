project('regular-tools',
    'c',
    default_options: ['c_std=c18']
)

disasm_sources = [
    'disasm.c', 'disasm.h',
    'asm.h',
    'instr.h',
    'util.h', 'buffie.h'
]
executable('regular-disasm', disasm_sources)

asm_sources = [
    'asm.c', 'asm.h',
    'lex.h',
    'asm_ext.h',
    'instr.h',
    'util.h', 'buffie.h'
]
executable('regular-asm', asm_sources)

emu_sources = [
    'emu.c', 'emu.h',
    'instr.h',
    'disasm.h',
    'util.h', 'buffie.h'
]
executable('regular-emu', emu_sources)
