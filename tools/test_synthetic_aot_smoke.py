"""Pure offline tests for the game-free EE compilation and execution fixture."""
from __future__ import annotations
import importlib.util
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

MODULE_PATH = Path(__file__).with_name('synthetic_aot_smoke.py')
spec = importlib.util.spec_from_file_location('synthetic_aot_smoke', MODULE_PATH)
smoke = importlib.util.module_from_spec(spec)
spec.loader.exec_module(smoke)


class AotFixture(unittest.TestCase):
    def test_elf32_structure_and_retail_free_payload(self):
        data = smoke.build_bytes()
        smoke.inspect(data)
        self.assertEqual(len(data), 364)
        self.assertEqual(data[:7], b'\x7fELF\x01\x01\x01')
        self.assertEqual(struct.unpack_from('<I', data, 24)[0], 0x100100)

    def test_instruction_sequence_and_expected_memory_write(self):
        self.assertEqual(smoke.WORDS[:4], (0x3C080001, 0x35082000, 0x2404000A, 0x0C040050))
        self.assertEqual(smoke.WORDS[5], 0xAD020000)
        self.assertEqual(smoke.WRITE_ADDRESS, 0x12000)
        self.assertEqual(smoke.WRITE_VALUE, 0x1234)
        self.assertEqual(smoke.WORDS[6:8], (0x24030004, 0x0000000C))
        self.assertEqual(smoke.WORDS[19], 0x0320F809)
        self.assertEqual(smoke.WORDS[24], 0x2482122A)

    def test_dynamic_jalr_target_has_explicit_entry_seed(self):
        with tempfile.TemporaryDirectory() as tmp:
            entry = Path(tmp) / 'entries.txt'
            smoke.write_entry_list(entry)
            self.assertEqual(entry.read_text(), '00100160\n')
            self.assertEqual(smoke.WORDS[19], 0x0320F809)
            self.assertEqual(smoke.CALLBACK, 0x00100160)
            self.assertEqual(smoke.WORDS[24], 0x2482122A)

    def test_guest_control_flow_with_delay_slots(self):
        """Independent instruction oracle; actual runtime exercised in Windows CI."""
        regs = [0] * 32
        memory = {}
        pc = smoke.ENTRY
        delayed = None
        traversed = []
        for _ in range(40):
            index = (pc - smoke.ENTRY) // 4
            self.assertTrue(0 <= index < len(smoke.WORDS), f'bad target {pc:08x}')
            op = smoke.WORDS[index]
            traversed.append(pc)
            major = op >> 26
            rs = (op >> 21) & 31
            rt = (op >> 16) & 31
            rd = (op >> 11) & 31
            imm = op & 0xffff
            signed = imm if imm < 0x8000 else imm - 0x10000
            next_jump = None
            if major == 15:  # LUI
                regs[rt] = imm << 16
            elif major == 13:  # ORI
                regs[rt] = regs[rs] | imm
            elif major == 9:  # ADDIU
                regs[rt] = (regs[rs] + signed) & 0xffffffff
            elif major == 43:  # SW
                memory[(regs[rs] + signed) & 0xffffffff] = regs[rt]
            elif major == 3:  # JAL
                regs[31] = pc + 8
                next_jump = ((pc+4) & 0xf0000000) | ((op & 0x3ffffff) << 2)
            elif major == 0:
                func = op & 63
                if func == 0:  # NOP
                    pass
                elif func == 33:  # ADDU
                    regs[rd] = (regs[rs] + regs[rt]) & 0xffffffff
                elif func == 9:  # JALR
                    regs[rd] = pc + 8
                    next_jump = regs[rs]
                elif func == 8:  # JR
                    next_jump = regs[rs]
                elif func == 12:  # SYSCALL
                    self.assertEqual(regs[3], 4)  # real Exit kernel number
                    self.assertEqual(memory, {0x12000: 0x1234})
                    self.assertIn(smoke.ENTRY + 0x4c, traversed)  # JALR
                    self.assertIn(smoke.ENTRY + 0x60, traversed)  # dynamic target
                    return
                else:
                    self.fail(f'Unexpected R-format function {func}')
            else:
                self.fail(f'Unexpected opcode {major}')
            old = delayed
            delayed = next_jump
            pc = old if old is not None else pc + 4
            regs[0] = 0
        self.fail('Synthetic guest never executed exit syscall')

    def test_mutated_opcodes_fail(self):
        b = bytearray(smoke.build_bytes())
        b[smoke.TEXT_OFFSET + 8] ^= 1
        with self.assertRaisesRegex(ValueError, 'payload changed'):
            smoke.inspect(b)

    def test_truncated_elf_fails(self):
        with self.assertRaisesRegex(ValueError, 'size'):
            smoke.inspect(smoke.build_bytes()[:-4])

    def test_bad_segment_fails(self):
        b = bytearray(smoke.build_bytes())
        struct.pack_into('<I', b, 52 + 24, 0)  # no executable flag
        with self.assertRaisesRegex(ValueError, 'segment'):
            smoke.inspect(b)

    def test_cli_create_and_check_no_rewrite(self):
        with tempfile.TemporaryDirectory() as t:
            out = Path(t) / 'sub' / 'synthetic.elf'
            for extra in ([], ['--check']):
                result = subprocess.run([sys.executable, str(MODULE_PATH), '--out', str(out), *extra],
                                        capture_output=True, text=True, check=True)
                self.assertIn('SYNTHETIC_AOT_FIXTURE_OK', result.stdout)
            before = out.read_bytes()
            self.assertEqual(before, smoke.build_bytes())
            self.assertEqual(out.read_bytes(), before)


if __name__ == '__main__':
    unittest.main()