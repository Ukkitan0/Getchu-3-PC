#!/usr/bin/env python3
"""Inventory REAL owned Ape Escape 3 stage assets. No synthetic fallbacks.

Produces an explicit factual completeness report for original-game stage
assets needed by the native renderer: original area.lua/luc, I3D mesh and
I3D_I3C collision, original PLC placements, converted original GLB models.
It never confuses tests with retail game boot success.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import sys
from launch_owned_retail_stage import original_pck_collision

MAX_FILES=150000
TYPES={".luc",".lua",".i3d",".i3c",".plc",".glb",".i3m",".tm2",".png",".pck"}
MAGIC={
    "original_model":b"I3D_BIN\x00",
    "original_collision":b"I3D_I3C\x00",
    "original_animation":b"I3D_I3M\x00"
}

def bounded_files(root:Path):
    if not root or not root.is_dir():return []
    results=[]
    for f in root.rglob("*"):
        if not f.is_file() or f.is_symlink() or f.suffix.casefold() not in TYPES:
            continue
        if len(results)>=MAX_FILES:raise ValueError("Original source tree exceeds audit budget")
        results.append(f)
    return sorted(results)

def audit(extracted:Path, glb:Path|None=None, lua:Path|None=None)->dict:
    extracted=extracted.resolve(strict=True)
    if not extracted.is_dir():raise ValueError("Original extraction directory missing")
    source=bounded_files(extracted)
    converted=bounded_files(glb) if glb is not None else []
    recovered=bounded_files(lua) if lua is not None else []
    counts={name:0 for name in MAGIC}
    by_type={key:[] for key in ("retail_area_luc","retail_area_lua","original_plc",
                                 "original_model","original_collision","original_animation")}
    for file in source:
        relative=file.relative_to(extracted).as_posix()
        suffix=file.suffix.casefold()
        if suffix==".luc" and file.name.casefold()=="area.luc":
            by_type["retail_area_luc"].append(relative)
        elif suffix==".lua" and file.name.casefold()=="area.lua":
            by_type["retail_area_lua"].append(relative)
        elif suffix==".plc" and file.name.casefold()=="placeholder.plc":
            by_type["original_plc"].append(relative)
        elif suffix==".pck":
            if original_pck_collision(file):
                # This original archive contains authentic I3D_I3C source.
                # The native collision loader decodes each member in memory.
                by_type["original_collision"].append(relative+"::PCK")
                counts["original_collision"]+=1
        elif suffix in (".i3c",".i3d",".i3m"):
            if file.stat().st_size>512*1024*1024:continue
            with file.open("rb") as f:signature=f.read(8)
            for name,magic in MAGIC.items():
                if signature==magic:
                    by_type[name].append(relative)
                    counts[name]+=1
                    break
    valid_glbs=[f.relative_to(glb).as_posix() for f in converted if f.suffix.casefold()==".glb"]
    recovered_lua=[f.relative_to(lua).as_posix() for f in recovered
                   if f.name.casefold()=="area.lua"] if lua else []
    # This is a global *availability* audit, NOT a claim that arbitrary
    # assets are from the same stage or that a game level runs.
    prerequisites={
        "original_area_luc_or_lua":bool(by_type["retail_area_luc"] or
                                         by_type["retail_area_lua"]),
        "recovered_original_area_lua":bool(recovered_lua or by_type["retail_area_lua"]),
        "original_I3D_BIN_geometry":bool(by_type["original_model"]),
        "original_I3D_I3C_collision":bool(by_type["original_collision"]),
        "original_placeholder_plc":bool(by_type["original_plc"]),
        "owner_converted_native_glbs":bool(valid_glbs)
    }
    blockers=[key for key,available in prerequisites.items() if not available]
    report={
        "schema":"getchu3-original-retail-stage-evidence/v1",
        "origin":"user-owned original Ape Escape 3 files only",
        "playable_game_verified":False,
        "ps2_emulator_used":False,
        "source":str(extracted),
        "counts":{
            "retail_area_luc":len(by_type["retail_area_luc"]),
            "retail_area_lua":len(by_type["retail_area_lua"]),
            "recovered_area_lua":len(recovered_lua),
            "original_I3D_BIN":counts["original_model"],
            "original_I3D_I3C":counts["original_collision"],
            "original_I3D_I3M":counts["original_animation"],
            "original_placeholder_plc":len(by_type["original_plc"]),
            "converted_GLBS":len(valid_glbs)
        },
        "prerequisites":prerequisites,
        "missing_prerequisites":blockers,
        "candidate_original_files":{key:value[:40] for key,value in by_type.items()},
        "candidate_converted_models":valid_glbs[:40],
        "recovered_original_lua":recovered_lua[:40],
        "warning":"Global counts do NOT prove a consistent retail level; per-stage asset identity, Lua host bindings, player logic, gadgets and AI remain necessary."
    }
    return report

def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--extracted",type=Path,required=True)
    p.add_argument("--glb",type=Path)
    p.add_argument("--lua",type=Path)
    p.add_argument("--out",type=Path,required=True)
    p.add_argument("--require-real-stage",action="store_true")
    args=p.parse_args(argv)
    report=audit(args.extracted,args.glb,args.lua)
    args.out.parent.mkdir(parents=True,exist_ok=True)
    args.out.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    print("ORIGINAL_RETAIL_STAGE_AUDIT",json.dumps(report["counts"],sort_keys=True))
    print("MISSING_REAL_ORIGINAL_DATA",json.dumps(report["missing_prerequisites"]))
    if args.require_real_stage and report["missing_prerequisites"]:
        return 2
    return 0

if __name__=="__main__":
    try:sys.exit(main())
    except (OSError,ValueError) as e:
        print("ORIGINAL_STAGE_AUDIT_FAILED",e,file=sys.stderr)
        sys.exit(1)
