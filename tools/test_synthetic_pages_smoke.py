"""Pure Python independent MIPS guest-flow verification for spread-out AOT fixture."""
from __future__ import annotations
import importlib.util
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

SOURCE = Path(__file__).with_name('synthetic_pages_smoke.py')
spec = importlib.util.spec_from_file_location('synthetic_pages_smoke', SOURCE)
fixture = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fixture)

class NonlocalAotFixture(unittest.TestCase):
    def test_elf_executable_range_and_padding(self):
        data = fixture.build_bytes()
        fixture.check(data)
        self.assertEqual(data[:4], b'\x7fELF')
        self.assertEqual(struct.unpack_from('<I', data, 24)[0], fixture.ENTRY)
        self.assertEqual(struct.unpack_from('<II', data, 52+16),
                         (len(data)-fixture.FILE_OFFSET,)*2)
        self.assertEqual(struct.unpack_from('<I', data, 52+24)[0], 5)

    def test_seven_nonlocal_compilation_regions_and_seeds(self):
        addresses = fixture.callback_addresses()
        self.assertEqual(len(addresses), 143)
        self.assertEqual(len(set(a // 0x4000 for a in addresses)), 7)
        self.assertEqual(addresses[-1], fixture.CALLBACK_BASE + 142*0x100)
        self.assertGreater(fixture.POINTER_TABLE, addresses[-1]+32)
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp)/'entries.txt'
            fixture.entries_file(output)
            self.assertEqual(output.read_text().splitlines(),
                             [f'{a:08x}' for a in addresses])

    def test_nonoverlap_multiple_ram_pages_and_code_integrity(self):
        addresses = [fixture.scratch_address(i) for i in range(fixture.COUNT)]
        self.assertEqual(len(set(a // 4096 for a in addresses)), 3)
        self.assertTrue(all(a%4==0 for a in addresses))
        data = fixture.build_bytes()
        for i in range(fixture.COUNT):
            start = fixture.FILE_OFFSET + fixture.callback_addresses()[i]-fixture.ENTRY
            self.assertEqual(struct.unpack_from('<8I',data,start), fixture.callback_words(i))
        modified = bytearray(data)
        modified[fixture.FILE_OFFSET+fixture.CALLBACK_BASE-fixture.ENTRY] ^= 1
        with self.assertRaisesRegex(ValueError, 'mismatch'):
            fixture.check(bytes(modified))

    def test_independent_mips_oracle_all_callbacks_and_ram(self):
        data = fixture.build_bytes()
        regs = [0]*32
        mem = {}
        entry = fixture.ENTRY
        payload_offset = fixture.FILE_OFFSET
        payload_size = len(data)-payload_offset
        pc = entry
        delayed = None
        visited = set()
        for step in range(6000):
            offset = pc-entry
            self.assertTrue(0<=offset<payload_size and offset%4==0, f'bad PC {pc:08x}')
            op = struct.unpack_from('<I', data, payload_offset+offset)[0]
            major, rs, rt, rd = op>>26, (op>>21)&31, (op>>16)&31, (op>>11)&31
            imm = op&0xffff
            signed = imm-(0x10000 if imm&0x8000 else 0)
            jump = None
            if pc in fixture.callback_addresses():
                visited.add(pc)
            if major==15:  # LUI
                regs[rt] = imm<<16
            elif major==13:  # ORI
                regs[rt] = regs[rs] | imm
            elif major==9:  # ADDIU
                regs[rt] = (regs[rs]+signed)&0xffffffff
            elif major==35:  # LW
                addr = (regs[rs]+signed)&0xffffffff
                if entry <= addr < entry+payload_size:
                    regs[rt]=struct.unpack_from('<I', data, payload_offset+addr-entry)[0]
                else:
                    self.assertIn(addr,mem)
                    regs[rt]=mem[addr]
            elif major==43:  # SW
                mem[(regs[rs]+signed)&0xffffffff]=regs[rt]
            elif major==4:  # BEQ
                if regs[rs]==regs[rt]:
                    jump=pc+4+signed*4
            elif major==3:  # JAL
                regs[31]=pc+8
                jump=((pc+4)&0xf0000000)|((op&0x3ffffff)<<2)
            elif major==0:
                f=op&63
                if f==0:  # nop
                    pass
                elif f==33:  # addu
                    regs[rd]=(regs[rs]+regs[rt])&0xffffffff
                elif f==9:  # jalr
                    jump=regs[rs]
                    regs[rd]=pc+8
                elif f==8:  # jr
                    jump=regs[rs]
                elif f==12:  # syscall
                    self.assertEqual(regs[3],4)
                    self.assertEqual(mem[fixture.TARGET], fixture.COUNT)
                    self.assertEqual(len(visited),fixture.COUNT)
                    for i in range(fixture.COUNT):
                        self.assertEqual(mem[fixture.scratch_address(i)],i+1)
                    return
                else:
                    self.fail(f'unexpected MIPS special function {f}')
            else:
                self.fail(f'unexpected MIPS opcode {major} at {pc:08x}')
            previous = delayed
            delayed = jump
            pc = previous if previous is not None else pc+4
            regs[0]=0
        self.fail('MIPS fixture never reached Exit syscall')

    def test_cli_generate_and_check(self):
        with tempfile.TemporaryDirectory() as temp:
            elf = Path(temp)/'fixture.elf'
            seeds = Path(temp)/'seeds.txt'
            create=subprocess.run([sys.executable,str(SOURCE),'--out',str(elf),'--entries',str(seeds)],
                                  check=True,text=True,capture_output=True)
            self.assertIn('SYNTHETIC_NONLOCAL_AOT_OK',create.stdout)
            before=elf.read_bytes()
            subprocess.run([sys.executable,str(SOURCE),'--out',str(elf),'--check'],
                           check=True,text=True,capture_output=True)
            self.assertEqual(before,elf.read_bytes())
            self.assertEqual(len(seeds.read_text().splitlines()),143)

if __name__=='__main__':
    unittest.main()
