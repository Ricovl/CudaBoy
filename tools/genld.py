regs = ['b', 'c', 'd', 'e', 'h', 'l', 'hlp', 'a']
inst_num = 0x40

for reg1 in regs:
    for reg2 in regs:
        print(f"// 0x{inst_num:x}")
        print(f"void ld_{reg1}_{reg2}(state_t &s) {{", end='')
        if not 'hlp' in reg1 and not 'hlp' in reg2:
            print(f" s.regs.{reg1} = s.regs.{reg2}; ", end='')
        elif 'hlp' in reg1 and not 'hlp' in reg2:
            print(f' write_u8(s, s.regs.hl, (_reg8_t)s.operand); ', end='')
        elif 'hlp' in reg2 and not 'hlp' in reg1:
            print(f' s.regs.a = read_u8(s, s.regs.hl); ', end='')
        else:
            print(f' s.halt = true; ', end='')

        print(f"}}\n")
        inst_num = inst_num + 1
        