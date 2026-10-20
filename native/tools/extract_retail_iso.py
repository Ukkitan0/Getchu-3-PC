#!/usr/bin/env python3
"""Extract REAL Ape Escape 3 (USA) Lua scripts and/or full game assets from owner's ISO.

Reads ISO9660 + the retail DATA.BIN/VFI index directly. Original Lua 5.0
bytecode is copied unchanged, including scripts outside debug/us/stage.
Optional --all includes every VFI member (PCK, PLC, TIM2, I3D, audio, etc.).
The SCUS_975.01 executable is copied from the actual ISO root in private
workspace for static reconstruction of game Lua callback registration sites.
No emulator/PS2 runtime, no fabricated scripts, and no upload of retail assets.
"""
from __future__ import annotations
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import struct
import sys
import zlib

from extract_original_stage import (
    MAX_MEMBER, find_databin, record, vfi_entries, u32, SECTOR
)

MAX_FILES=70000
MAX_TOTAL=24*1024*1024*1024
MAX_DECOMPRESS=MAX_MEMBER
ELF_NAME="SCUS_975.01"
SCRIPT_ENDINGS=(".luc",".lua")


def safe_path(name: str) -> Path:
    path=PurePosixPath(name.replace("\\","/"))
    parts=path.parts
    if not parts or name.startswith(("/","\\")) or any(
        part in ("",".","..") or ":" in part or "\x00" in part
        or any(ord(c)<32 for c in part) for part in parts
    ):
        raise ValueError(f"Unsafe original retail resource path: {name!r}")
    return Path(*parts)


def root_entries(image):
    """List actual root files and enforce ISO extent boundaries."""
    image.seek(0,2)
    length=image.tell()
    image.seek(16*SECTOR)
    pvd=image.read(SECTOR)
    if len(pvd)!=SECTOR or pvd[0]!=1 or pvd[1:6]!=b"CD001" or pvd[6]!=1:
        raise ValueError("Not an original ISO9660 primary volume descriptor")
    _,base,size,is_dir=record(pvd,156)
    if not is_dir or size>32*1024*1024 or base+size>length:
        raise ValueError("Invalid original game ISO root")
    image.seek(base)
    data=image.read(size)
    if len(data)!=size:
        raise ValueError("Truncated game ISO root")
    cursor=0
    entries={}
    while cursor<len(data):
        if data[cursor]==0:
            cursor=((cursor//SECTOR)+1)*SECTOR
            continue
        name,offset,bytes_,folder=record(data,cursor)
        cursor+=data[cursor]
        if folder or name in ("\x00","\x01"):
            continue
        if offset>length or bytes_>length-offset:
            raise ValueError("Original ISO file outside disc")
        entries[name.split(";",1)[0].upper()]=(offset,bytes_)
    return entries


def expand_original(data: bytes, name: str) -> tuple[str,bytes]:
    if not name.lower().endswith(".sz"):
        return name,data
    if len(data)<4:
        raise ValueError(f"Truncated SZ-compressed retail member: {name}")
    length=u32(data,0)
    if length>MAX_DECOMPRESS:
        raise ValueError("Original SZ expansion exceeds per-member limit")
    decoder=zlib.decompressobj(-15)
    original=decoder.decompress(data[4:],length+1)
    original+=decoder.flush()
    if len(original)!=length or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError(f"Invalid original SZ stream or size: {name}")
    return name[:-3],original


def extract(iso: Path,out: Path,*,all_resources=False,include_elf=True) -> dict:
    iso=iso.resolve(strict=True)
    out=out.resolve()
    if not iso.is_file() or iso.suffix.lower()!=".iso":
        raise ValueError("Provide your own original Ape Escape 3 ISO")
    if out==iso.parent or out==iso or out in iso.parents:
        raise ValueError("Private output must be separate from original ISO")
    if out.exists() and any(out.iterdir()):
        raise FileExistsError("Refusing to overwrite an existing original-data output; choose a new empty directory")
    selected=[]
    elf=None
    original_scripts=0
    original_bytes=0
    compressed=0
    with iso.open("rb") as image:
        base,total=find_databin(image)
        entries=vfi_entries(image,base,total)
        if not entries:
            raise ValueError("Original VFI contains no files")
        if len(entries)>MAX_FILES:
            raise ValueError("Original retail VFI index exceeds extraction budget")
        for archive_path,(_,relative,size) in sorted(entries.items()):
            logical=archive_path[:-3] if archive_path.endswith(".sz") else archive_path
            if not all_resources and not logical.endswith(SCRIPT_ENDINGS):
                continue
            safe_path(logical)
            if size>MAX_MEMBER:
                raise ValueError("Original VFI file exceeds per-member size budget")
            if original_bytes+size>MAX_TOTAL:
                raise ValueError("Original extraction exceeds private data budget")
            selected.append((archive_path,relative,size))
            original_bytes+=size
            if logical.endswith(SCRIPT_ENDINGS):
                original_scripts+=1
        if not selected:
            raise ValueError("Retail VFI did not contain any Lua scripts; no invented data extracted")
        root=root_entries(image)
        if include_elf:
            if ELF_NAME not in root:
                raise ValueError("Original USA SCUS_975.01 ELF was not found in disc root")
            elf=root[ELF_NAME]
            if elf[1]>MAX_MEMBER or elf[1]<52:
                raise ValueError("Original ELF extent is invalid")
        # Fail preflight path collisions before any original output is written.
        output_names=set()
        for name,_,_ in selected:
            final=safe_path(name[:-3] if name.endswith(".sz") else name).as_posix().casefold()
            if final in output_names:
                raise ValueError(f"Collision between original VFI members: {name}")
            output_names.add(final)

        # Read all original member data from user-owned disc, preserving Lua code.
        # Raw bytes are not put in the public source repository.
        manifest=[]
        out.mkdir(parents=True,exist_ok=True)
        for name,relative,size in selected:
            image.seek(base+relative)
            raw=image.read(size)
            if len(raw)!=size:
                raise ValueError(f"Short original VFI member read: {name}")
            original_name,payload=expand_original(raw,name)
            if original_name!=name:
                compressed+=1
            final=out/"vfi"/safe_path(original_name)
            final.parent.mkdir(parents=True,exist_ok=True)
            with final.open("xb") as writer:
                writer.write(payload)
            manifest.append({
                "source":name,
                "path":final.relative_to(out).as_posix(),
                "size":len(payload),
                "sha256":hashlib.sha256(payload).hexdigest()
            })
        elf_result=None
        if elf is not None:
            image.seek(elf[0])
            contents=image.read(elf[1])
            if len(contents)!=elf[1] or contents[:4]!=b"\x7fELF":
                raise ValueError("Original SCUS_975.01 is not a complete ELF")
            final=out/ELF_NAME
            with final.open("xb") as writer:
                writer.write(contents)
            elf_result={"path":ELF_NAME,"size":len(contents),
                        "sha256":hashlib.sha256(contents).hexdigest()}
    counts=Counter(Path(item["path"]).suffix.lower() for item in manifest)
    result={
        "schema":"getchu3-original-retail-iso-full-lua-recovery/v1",
        "source":"Owner-provided original Ape Escape 3 USA ISO",
        "iso_sha256":None,
        "vfi_index_entries":len(entries),
        "extraction_mode":"all_original_vfi_resources" if all_resources else "all_original_lua_scripts",
        "original_lua_scripts":original_scripts,
        "original_vfi_files_extracted":len(manifest),
        "original_sz_members_decompressed":compressed,
        "resource_counts":dict(sorted(counts.items())),
        "original_elf":elf_result,
        "private_output":str(out),
        "manifest":manifest,
        "not_playable":True,
        "retail_assets_must_not_be_committed_or_distributed":True
    }
    (out/"original_iso_recovery_manifest.json").write_text(
        json.dumps(result,indent=2)+"\n",encoding="utf-8"
    )
    return {k:v for k,v in result.items() if k!="manifest"}


def main(argv=None)->int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso",type=Path,help="Original user-owned Ape Escape 3 (USA).iso")
    parser.add_argument("--out",type=Path,required=True,help="New EMPTY PRIVATE output folder")
    parser.add_argument("--all",action="store_true",help="Extract ALL original VFI game data, not only Lua")
    parser.add_argument("--no-elf",action="store_true",help="Skip original ELF extraction")
    args=parser.parse_args(argv)
    print(json.dumps(extract(args.iso,args.out,all_resources=args.all,
                             include_elf=not args.no_elf),indent=2))
    return 0


if __name__=="__main__":
    try:
        sys.exit(main())
    except (OSError,ValueError,zlib.error) as e:
        print("ORIGINAL_RETAIL_ISO_EXTRACTION_FAILED:",e,file=sys.stderr)
        sys.exit(2)
