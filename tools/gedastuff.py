regs = ['b', 'c', 'd', 'e', 'h', 'l', 'hlp', 'a']
instructions = ['add', 'adc', 'sub', 'sbc', 'and', 'xor', 'or', 'cp']
inst_num = 0x80

for inst in instructions:
    for reg in regs:
        print(f"// 0x{inst_num:x}")
        print(f"void {inst}_a_{reg}(state_t &s) {{", end=' ')
        regstr = f's.regs.{reg}'
        if 'hlp' in reg:
            regstr = f'read_u8(s, s.regs.hl)'

        print(f'_{inst}_a_n(s, {regstr});', end=' ')

        print(f"}}\n")
        inst_num = inst_num + 1
        