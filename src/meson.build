project('regular-tools',
    'c',
    default_options: ['c_std=c18']
)

disasm_sources = ['disasm.c']
executable('regular-disasm', disasm_sources)

asm_sources = [
    'asm.c', 'asm.h',
    'util.h'
]
executable('regular-asm', asm_sources)
