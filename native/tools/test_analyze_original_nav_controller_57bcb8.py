#!/usr/bin/env python3
"""Synthetic decoder/CFG tests; no original copyrighted executable needed."""
import tempfile
import unittest
from pathlib import Path
from analyze_original_nav_controller_57bcb8 import (
    ANCHORS, START, END, audit_original, analyze_words, branch_target,
    decode, jump_target, load_original_region, signed16,
)


class RetailControllerDecoderTests(unittest.TestCase):
    def test_signed_branch_target(self):
        self.assertEqual(signed16(0xfffc),-4)
        self.assertEqual(branch_target(0x57bce0,0x11e00012),0x57bd2c)
        self.assertEqual(branch_target(0x57bd24,0x45010002),0x57bd30)
        self.assertEqual(jump_target(0x57bae4,0x0c15ef2e),0x57bcb8)

    def test_all_known_verified_anchors(self):
        a={pc:decode(pc,word) for pc,word in ANCHORS.items()}
        self.assertEqual(a[0x57bcb8].mnemonic,'addiu')
        self.assertEqual(a[0x57bcb8].operands,'sp, sp, -128')
        self.assertEqual(a[0x57bcdc].mnemonic,'lbu')
        self.assertEqual(a[0x57bcdc].operands,'t7, 48(a0)')
        self.assertEqual(a[0x57bce0].mnemonic,'beq')
        self.assertEqual(a[0x57bce0].target,'0x0057bd2c')
        self.assertEqual(a[0x57bd24].mnemonic,'bc1t')
        self.assertEqual(a[0x57bd24].target,'0x0057bd30')

    def test_unresolved_cop2_does_not_get_made_up(self):
        self.assertEqual(decode(START,0x4a000001).mnemonic,'cop2_unresolved')
        self.assertEqual(decode(START,0x70000000).mnemonic,'mmi_unresolved')

    def test_control_flow_and_delay_slot(self):
        words=[0x27bdff80,0x11e00002,0x908f0030,0x00000000,
               0x45010002,0x00000000,0x03e00008,0x00000000]
        report=analyze_words(words)
        self.assertEqual(report['instruction_count'],8)
        self.assertEqual(len(report['control_transfers']),3)
        self.assertTrue(report['instructions'][2]['delay_slot'])
        self.assertTrue(report['instructions'][5]['delay_slot'])
        self.assertTrue(report['instructions'][7]['delay_slot'])
        self.assertEqual(report['control_transfers'][0]['target'],'0x0057bcc8')
        self.assertEqual(report['control_transfers'][1]['target'],'0x0057bcd4')
        self.assertEqual(report['control_transfers'][2]['kind'],'return')
        self.assertEqual(report['basic_blocks'][0]['start'],'0x0057bcb8')
        self.assertEqual(report['basic_blocks'][0]['successors'],
                         ['0x0057bcc8','0x0057bcc4'])
        self.assertEqual(report['basic_blocks'][-1]['successors'],[])

    def test_likely_branch_annuls_delay_slot_when_not_taken(self):
        report=analyze_words([0x51000001,0x00000000,0x00000000])
        self.assertEqual(report['control_transfers'][0]['kind'],'branch')
        self.assertTrue(report['control_transfers'][0]['annul_delay_on_not_taken'])

    def test_missing_final_delay_slot_rejected(self):
        with self.assertRaises(ValueError):
            analyze_words([0x03e00008])

    def test_cli_refuses_unverified_source(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'SCUS_975.01'
            p.write_bytes(b'not a genuine USA game executable')
            with self.assertRaisesRegex(ValueError,'SHA-256 mismatch'):
                load_original_region(p)
            with self.assertRaisesRegex(ValueError,'SHA-256 mismatch'):
                audit_original(p)

    def test_region_size_and_source_anchors(self):
        self.assertEqual((END-START)//4,33)
        self.assertEqual(set(ANCHORS),{0x57bcb8,0x57bcdc,0x57bce0,0x57bd24})


if __name__=='__main__':
    unittest.main()
