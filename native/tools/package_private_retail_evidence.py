#!/usr/bin/env python3
"""Create an owner-private handoff of ACTUAL Ape Escape 3 scripts and executable.

The full original ISO / extracted DATA.BIN never enters this handoff.
Archive only the original Lua bytecode, registration/API analyses, original
SCUS_975.01 executable, and optionally one selected actual retail stage's
PLC/PCK resources. This is not a public GitHub game asset or demo release.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import zipfile
import sys

MAX_FILES=10000
MAX_SOURCE_BYTES=1024*1024*1024
REQUIRED=("original_iso_recovery_manifest.json",
          "original_lua_engine_api_audit.json",
          "original_elf_lua_registrations.json",
          "SCUS_975.01")


def package(root:Path,target:Path,stage:str|None=None)->dict:
    root=root.resolve(strict=True)
    if not root.is_dir():
        raise NotADirectoryError("Expected PRIVATE original ISO extraction directory")
    target=target.resolve()
    if root==target or target in root.parents:
        raise ValueError("Owner-private handoff ZIP must not replace original ISO data")
    if target.exists():
        raise FileExistsError("Refusing to overwrite private evidence ZIP")
    for item in REQUIRED:
        if not (root/item).is_file():
            raise FileNotFoundError(f"Missing original retail recovery evidence: {item}")
    if stage and (len(stage)>70 or not stage.replace("_","").isalnum()):
        raise ValueError("Invalid original stage identifier")
    tree=root/"vfi"
    if not tree.is_dir():
        raise FileNotFoundError("Original retail VFI extraction was not recovered")
    selected=[root/name for name in REQUIRED]
    selected.extend(sorted(p for p in tree.rglob("*")
                           if p.is_file() and not p.is_symlink()
                           and p.suffix.casefold() in (".luc",".lua")))
    if not any(p.suffix.casefold()==".luc" for p in selected):
        raise ValueError("No original Lua bytecode; no synthetic scripts accepted")
    if stage:
        stage_root=tree/"debug"/"us"/"stage"/stage
        if not stage_root.is_dir():
            raise ValueError("Selected original retail stage does not exist")
        selected.extend(sorted(p for p in stage_root.rglob("*")
                               if p.is_file() and not p.is_symlink()
                               and p.suffix.casefold() in (".pck",".plc",".i3c",".i3d")))
    if len(selected)>MAX_FILES:
        raise ValueError("Too many original evidence files for owner-private ZIP")
    total=sum(p.stat().st_size for p in selected)
    if total>MAX_SOURCE_BYTES:
        raise ValueError("Original game evidence exceeds 1 GiB private ZIP budget")
    target.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(target,"x",compression=zipfile.ZIP_DEFLATED,
                         compresslevel=6,allowZip64=True) as output:
        for item in selected:
            rel=item.relative_to(root).as_posix()
            if item.is_symlink():
                raise ValueError("Original evidence must not include symlinks")
            output.write(item,rel)
    return {"private_zip":str(target),"original_lua_and_evidence_files":len(selected),
            "original_uncompressed_bytes":total,
            "selected_original_stage":stage,
            "synthetic_game_resources":False,
            "do_not_publish_retail_assets":True}


def main(argv=None)->int:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--source",type=Path,required=True)
    p.add_argument("--out",type=Path,required=True)
    p.add_argument("--stage",help="Optionally add one REAL stage's .pck/.plc originals")
    args=p.parse_args(argv)
    report=package(args.source,args.out,args.stage)
    for name,value in report.items():
        print(f"{name}: {value}")
    return 0


if __name__=="__main__":
    try:sys.exit(main())
    except (OSError,ValueError) as e:
        print("PRIVATE_ORIGINAL_EVIDENCE_PACKAGING_FAILED:",e,file=sys.stderr)
        sys.exit(2)
