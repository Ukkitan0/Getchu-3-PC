#!/usr/bin/env python3
"""Convert Saru-extracted I3D_BIN geometry to host-native glTF using MIT ae3-sdk.

ae3-sdk supplies the separately installed 'ae3 gltf' developer utility.
It runs OFFLINE and is not linked into Getchu3Native.exe.
Do not commit Sony assets or resulting GLBs to the public source repository.
"""
from __future__ import annotations
import argparse
from collections import Counter
import json
from pathlib import Path
import shutil
import subprocess
import sys

MAGIC=b"I3D_BIN\x00"
MAX_MODELS=4096
MAX_BYTES=512*1024*1024


def export_models(extracted: Path, output: Path, ae3: str = "ae3",
                  *, max_models=MAX_MODELS, animations=False, textures=None,
                  runner=subprocess.run) -> dict:
    extracted = extracted.resolve(strict=True)
    output = output.resolve()
    if not extracted.is_dir():
        raise NotADirectoryError("Expected Saru3VfiTool extracted asset directory")
    if output == extracted or output in extracted.parents or extracted in output.parents:
        raise ValueError("Keep converted GLBs separate from original extracted Sony data")
    if not shutil.which(ae3) and not Path(ae3).is_file():
        raise FileNotFoundError("ae3-sdk CLI missing: install MIT pxdl/ae3-sdk tools first")
    models=[]
    candidates=0
    for path in extracted.rglob("*"):
        if not path.is_file() or path.is_symlink() or path.suffix.casefold()!=".i3d":
            continue
        candidates+=1
        if path.stat().st_size>MAX_BYTES:
            continue
        with path.open("rb") as stream:
            if stream.read(8)!=MAGIC:
                continue
        models.append(path)
        if len(models)>max_models:
            raise ValueError("Unreasonably many I3D_BIN models")
    output.mkdir(parents=True,exist_ok=True)
    counts=Counter()
    failed=[]
    for source in sorted(models):
        relative=source.relative_to(extracted)
        destination=output/relative.parent
        destination.mkdir(parents=True,exist_ok=True)
        expected=destination/(source.stem+".glb")
        # Static conversions may be reused, but upgrading to original
        # I3M skeletal clips MUST regenerate a prior static GLB. Otherwise
        # -Animate appears to succeed without ever creating animations.
        # Only derived GLBs are regenerated; source I3D/I3M/TIM2 is untouched.
        if expected.exists() and not animations:
            counts["already_converted"]+=1
            continue
        upgrading=expected.exists()
        command=[ae3,"gltf",str(source),"--out",str(destination),
                 "--textures",str(textures if textures is not None else extracted)]
        if animations:
            # MIT ae3-sdk matches I3M named tracks to source skeletons
            # only when its independent match/coverage gates both pass.
            # --anims with no list requests this conservative auto-pairing.
            command.append("--anims")
        try:
            result=runner(command,capture_output=True,timeout=120,check=False)
            if result.returncode!=0 or not expected.is_file():
                failed.append(relative.as_posix())
                counts["failed"]+=1
            else:
                counts["upgraded_to_original_animation"]+=1 if upgrading else 0
                counts["converted"]+=1
        except (OSError,subprocess.TimeoutExpired):
            failed.append(relative.as_posix())
            counts["failed"]+=1
    report={
        "format":"getchu3-native-i3d-to-glb/v1",
        "offline_ae3_sdk":True,
        "requested_original_i3m_animation_pairing": bool(animations),
        "animation_pairing_coverage_unverified_until_glb_inspection": True,
        "original_i3d_bin_files":len(models),
        "all_i3d_extension_files":candidates,
        "results":dict(sorted(counts.items())),
        "failed_sources":failed,
        "ps2_hardware_emulation":False,
        "native_runtime_ae3_sdk_dependency":False
    }
    (output/"native_glb_manifest.json").write_text(
        json.dumps(report,indent=2)+"\n",encoding="utf-8")
    return report


def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--extracted",type=Path,required=True)
    p.add_argument("--out",type=Path,required=True)
    p.add_argument("--ae3",default="ae3",help="Installed ae3-sdk 'ae3' executable")
    p.add_argument("--animations",action="store_true",
                   help="Bake original named I3M bone tracks into GLBs through "
                        "ae3-sdk's conservative resolution/coverage gates")
    args=p.parse_args(argv)
    report=export_models(args.extracted,args.out,args.ae3,animations=args.animations)
    print(json.dumps(report,indent=2))
    return 1 if report["results"].get("failed",0) else 0


if __name__=="__main__":
    try:
        sys.exit(main())
    except (OSError,ValueError) as exc:
        print("NATIVE_I3D_CONVERSION_ERROR:",exc,file=sys.stderr)
        sys.exit(1)
