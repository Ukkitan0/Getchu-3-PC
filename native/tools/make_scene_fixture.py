#!/usr/bin/env python3
"""Generate a game-free textured glTF 2.0 scene for testing native D3D11.

No commercial AE3 geometry or game textures. This fixture is a real GLB 2.0
with two native mesh node instances, UVs, embedded PNG and index buffer.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import struct
import zlib

def png(rgba_width=8):
    width=height=rgba_width
    def chunk(name,data):
        return struct.pack(">I",len(data))+name+data+struct.pack(
            ">I",zlib.crc32(name+data)&0xffffffff)
    raw=bytearray()
    for y in range(height):
        raw.append(0)
        for x in range(width):
            tile=(x//2+y//2)%2
            raw.extend((54,174,245,255) if tile else (246,172,65,255))
    return (b"\x89PNG\r\n\x1a\n"+
            chunk(b"IHDR",struct.pack(">IIBBBBB",width,height,8,6,0,0,0))+
            chunk(b"IDAT",zlib.compress(bytes(raw),9))+
            chunk(b"IEND",b""))

def build_glb():
    # One original-shaped floor plane. Not from Ape Escape 3.
    points=[
       (-3,0,-3), (3,0,-3),(3,0,3),(-3,0,3)
    ]
    texcoords=[(0,0),(2,0),(2,2),(0,2)]
    indices=[0,2,1,0,3,2]
    data=bytearray()
    views=[]
    def add_buffer(blob,target=None):
        while len(data)%4:data.append(0)
        offset=len(data)
        data.extend(blob)
        view={"buffer":0,"byteOffset":offset,"byteLength":len(blob)}
        if target is not None:view["target"]=target
        views.append(view)
        return len(views)-1
    pos=add_buffer(b"".join(struct.pack("<3f",*p) for p in points),34962)
    uv=add_buffer(b"".join(struct.pack("<2f",*p) for p in texcoords),34962)
    index=add_buffer(struct.pack("<6H",*indices),34963)
    image=add_buffer(png())
    while len(data)%4:data.append(0)
    gltf={
      "asset":{"version":"2.0","generator":"Getchu3 Native game-free test"},
      "buffers":[{"byteLength":len(data)}],
      "bufferViews":views,
      "accessors":[
        {"bufferView":pos,"componentType":5126,"count":4,"type":"VEC3",
         "min":[-3,0,-3],"max":[3,0,3]},
        {"bufferView":uv,"componentType":5126,"count":4,"type":"VEC2"},
        {"bufferView":index,"componentType":5123,"count":6,"type":"SCALAR"}
      ],
      "images":[{"bufferView":image,"mimeType":"image/png"}],
      "samplers":[{"magFilter":9729,"minFilter":9729,"wrapS":10497,"wrapT":10497}],
      "textures":[{"sampler":0,"source":0}],
      "materials":[{"pbrMetallicRoughness":{"baseColorFactor":[1,1,1,1],
                    "baseColorTexture":{"index":0}},"doubleSided":True}],
      "meshes":[{"primitives":[{"attributes":{"POSITION":0,"TEXCOORD_0":1},
                               "indices":2,"mode":4,"material":0}]}],
      "nodes":[{"name":"Native test arena","mesh":0},
               {"name":"Second independent node","mesh":0,"translation":[8,0,0]}],
      "scenes":[{"nodes":[0,1]}],"scene":0
    }
    encoded=json.dumps(gltf,separators=(",",":")).encode("utf8")
    encoded+=b" "*((-len(encoded))%4)
    length=12+8+len(encoded)+8+len(data)
    return (struct.pack("<III",0x46546C67,2,length)
            +struct.pack("<II",len(encoded),0x4e4f534a)+encoded
            +struct.pack("<II",len(data),0x004e4942)+data)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("out",type=Path)
    args=parser.parse_args()
    args.out.parent.mkdir(parents=True,exist_ok=True)
    out=build_glb()
    args.out.write_bytes(out)
    print("Generated true-native textured GLB fixture",args.out,
          "bytes",len(out))
if __name__=="__main__":
    main()
