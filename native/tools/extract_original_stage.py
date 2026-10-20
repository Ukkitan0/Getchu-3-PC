#!/usr/bin/env python3
"""Extract ACTUAL retail Ape Escape 3 USA stage files from your OWN original ISO.

No mock scene, no synthetic assets, no emulator and no PS2 guest execution.
Walks ISO9660 primary directory and authentic DATA.BIN/VFI index.
'SZ' means uint32 original size followed by raw-DEFLATE (per Saru3VfiTool).
Extracted game data MUST stay owner-private; do not commit these files.
"""
from __future__ import annotations
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import zlib

SECTOR=2048
MAX_TABLE=32*1024*1024
MAX_MEMBER=512*1024*1024
def u16(b,o):return struct.unpack_from("<H",b,o)[0]
def u32(b,o):return struct.unpack_from("<I",b,o)[0]
def record(b,offset):
    n=b[offset]
    if n<34 or offset+n>len(b):raise ValueError("Corrupt ISO directory entry")
    name_len=b[offset+32]
    if 33+name_len>n:raise ValueError("Corrupt ISO name")
    return (b[offset+33:offset+33+name_len].decode("ascii"),
            u32(b,offset+2)*SECTOR,u32(b,offset+10),bool(b[offset+25]&2))
def find_databin(image):
    image.seek(16*SECTOR)
    pvd=image.read(SECTOR)
    if len(pvd)!=SECTOR or pvd[1:6]!=b"CD001":
        raise ValueError("Not an ISO9660 Ape Escape 3 USA disc")
    _,loc,size,directory=record(pvd,156)
    if not directory or size>32*1024*1024:
        raise ValueError("Corrupt ISO root directory")
    image.seek(loc)
    root=image.read(size)
    at=0
    while at<len(root):
        if root[at]==0:
            at=((at//SECTOR)+1)*SECTOR
            continue
        name,start,length,is_directory=record(root,at)
        if name.upper()=="DATA.BIN;1" and not is_directory:
            return start,length
        at+=root[at]
    raise ValueError("Retail ISO has no DATA.BIN")
def vfi_entries(image,offset,size):
    image.seek(offset)
    head=image.read(32)
    if head[:4]!=b"VFI\x00":
        raise ValueError("DATA.BIN has wrong VFI signature")
    count=u16(head,16)
    dirs=u16(head,18)
    table,folders,end=u32(head,20),u32(head,24),u32(head,28)
    if not (32<=table<=folders<=end<=MAX_TABLE):
        raise ValueError("Invalid original VFI tables")
    image.seek(offset)
    data=image.read(end)
    if len(data)!=end:raise ValueError("Short original VFI index")
    p=table
    raw=[]
    for _ in range(count):
        if p+12>folders:raise ValueError("Truncated VFI file table")
        n,parent=u16(data,p),u16(data,p+2)
        if n<13 or p+n>folders:raise ValueError("Corrupt VFI original filename")
        name=data[p+12:p+n].split(b"\0",1)[0].decode("utf-8")
        file_offset=u32(data,p+4)*SECTOR
        length=u32(data,p+8)
        if file_offset+length>size:raise ValueError("Original VFI file exceeds DATA.BIN")
        raw.append((name,parent,file_offset,length))
        p+=n
    names={}
    p=folders
    for _ in range(dirs):
        if p+8>end:raise ValueError("Truncated VFI folder")
        n,parent=u16(data,p),u16(data,p+4)
        if n<9 or p+n>end:raise ValueError("Corrupt VFI folder name")
        names[p-folders]=(data[p+8:p+n].split(b"\0",1)[0].decode("utf-8"),parent)
        p+=n
    entries={}
    for name,parent,place,length in raw:
        path=name
        seen=set()
        for _ in range(65):
            if not parent:break
            if parent in seen or parent not in names:raise ValueError("VFI directory corruption")
            seen.add(parent)
            part,parent=names[parent]
            path=part+"/"+path
        else:raise ValueError("VFI folder chain exceeds limit")
        parts=Path(path).parts
        if not parts or any(p in ("..",".") or not p for p in parts):
            raise ValueError("Unsafe original archive path")
        entries[path.lower()]=(path,place,length)
    return entries
def extract_original(iso:Path,stage:str,out:Path):
    if not stage or len(stage)>64 or not all(x.isalnum() or x=="_" for x in stage):
        raise ValueError("Stage must be an original game identifier")
    if out.exists() and any(out.iterdir()):
        raise FileExistsError("Choose an empty PRIVATE output folder")
    with iso.open("rb") as image:
        data_base,data_size=find_databin(image)
        entries=vfi_entries(image,data_base,data_size)
        base="debug/us/stage/"+stage.lower()+"/"
        chosen=[e for name,e in entries.items() if name.startswith(base)]
        if not chosen:raise ValueError("No authentic retail stage named "+stage)
        selected=[]
        for name,relative,size in sorted(chosen):
            if size>MAX_MEMBER:raise ValueError("Huge original source asset refused")
            image.seek(data_base+relative)
            raw=image.read(size)
            if len(raw)!=size:raise ValueError("Short retail game asset read")
            if name.lower().endswith(".sz"):
                if len(raw)<4:raise ValueError("Truncated original SZ header")
                target=u32(raw,0)
                if target>MAX_MEMBER:raise ValueError("Oversized original SZ expansion")
                decoder=zlib.decompressobj(-15)
                raw=decoder.decompress(raw[4:],target+1)
                if len(raw)!=target or not decoder.eof or decoder.unconsumed_tail:
                    raise ValueError("Original SZ size/termination check failed")
                name=name[:-3]
            selected.append((name,raw))
    out.mkdir(parents=True,exist_ok=True)
    manifests=[]
    for name,raw in selected:
        dest=out/name
        dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_bytes(raw)
        manifests.append({"path":name,"bytes":len(raw),
                          "sha256":hashlib.sha256(raw).hexdigest()})
    summary={"kind":"ACTUAL_AE3_USA_RETAIL_STAGE","stage":stage,
             "original_files":len(manifests),"sample":manifests,
             "game_data_is_user_private":True,
             "hardware_emulation":False,
             "claim":"Files extracted; original game not yet native playable"}
    (out/"original_stage_file_hashes.json").write_text(
        json.dumps(summary,indent=2)+"\n",encoding="utf-8")
    return summary
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("owned_iso",type=Path)
    p.add_argument("stage",help="Real retail area ID, e.g. seaside_a")
    p.add_argument("--out",type=Path,required=True,
                   help="Fresh EMPTY owner-private directory outside repo")
    args=p.parse_args()
    print(json.dumps(extract_original(args.owned_iso,args.stage,args.out),
                     indent=2),flush=True)
if __name__=="__main__":main()
