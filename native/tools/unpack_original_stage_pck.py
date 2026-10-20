#!/usr/bin/env python3
"""Expose real Ape Escape 3 stage PCK mesh/animation/texture data to the native converter.

Only extracts owner-local original-format data inside the PRIVATE VFI extraction.
Never creates models, replaces gameplay or reads PlayStation machine instructions.
Collision stays in PCK because Getchu3Native now decodes it directly in memory,
avoiding duplicate collision triangles.
"""
from __future__ import annotations
import argparse
from collections import Counter
import json
from pathlib import Path
import sys
import struct

MAX_PCK_BYTES=512*1024*1024
MAX_PCKS=2048
MAX_MEMBERS=100000
EXPORT_MAGIC={
    b"I3D_BIN\x00":".i3d",
    b"I3D_I3M\x00":".i3m",
    b"TIM2":".tm2",
}
COLLISION_MAGIC=b"I3D_I3C\x00"


def original_name(data: bytes, table: int, relative: int) -> str:
    pool=16
    if table<pool or relative>=table-pool:
        return "unnamed"
    at=pool+relative
    start=at
    while start>pool and data[start-1]!=0:
        start-=1
    if data[at]==0:
        end=at
    else:
        end=data.find(b"\x00",at,table)
    if end<=start or end<0 or end-start>255:
        return "unnamed"
    raw=data[start:end]
    if any(c<32 or c>126 for c in raw):
        return "unnamed"
    return raw.decode("ascii")


def clean_name(name: str) -> str:
    value="".join(c if c.isascii() and (c.isalnum() or c in "_-") else "_"
                  for c in name)
    return (value or "unnamed")[:80]


def scan_pck(path: Path) -> tuple[list[tuple[str,bytes]],Counter]:
    if not path.is_file() or path.is_symlink():
        raise ValueError("Original PCK must be a regular private file")
    size=path.stat().st_size
    if size<16 or size>MAX_PCK_BYTES:
        raise ValueError(f"Original PCK size invalid: {path}")
    raw=path.read_bytes()
    if raw[:4]!=b"PCK\x00":
        raise ValueError(f"Not an original PCK: {path}")
    table,count=struct.unpack_from("<II",raw,4)
    end=table+count*16
    if table<16 or count>MAX_MEMBERS or end>len(raw):
        raise ValueError(f"Corrupt original PCK table: {path}")
    entries=[]
    for i in range(count):
        name_id,_,offset,length=struct.unpack_from("<IIII",raw,table+i*16)
        if offset<end or offset>len(raw) or length>len(raw)-offset:
            raise ValueError(f"Corrupt original PCK member bounds: {path} member {i}")
        entries.append((i,name_id,offset,length))
    exported=[]
    counts=Counter()
    for i,name_id,offset,length in entries:
        prefix=raw[offset:offset+8]
        if prefix==COLLISION_MAGIC:
            counts["collision_remains_in_original_pck"]+=1
            continue
        extension=next((ext for magic,ext in EXPORT_MAGIC.items()
                        if prefix.startswith(magic)),None)
        if extension is None:
            counts["other_original_members"]+=1
            continue
        filename=f"{i:04d}_{clean_name(original_name(raw,table,name_id))}{extension}"
        exported.append((filename,raw[offset:offset+length]))
        counts[extension]+=1
    return exported,counts


def unpack(extracted: Path, stage: str|None=None) -> dict:
    root=extracted.resolve(strict=True)
    stage_root=root/"debug"/"us"/"stage"
    if not stage_root.is_dir():
        raise ValueError("Original extracted USA debug/us/stage tree missing")
    if stage and (not stage.replace("_","").isalnum() or len(stage)>70):
        raise ValueError("Invalid original stage ID")
    packs=sorted(p for p in stage_root.rglob("*")
                 if p.is_file() and not p.is_symlink() and
                 p.suffix.casefold()==".pck" and "pck_assets" not in p.parts and
                 (stage is None or p.relative_to(stage_root).parts[0].casefold()==stage.casefold()))
    if len(packs)>MAX_PCKS:
        raise ValueError("Too many original game stage PCK archives")
    totals=Counter()
    touched=0
    for pack in packs:
        exports,counts=scan_pck(pack)
        totals.update(counts)
        target=pack.parent/"pck_assets"/clean_name(pack.stem)
        for filename,bytes_ in exports:
            destination=target/filename
            if destination.exists():
                if not destination.is_file() or destination.read_bytes()!=bytes_:
                    raise FileExistsError(f"Private original PCK extraction differs: {destination}")
                totals["existing_exact_match"]+=1
                continue
            target.mkdir(parents=True,exist_ok=True)
            with destination.open("xb") as output:
                output.write(bytes_)
            touched+=1
    return {
        "schema":"getchu3-owned-stage-pck-unpack/v1",
        "original_stage_pck_archives":len(packs),
        "new_original_members_written":touched,
        "counts":dict(sorted(totals.items())),
        "retail_collision_decoded_from_pck_at_runtime":True,
        "synthetic_assets_generated":False,
        "original_files_remain_private":True,
    }


def main(argv=None)->int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--extracted",type=Path,required=True)
    parser.add_argument("--stage",help="Only unpack this original stage ID")
    args=parser.parse_args(argv)
    print(json.dumps(unpack(args.extracted,args.stage),indent=2))
    return 0


if __name__=="__main__":
    try:sys.exit(main())
    except (OSError,ValueError) as exc:
        print("ORIGINAL_STAGE_PCK_UNPACK_FAILED:",exc,file=sys.stderr)
        sys.exit(2)
