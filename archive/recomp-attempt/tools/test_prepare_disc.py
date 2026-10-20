"""Synthetic ISO9660 to private native recomp pipeline, with NO retail game data."""
from __future__ import annotations
import hashlib
import os
from pathlib import Path
import struct
import tempfile
import unittest

import prepare_disc
import synthetic_aot_smoke

SECTOR = 2048

def record(name: bytes, lba: int, size: int, directory=False) -> bytes:
    length = 33 + len(name) + (len(name) % 2 == 0)
    data = bytearray(length)
    data[0] = length
    struct.pack_into('<I',data,2,lba)
    struct.pack_into('<I',data,10,size)
    data[25] = 2 if directory else 0
    data[32] = len(name)
    data[33:33+len(name)] = name
    return bytes(data)

def synthetic_elf_with_empty_executable_window() -> bytes:
    data = bytearray(synthetic_aot_smoke.build_bytes())
    # Add empty executable LOAD header while leaving the original test segment.
    struct.pack_into('<H',data,44,2)
    struct.pack_into('<8I',data,84,
                     1, 0, 0x00300000, 0x00300000, 0, 0x4000, 5, 0x4000)
    return bytes(data)

def make_disc(path: Path, elf: bytes, boot='SCUS_975.01') -> None:
    image = bytearray(48 * SECTOR)
    pvd = memoryview(image)[16*SECTOR:17*SECTOR]
    pvd[0:6] = b'\x01CD001'
    struct.pack_into('<I',pvd,80,48)
    root = record(b'\0',20,SECTOR,True)
    pvd[156:156+len(root)] = root
    cnf = ('BOOT2 = cdrom0:\\\\' + boot + ';1\r\nVER = 1.00\r\n').encode('ascii')
    image[21*SECTOR:21*SECTOR+len(cnf)] = cnf
    image[22*SECTOR:22*SECTOR+len(elf)] = elf
    directory = (record(b'SYSTEM.CNF;1',21,len(cnf)) +
                 record(b'SCUS_975.01;1',22,len(elf)))
    image[20*SECTOR:20*SECTOR+len(directory)] = directory
    path.write_bytes(image)

class IsoBootPreparation(unittest.TestCase):
    def test_boot2_subdirectories_and_version(self):
        self.assertEqual(prepare_disc.parse_boot2(
            b'BOOT2 = cdrom0:\\\\DATA\\\\SLUS_123.45;1\r\n'), 'DATA/SLUS_123.45')
        self.assertEqual(prepare_disc.parse_boot2(
            b'VER = 1.0\n BOOT2 = cdrom0:/SCUS_975.01;1\n'), 'SCUS_975.01')

    def test_missing_or_non_disc_boot_refused(self):
        with self.assertRaisesRegex(ValueError,'BOOT2'):
            prepare_disc.parse_boot2(b'BOOT = cdrom0:\\\\SLUS_000.01;1')
        with self.assertRaisesRegex(ValueError,'BOOT2'):
            prepare_disc.parse_boot2(b'BOOT2 = host:foo.exe')

    def test_path_traversal_rejected(self):
        for name in ('../evil', 'foo/../evil', 'C:/exe', '/../../bad'):
            with self.subTest(name=name):
                with self.assertRaisesRegex(ValueError,'Unsafe'):
                    prepare_disc.parse_boot2(('BOOT2 = cdrom0:\\\\' + name).encode())

    @unittest.skipUnless(os.environ.get('AE3_UPSTREAM_PATH'),
                         'Run with AE3_UPSTREAM_PATH on pinned upstream checkout')
    def test_real_upstream_iso_extracts_elf_and_identifies_code_window(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            elf=synthetic_elf_with_empty_executable_window()
            disc=root/'synthetic.iso'
            make_disc(disc,elf)
            owned=root/'owned.elf'
            owned.write_bytes(elf)
            report=prepare_disc.prepare(
                disc,Path(os.environ['AE3_UPSTREAM_PATH']),root/'private',
                hashlib.sha256(elf).hexdigest(),owned)
            self.assertEqual(report['boot_program'],'SCUS_975.01')
            self.assertEqual((root/'private/disc/SCUS_975.01').read_bytes(),elf)
            self.assertEqual(report['code_windows'],[{'base':'00300000','size':16384}])
            self.assertTrue(Path(report['overlays_dir']).is_dir())
            self.assertIsNone(report['modules_dir'])
            self.assertFalse(report['retail_assets_uploaded'])
            self.assertTrue((root/'private/disc_prepare.json').is_file())

    @unittest.skipUnless(os.environ.get('AE3_UPSTREAM_PATH'),
                         'Run with AE3_UPSTREAM_PATH on pinned upstream checkout')
    def test_real_upstream_refuses_wrong_hash_and_mismatched_elf(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            elf=synthetic_elf_with_empty_executable_window()
            disc=root/'synthetic.iso'
            make_disc(disc,elf)
            with self.assertRaisesRegex(ValueError,'SHA256'):
                prepare_disc.prepare(disc,Path(os.environ['AE3_UPSTREAM_PATH']),
                                     root/'private','0'*64)
            self.assertFalse((root/'private/disc/SCUS_975.01').exists())
            other=root/'other.elf'
            other.write_bytes(b'not the exact ISO ELF')
            with self.assertRaisesRegex(ValueError,'does not match'):
                prepare_disc.prepare(disc,Path(os.environ['AE3_UPSTREAM_PATH']),
                                     root/'private',hashlib.sha256(elf).hexdigest(),other)

if __name__=='__main__':
    unittest.main()
