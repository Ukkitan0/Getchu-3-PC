#!/usr/bin/env python3
"""Inventory globals actually referenced by owner-owned AE3 USA Lua 5.0 chunks.

Reads verified-format bytecode only; never runs Lua, recovers script source,
or writes private assets to Git. Output global reads/writes are evidence of
symbol dependencies, not proof of invocation or native C++ semantics.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import struct

HEADER = bytes.fromhex('1b4c756150010404040608090904')
MAX_CHUNK = 16 * 1024 * 1024
MAX_COUNT = 2_000_000
# Lua 5.0.3 lopcodes.h: GETGLOBAL=5, SETGLOBAL=7, 6-bit opcode,
# iABx operand Bx at bit 6 (A at bit 24). NOT the Lua 5.1+ or modern opcode table.
OP_GETGLOBAL = 5
OP_SETGLOBAL = 7
OP_MOVE = 0
OP_LOADBOOL = 2
OP_LOADNIL = 3
OP_SELF = 11
OP_CALL = 25
OP_TAILCALL = 26
# Exact Lua 5.0 numbering, not Lua 5.1+. These instructions invalidate
# linear register tracking because control flow may merge or jump.
FLOW_BARRIERS = {20, 21, 22, 23, 24, 27, 28, 29, 30}
# Source-defined instructions that produce a single register A.
WRITES_A = {1, 4, 6, 10, 12, 13, 14, 15, 16, 17, 18, 19, 34}


class InvalidChunk(ValueError):
    pass


class Reader:
    def __init__(self, blob: bytes):
        if len(blob) < 32 or len(blob) > MAX_CHUNK:
            raise InvalidChunk('Lua chunk size outside safe budget')
        self.data = memoryview(blob)
        self.pos = 0
        self.prototypes = 0
        self.instructions = 0
        self.global_reads: set[str] = set()
        self.global_writes: set[str] = set()
        self.direct_global_calls: set[str] = set()
        self.prototype_calls: list[dict] = []

    def take(self, size: int) -> memoryview:
        if size < 0 or size > len(self.data) - self.pos:
            raise InvalidChunk(f'truncated original Lua chunk at offset {self.pos}')
        result = self.data[self.pos:self.pos + size]
        self.pos += size
        return result

    def u32(self) -> int:
        return struct.unpack('<I', self.take(4))[0]

    def count(self) -> int:
        count = self.u32()
        if count > MAX_COUNT:
            raise InvalidChunk('Lua prototype field exceeds safe count')
        return count

    def text(self) -> bytes:
        length = self.u32()
        if length > MAX_CHUNK:
            raise InvalidChunk('Lua string exceeds memory budget')
        if length == 0:
            return b''
        result = bytes(self.take(length))
        if result[-1] != 0:
            raise InvalidChunk('Lua string missing NUL terminator')
        return result[:-1]

    @staticmethod
    def symbol(raw: bytes) -> str | None:
        # A global symbol is usually ASCII; do not invent replacements for
        # binary bytes or control characters in original game strings.
        if len(raw) > 256 or not raw or any(c < 32 or c > 126 for c in raw):
            return None
        return raw.decode('ascii')

    @staticmethod
    def direct_calls(instructions: list[int], constants: list[bytes | None]) -> set[str]:
        """Conservative intra-basic-block Lua 5.0 register provenance.

        Tracks a global loaded into R(A), its MOVE aliases, and the call
        register of OP_CALL/OP_TAILCALL. A branch drops provenance entirely,
        avoiding invented call edges across unknown runtime control flow.
        Indirect table calls and calls through upvalues deliberately remain
        unclassified, not silently attributed to a guessed native binding.
        """
        registers: dict[int, str] = {}
        called: set[str] = set()
        for word in instructions:
            op = word & 63
            a = (word >> 24) & 255
            b = (word >> 15) & 511
            c = (word >> 6) & 511
            if op in FLOW_BARRIERS:
                registers.clear()
            elif op == OP_GETGLOBAL:
                index = (word >> 6) & 0x3FFFF
                name = Reader.symbol(constants[index]) if isinstance(constants[index], bytes) else None
                registers.pop(a, None)
                if name is not None:
                    registers[a] = name
            elif op == OP_MOVE:
                name = registers.get(b)
                registers.pop(a, None)
                if name is not None:
                    registers[a] = name
            elif op == OP_LOADNIL:
                for i in range(a, b + 1):
                    registers.pop(i, None)
            elif op in (OP_CALL, OP_TAILCALL):
                if a in registers:
                    called.add(registers[a])
                if c == 0:
                    registers = {k:v for k,v in registers.items() if k < a}
                else:
                    # The return region begins at A; even a call
                    # expecting zero results may mutate the call register.
                    for i in range(a, a + max(1, c - 1)):
                        registers.pop(i, None)
                if op == OP_TAILCALL:
                    registers.clear()
            elif op == OP_SELF:
                registers.pop(a, None)
                registers.pop(a+1, None)
            elif op in WRITES_A:
                registers.pop(a, None)
            elif op == OP_LOADBOOL:
                registers.pop(a, None)
                if c:
                    registers.clear()
            elif op > 34:
                raise InvalidChunk(f'unknown Lua 5.0 opcode {op}')
        return called

    def function(self, depth: int) -> None:
        self.prototypes += 1
        prototype_id = self.prototypes
        if depth > 90 or self.prototypes > 10000:
            raise InvalidChunk('Lua prototype recursion or count budget reached')
        self.text()                           # function source name
        line_defined = self.u32()
        self.take(4)                          # upvalues, params, vararg, maxstack
        self.take(self.count() * 4)          # original instruction line numbers
        for _ in range(self.count()):         # locals
            self.text()
            self.take(8)
        for _ in range(self.count()):         # upvalue names
            self.text()
        constants: list[bytes | None] = []
        for _ in range(self.count()):
            kind = self.take(1)[0]
            if kind == 0:
                constants.append(None)
            elif kind == 3:
                self.take(4)                  # genuine original float32
                constants.append(None)
            elif kind == 4:
                constants.append(self.text())
            else:
                raise InvalidChunk(f'unexpected Lua 5.0 constant type {kind}')
        for _ in range(self.count()):         # nested functions
            self.function(depth + 1)
        n = self.count()
        self.instructions += n
        if self.instructions > MAX_COUNT:
            raise InvalidChunk('Lua opcode budget reached')
        instructions = self.take(n * 4)
        words = [struct.unpack_from('<I', instructions, i * 4)[0] for i in range(n)]
        for opcode in words:
            operation = opcode & 0x3F
            if operation not in (OP_GETGLOBAL, OP_SETGLOBAL):
                continue
            index = (opcode >> 6) & 0x3FFFF
            if index >= len(constants) or not isinstance(constants[index], bytes):
                raise InvalidChunk(f'Lua global opcode refers to invalid constant {index}')
            name = self.symbol(constants[index])
            if name is not None:
                target = self.global_reads if operation == OP_GETGLOBAL else self.global_writes
                target.add(name)
        direct = self.direct_calls(words, constants)
        self.direct_global_calls.update(direct)
        self.prototype_calls.append({'prototype': prototype_id,
                                     'line_defined': line_defined,
                                     'direct_global_calls': sorted(direct)})

    def parse(self) -> dict:
        if bytes(self.take(14)) != HEADER:
            raise InvalidChunk('not AE3 retail Lua 5.0 little-endian float32 bytecode')
        # The serialized numerical test marker is rounded when stored as float32.
        test, = struct.unpack('<f', self.take(4))
        if int(test) != 31415926:
            raise InvalidChunk('invalid Lua 5.0 float32 test number')
        self.function(0)
        if self.pos != len(self.data):
            raise InvalidChunk('trailing bytes after Lua 5.0 prototype')
        return {'prototypes': self.prototypes, 'instructions': self.instructions,
                'global_reads': sorted(self.global_reads),
                'global_writes': sorted(self.global_writes),
                'direct_global_calls': sorted(self.direct_global_calls),
                'prototype_calls': sorted(self.prototype_calls, key=lambda x: x['prototype'])}


def inspect(blob: bytes) -> dict:
    return Reader(blob).parse()


def host_callbacks(source: Path) -> list[str]:
    text = source.read_text(encoding='utf-8')
    return sorted(set(re.findall(r'registerCallback\("([A-Za-z][A-Za-z0-9_]*)"'
                                 , text)))


def audit(path: Path, host_source: Path | None = None) -> dict:
    if path.is_file():
        sources = [path]
        root = path.parent
    elif path.is_dir():
        root = path
        sources = sorted(p for p in root.rglob('*') if p.is_file() and not p.is_symlink()
                         and p.suffix.lower() == '.luc')
    else:
        raise FileNotFoundError(path)
    if len(sources) > 10000:
        raise ValueError('too many original Lua scripts')
    registered = host_callbacks(host_source) if host_source else []
    reads: set[str] = set()
    writes: set[str] = set()
    direct_calls: set[str] = set()
    stages = []
    for source in sources:
        if source.is_symlink():
            raise ValueError('refusing to read symlinked original script')
        size = source.stat().st_size
        if size > MAX_CHUNK:
            raise InvalidChunk(f'{source}: script exceeds load budget')
        result = inspect(source.read_bytes())
        reads.update(result['global_reads'])
        writes.update(result['global_writes'])
        direct_calls.update(result['direct_global_calls'])
        stages.append({'path': source.relative_to(root).as_posix(), **result})
    # These are unresolved *global loads*, not necessarily direct calls.
    # Lua's dynamically installed functions, table reads, and stage-specific
    # initialization sequences still require runtime validation.
    unresolved = sorted(reads - writes - set(registered))
    return {'schema': 'getchu3-original-lua50-global-dependencies/v1',
            'claim': 'opcode global references only; not a call graph or proof of C++ binding semantics',
            'scripts': stages, 'registered_native_names': registered,
            'global_reads': sorted(reads), 'global_writes': sorted(writes),
            'direct_global_calls': sorted(direct_calls),
            'unresolved_direct_global_calls': sorted(
                direct_calls - writes - set(registered)),
            'unresolved_global_reads': unresolved}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('owned_script_or_folder', type=Path)
    parser.add_argument('--host-source', type=Path,
                        help='Path to native/src/lua_stage.cpp for current C++ registrations')
    parser.add_argument('--out', type=Path,
                        help='Owner-private JSON result path, never commit result')
    args = parser.parse_args()
    report = audit(args.owned_script_or_folder, args.host_source)
    encoded = json.dumps(report, indent=2) + '\n'
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(encoded, encoding='utf-8')
    else:
        print(encoded, end='')
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError) as error:
        raise SystemExit(f'ORIGINAL_LUA50_DEPENDENCY_AUDIT_FAILED: {error}') from error
