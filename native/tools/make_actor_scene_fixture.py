#!/usr/bin/env python3
"""Original-free native AE3 stage demo with visible 3D player/monkey proxies.

Every asset is procedurally generated and unrelated to Sony game meshes.
Verifies the REAL Lua new_ent -> C++ actor -> host-native D3D11 world route.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import struct
from make_scene_fixture import build_glb


def make_character(kind: str) -> bytes:
    points: list[tuple[float,float,float]] = []
    colors: list[tuple[float,float,float,float]] = []

    def cube(center, half, rgb):
        cx,cy,cz=center
        dx,dy,dz=half
        v=[(cx+sx*dx,cy+sy*dy,cz+sz*dz) for sx,sy,sz in
           [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),
            (-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
        tri=[0,1,2,0,2,3,4,6,5,4,7,6,0,4,5,0,5,1,
             3,2,6,3,6,7,0,3,7,0,7,4,1,5,6,1,6,2]
        for n in tri:
            points.append(v[n])
            colors.append((*rgb,1.0))
    if kind=="player":
        cube((0,1.18,0),(.42,.58,.25),(.22,.42,.98))
        cube((0,2.02,0),(.34,.33,.34),(.98,.72,.48))
        cube((-.56,1.2,0),(.16,.49,.18),(.22,.42,.98))
        cube((.56,1.2,0),(.16,.49,.18),(.22,.42,.98))
        cube((-.22,.37,0),(.17,.35,.20),(.19,.20,.27))
        cube((.22,.37,0),(.17,.35,.20),(.19,.20,.27))
    else:
        cube((0,.7,0),(.35,.48,.25),(.49,.24,.14))
        cube((0,1.42,0),(.43,.41,.35),(.51,.26,.15))
        cube((-.48,1.45,0),(.19,.26,.12),(.72,.45,.22))
        cube((.48,1.45,0),(.19,.26,.12),(.72,.45,.22))
        cube((0,1.34,-.31),(.26,.22,.08),(.86,.64,.39))
        cube((-.19,.25,0),(.14,.29,.15),(.42,.22,.14))
        cube((.19,.25,0),(.14,.29,.15),(.42,.22,.14))
        cube((0,.45,.55),(.12,.15,.56),(.49,.24,.14))
    data=bytearray()
    for xyz in points:data+=struct.pack("<3f",*xyz)
    off=len(data)
    for color in colors:data+=struct.pack("<4f",*color)
    gltf={
        "asset":{"version":"2.0","generator":"native AE3 actor integration test"},
        "buffers":[{"byteLength":len(data)}],
        "bufferViews":[
            {"buffer":0,"byteOffset":0,"byteLength":off,"target":34962},
            {"buffer":0,"byteOffset":off,"byteLength":len(data)-off,"target":34962}
        ],
        "accessors":[
            {"bufferView":0,"componentType":5126,"count":len(points),"type":"VEC3"},
            {"bufferView":1,"componentType":5126,"count":len(colors),"type":"VEC4"}
        ],
        "meshes":[{"primitives":[{"attributes":{"POSITION":0,"COLOR_0":1},"mode":4}]}],
        "nodes":[{"mesh":0}],
        "scenes":[{"nodes":[0]}],"scene":0
    }
    encoded=json.dumps(gltf,separators=(",",":")).encode()
    encoded+=b" "*(-len(encoded)%4)
    result=bytearray(struct.pack("<III",0x46546c67,2,12+8+len(encoded)+8+len(data)))
    result+=struct.pack("<II",len(encoded),0x4e4f534a)
    result+=encoded
    result+=struct.pack("<II",len(data),0x004e4942)
    result+=data
    return bytes(result)


def build(path: Path) -> None:
    path.mkdir(parents=True,exist_ok=True)
    (path/"test_arena_background_bg.glb").write_bytes(build_glb())
    (path/"native_dummy_player.glb").write_bytes(make_character("player"))
    (path/"native_dummy_monkey.glb").write_bytes(make_character("monkey"))
    plc=bytearray(0xc0+0x80)
    struct.pack_into("<I",plc,0,1)
    label=b"unused_stage_marker"
    plc[0xc0:0xc0+len(label)]=label
    (path/"placeholder.plc").write_bytes(plc)
    (path/"actor_bindings.txt").write_text(
        "# Optional local mapping: never guesses whether models are correct\n"
        "player=native_dummy_player.glb\n"
        "class:Monkey=native_dummy_monkey.glb\n",encoding="utf-8")
    (path/"area.lua").write_text(
        "-- ORIGINAL-FREE test of the real native Lua 5.0 bindings.\n"
        "-- These entities are deliberately not proprietary Ape Escape 3 gameplay.\n"
        "function setupArea(opts)\n"
        " new_ent('Player',{name='player',pos={0,0,0}})\n"
        " new_ent('Monkey',{name='ape_test_01',pos={2.5,0,2.5}})\n"
        " new_ent('Monkey',{name='ape_test_02',pos={-2.5,0,2.5}})\n"
        " lua_param({namespace='native_actor_fixture'})\n"
        "end\n",encoding="utf-8")
    (path/"README.txt").write_text(
        "NATIVE HOST GAME-FREE LUA ACTOR TEST\n\n"
        "1. Open Getchu3Native.exe.\n"
        "2. Choose 'Open GLB level folder' -> this folder.\n"
        "3. Choose 'Run original Lua 5.0 stage' -> area.lua in this folder.\n"
        "4. 3 independent native actor models should render over arena: 1 player, "
        "2 monkeys.\n"
        "5. F5 to toggle native walking; the spawned player model follows the "
        "walk-state position. The other two actors remain separate.\n\n"
        "This is a game-free stand-in to prove the engine plumbing, not an "
        "authentic retail stage, monkey AI or capture mechanics.\n",
        encoding="utf-8")


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("output",type=Path)
    args=p.parse_args()
    build(args.output)
    print("NATIVE_LUA_ACTOR_GAME_FREE_SCENE_CREATED:",args.output)


if __name__=="__main__":
    main()
