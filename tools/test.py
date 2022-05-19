import json
import os

__location__ = os.path.realpath(
    os.path.join(os.getcwd(), os.path.dirname(__file__)))

file = open(os.path.join(__location__, 'ops.json'))

data = json.load(file)

for op in data["CBPrefixed"]:
    operands = 0
    name = op['Name']
    if 'u8' in name or 'i8' in name:
        operands = 1
    if 'u16' in name:
        operands = 2 
    nname = name.replace('u8', '0x%02X')
    nname = name.replace('u16', '0x%04X')

    method = 'NULL'
    method = name.lower()
    method = method.replace('(', '')
    method = method.replace(')', 'p')
    method = method.replace(' ', '_')
    method = method.replace(',', '_')
    method = method.replace('u8', 'n')
    method = method.replace('i8', 'n')
    method = method.replace('u16', 'nn')
    if 'hl-' in method:
        method = method.replace('ld', 'ldd').replace('hl-', 'hl')
    if 'hl+' in method:
        method = method.replace('ld', 'ldi').replace('hl+', 'hl')
    if 'ff00' in method:
        method = method.replace('ff00+', '')
        if 'n' in method:
            method = method.replace('ld', 'ldh')
    if method == 'unused':
        method = 'NULL'
    

    print(f"    {{\"{nname}\", {op['Length']}, {op['TCyclesBranch']}, {operands}, {method}}},")