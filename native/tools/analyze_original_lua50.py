#!/usr/bin/env python3
"""Audit ACTUAL Ape Escape 3 area.luc bytecode to locate unported native APIs.

Runs against owner-extracted retail scripts, never fake stages. Directly
decodes original Lua 5.0 GETGLOBAL/SETGLOBAL instruction operands per
https://www.lua.org/source/5.0/lopcodes.h.html
No game bytes or copyrighted Lua scripts are written to this repository.
"""
from __future__ import annotations
import argparse
from collections import Counter
import json
from pathlib import Path
import struct

# Actually implemented game APIs only. Registered callbacks that merely
# raise on original retail bytecode MUST remain on the missing list:
# stage_var_get_int, is_survival_mode. They cannot be counted as playable.
IMPLEMENTED_NATIVE={
  "new_ent","lua_param","c_ctrl","c_order",
  "gw_set_flag","lw_set_flag","area_set_flag","gw_get_flag",
  "lw_get_flag","area_get_flag","get_capture_flag",
  "get_costume_possess"
}
# Host Lua 5.0 interpreter builtins (not game engine functions).
LUA_NATIVE={
 "assert","error","getfenv","getmetatable","ipairs","next",
 "pairs","pcall","print","rawequal","rawget","rawset",
 "select","setfenv","setmetatable","tonumber","tostring",
 "type","unpack","xpcall","math","string","table","_G","_VERSION",
 "coroutine"
}
class RetailLua:
    def __init__(self,raw:bytes):
        self.raw=raw
        self.pos=0
        self.get=Counter()
        self.set=Counter()
        self.functions=0
        self.instructions=0
        self.constants=0
    def read(self,n:int)->bytes:
        if n<0 or self.pos+n>len(self.raw):
            raise ValueError(f"Lua bytecode truncated at offset {self.pos}")
        data=self.raw[self.pos:self.pos+n]
        self.pos+=n
        return data
    def u32(self)->int:
        return struct.unpack("<I",self.read(4))[0]
    def lua_string(self)->str:
        size=self.u32()
        if size>16*1024*1024:
            raise ValueError("Unreasonable original Lua string size")
        data=self.read(size)
        if size and data[-1]!=0:raise ValueError("Unterminated Lua string")
        return data[:-1].decode("latin1") if size else ""
    def parse_function(self,depth=0):
        if depth>90 or self.functions>10000:raise ValueError("Too many game functions")
        self.functions+=1
        self.lua_string()
        self.u32()
        self.read(4)
        line_count=self.u32()
        self.read(line_count*4)
        for _ in range(self.u32()):
            self.lua_string()
            self.read(8)
        for _ in range(self.u32()):self.lua_string()
        constants=[]
        for _ in range(self.u32()):
            kind=self.read(1)[0]
            if kind==0:constants.append(None)
            elif kind==3:constants.append(struct.unpack("<f",self.read(4))[0])
            elif kind==4:constants.append(self.lua_string())
            else:raise ValueError(f"Unknown original Lua constant tag {kind}")
            self.constants+=1
        for _ in range(self.u32()):self.parse_function(depth+1)
        instruction_count=self.u32()
        self.instructions+=instruction_count
        for _ in range(instruction_count):
            word=self.u32()
            opcode=word&0x3f
            if opcode not in (5,7):continue
            bx=(word>>6)&0x3ffff
            if bx>=len(constants) or not isinstance(constants[bx],str):
                raise ValueError(f"GETGLOBAL/SETGLOBAL invalid constant index {bx}")
            (self.get if opcode==5 else self.set)[constants[bx]]+=1
    def analyze(self):
        if len(self.raw)<18 or self.raw[:14]!=bytes(
            [27,76,117,97,80,1,4,4,4,6,8,9,9,4]):
            raise ValueError("Not a retail AE3 Lua 5.0 32bit/float32 chunk")
        self.pos=18
        self.parse_function()
        if self.pos!=len(self.raw):raise ValueError("Unexpected original bytecode trailing bytes")
        return self
def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("original",type=Path,help="Original owner-local area.luc or extracted stage directory")
    p.add_argument("--out",type=Path,help="Optional local JSON report path OUTSIDE the public repo")
    p.add_argument("--all-scripts",action="store_true",
                   help="Inspect EVERY original .luc file, not just area.luc scripts")
    args=p.parse_args(argv)
    files=[args.original] if args.original.is_file() else sorted(
        f for f in args.original.rglob("*") if f.is_file() and not f.is_symlink()
        and f.name.casefold().endswith(".luc")
        and (args.all_scripts or f.name.casefold()=="area.luc"))
    if not files:raise SystemExit("No original area.luc files")
    stage={}
    gets=Counter()
    sets=Counter()
    for f in files:
        r=RetailLua(f.read_bytes()).analyze()
        gets.update(r.get)
        sets.update(r.set)
        defined=set(r.set)|LUA_NATIVE|IMPLEMENTED_NATIVE
        # A GETGLOBAL can refer to a value supplied by another stage/engine
        # even when SETGLOBAL is missing. Report without guessing semantics.
        unresolved={k:v for k,v in r.get.items() if k not in defined}
        stage[str(f)]={"functions":r.functions,"instructions":r.instructions,
                       "globals_read":dict(r.get.most_common()),
                       "globals_written":dict(r.set.most_common()),
                       "unresolved_engine_or_external_globals":unresolved}
    implemented=set(sets)|LUA_NATIVE|IMPLEMENTED_NATIVE
    missing={k:v for k,v in gets.most_common() if k not in implemented}
    report={"schema":"native-original-ae3-retail-lua50-game-api-audit/v1",
            "source":"real owned Lua 5.0 bytecode, NOT synthetic examples",
            "scripts":len(files),
            "all_scripts_scanned":bool(args.all_scripts),
            "total_getglobal_sites":sum(gets.values()),
            "total_setglobal_sites":sum(sets.values()),
            "external_or_unimplemented_globals":missing,
            "files":stage,
            "warning":"Missing names may be game engine functions, globals or extension libraries. No missing behavior is implemented by this audit."}
    if args.out:
        args.out.parent.mkdir(parents=True,exist_ok=True)
        args.out.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print(f"ORIGINAL_AE3_LUA_ENGINE_AUDIT: {len(files)} original Lua scripts")
    print(f"ORIGINAL_LUA_GLOBAL_READS {sum(gets.values())}")
    print(f"UNKNOWN_GAME_ENGINE_GLOBALS {len(missing)}")
    for name,count in list(missing.items())[:120]:
        print(f"  {count:5} {name}")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
