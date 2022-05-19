regs = ['b', 'c', 'd', 'e', 'h', 'l', 'hlp', 'a']
instructions = ['rlc', 'rrc', 'rl', 'rr', 'sla', 'sra', 'swap', 'srl', 
                'bit_0', 'bit_1', 'bit_2', 'bit_3', 'bit_4', 'bit_5', 'bit_6', 'bit_7',
                'set_0', 'set_1', 'set_2', 'set_3', 'set_4', 'set_5', 'set_6', 'set_7',
                'res_0', 'res_1', 'res_2', 'res_3', 'res_4', 'res_5', 'res_6', 'res_7',]
inst_num = 0x00

for inst in instructions:
    for reg in regs:
        print(f"// 0xcb{inst_num:x}")
        arg = f's.regs.{reg}'
        if 'hlp in reg':
            arg = 'read_u8(s, s.regs.hl)'

        print(f"void {inst}_{reg}(state_t &s) {{", end=' ')

        regstr = f's.regs.{reg}'
        if 'hlp' in reg:
            regstr = f'read_u8(s, s.regs.hl)'

        result_call = f'_{inst}_n(s, {regstr})'
        if 'bit' in inst or 'set' in inst or 'res' in inst:
            result_call = f'_{inst[0:-2]}_b_r(s, {int(inst[-1])}, {regstr})'

        if 'bit' in inst:
            print(f'{result_call};', end=' ')
        elif not 'hlp' in reg:
            print(f'{regstr} = {result_call};', end=' ')
        else:
            print(f'write_u8(s, s.regs.hl, {result_call});', end=' ')




        print(f"}}")
        inst_num = inst_num + 1
        