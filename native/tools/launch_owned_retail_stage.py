#!/usr/bin/env python3
"""Run only REAL owner-extracted Ape Escape 3 stage assets in the native port.

This is the actual original-game resource path, not a scene fixture generator.
Requires an authentic retail USA ISO (whose area.luc executes in native Lua
5.0), owner-converted I3D GLBs, and matching original PLC / I3C assets.
NEVER invents missing original material, collision, AI or Lua behavior.
The native game is incomplete: passing asset checks does not prove playability.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys

I3C_MAGIC=b"I3D_I3C\x00"

def stage_segment(path: Path, stage: str) -> bool:
    parts=[part.casefold() for part in path.parts]
    key=stage.casefold()
    return any(parts[i]=="stage" and parts[i+1]==key
               for i in range(len(parts)-1))

def stage_candidates(root: Path, stage: str, suffixes: set[str]) -> list[Path]:
    if not root.is_dir():
        return []
    candidates=[]
    for path in root.rglob("*"):
        if path.is_symlink() or not path.is_file():
            continue
        if path.suffix.casefold() not in suffixes:
            continue
        if stage_segment(path.relative_to(root),stage):
            candidates.append(path.resolve())
        if len(candidates)>5000:
            raise ValueError("Stage asset scan exceeds safe budget")
    return sorted(candidates)

def original_collision(files: list[Path]) -> list[Path]:
    return [path for path in files if path.stat().st_size>=8 and
            path.open("rb").read(8)==I3C_MAGIC]

def choose_glb_folder(glbs: list[Path], stage: str) -> Path | None:
    # Stage's actual geometry must come from one stage directory.
    # All source assets must have the same source stage-ID folder.
    for file in glbs:
        parts=file.parts
        for i in range(len(parts)-1):
            if parts[i].casefold()=="stage" and parts[i+1].casefold()==stage.casefold():
                return Path(*parts[:i+2])
    return None

def launch(*, iso: Path, stage: str, extracted: Path, glb: Path, exe: Path,
           spawn: str="spawn_default", dry_run: bool=False,
           report_path: Path|None=None, runner=subprocess.run) -> dict:
    if not stage or len(stage)>70 or not all(c.isalnum() or c=="_" for c in stage):
        raise ValueError("Stage must be a real game stage ID (letters/digits/underscore)")
    if not iso.is_file():raise FileNotFoundError("owned Ape Escape 3 USA ISO not found")
    if not exe.is_file():raise FileNotFoundError("Getchu3Native.exe not built")
    if not extracted.is_dir():
        raise FileNotFoundError("original owner-extracted retail game files missing")
    if not glb.is_dir():
        raise FileNotFoundError("real I3D-to-GLB conversion output missing")
    source=stage_candidates(extracted,stage,{".plc",".i3c",".i3d"})
    original_plcs=[p for p in source if p.name.casefold()=="placeholder.plc"]
    collision=original_collision(source)
    model_files=stage_candidates(glb,stage,{".glb"})
    real_models=[p for p in model_files if p.stat().st_size>20 and
                 p.open("rb").read(4)==b"glTF"]
    folder=choose_glb_folder(real_models,stage)
    report={
        "schema":"getchu3-original-stage-launch-gate/v1",
        "stage":stage,
        "source":"original user-owned ISO and extracted commercial assets",
        "retail_lua_path":f"debug/us/stage/{stage}/area.luc",
        "original_iso":str(iso.resolve()),
        "original_plc_candidates":[str(p) for p in original_plcs],
        "original_i3c_candidates":[str(p) for p in collision],
        "native_glb_model_count":len(real_models),
        "native_glb_stage_directory":str(folder) if folder else None,
        "gameplay_verified":False,
        "missing_original_requirements":[],
    }
    if len(original_plcs)!=1:
        report["missing_original_requirements"].append(
            f"Exactly one authentic placeholder.plc for {stage} is needed; found {len(original_plcs)}")
    if not collision:
        report["missing_original_requirements"].append(
            f"No I3D_I3C collision data from the SAME original stage {stage}")
    if not real_models or folder is None:
        report["missing_original_requirements"].append(
            f"No converted original I3D GLB models from stage {stage}")
    if report_path:
        report_path.parent.mkdir(parents=True,exist_ok=True)
        report_path.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")
    if report["missing_original_requirements"]:
        raise RuntimeError(
            "Authentic Ape Escape 3 stage resource prerequisites missing:\n"+
            "\n".join(report["missing_original_requirements"])+
            "\nNo placeholder level will be launched.")
    # The original ISO itself supplies genuine area.luc. The native runtime
    # verifies that the script and stage ID exist before reading the bytecode.
    # A directory argument lets native loader concatenate ALL collision
    # members from this stage (no guessed single-file substitution).
    collision_dir=choose_glb_folder(collision,stage)
    if collision_dir is None:
        raise RuntimeError("Cannot determine original collision stage folder")
    command=[str(exe.resolve()),"--retail",str(iso.resolve()),stage,
             str(folder),str(original_plcs[0]),str(collision_dir),spawn]
    report["native_argv"]=command
    report["status"]="original_sources_located_native_game_incomplete"
    if dry_run:
        return report
    result=runner(command,check=False)
    report["native_process_exit_code"]=result.returncode
    if result.returncode!=0:
        raise RuntimeError(f"Original native AE3 stage process exited {result.returncode}")
    return report

def main(argv=None)->int:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--iso",type=Path,required=True)
    p.add_argument("--stage",required=True)
    p.add_argument("--extracted",type=Path,required=True)
    p.add_argument("--glb",type=Path,required=True)
    p.add_argument("--exe",type=Path,default=Path("native/build/Release/Getchu3Native.exe"))
    p.add_argument("--spawn",default="spawn_default")
    p.add_argument("--report",type=Path)
    p.add_argument("--dry-run",action="store_true")
    args=p.parse_args(argv)
    result=launch(iso=args.iso,stage=args.stage,extracted=args.extracted,
                  glb=args.glb,exe=args.exe,spawn=args.spawn,
                  report_path=args.report,dry_run=args.dry_run)
    print(json.dumps(result,indent=2))
    return 0

if __name__=="__main__":
    try:sys.exit(main())
    except (RuntimeError,ValueError,OSError) as e:
        print("ORIGINAL_GAME_NATIVE_LAUNCH_BLOCKED:",e,file=sys.stderr)
        sys.exit(2)
