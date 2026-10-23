"""Original AE3 stage-by-stage Lua native dependency report.

This is a complete static *readiness audit*, not a replacement actor factory.
It consumes only evidence produced by the owner's genuine USA executable and
original Lua 5.0 bytecode. The status never claims a playable stage.
"""
from __future__ import annotations

from collections import Counter
import json
from pathlib import Path, PurePosixPath

SCHEMA = "getchu3-original-stage-native-readiness/v1"
MAX_STAGES = 1000
MAX_LUA_SOURCES = 10000
MAX_GLOBALS_PER_SCRIPT = 2000


def _stage_of_script(path: str) -> str:
    """Native opcode audit paths are relative to debug/us/stage/."""
    if not isinstance(path, str) or not path or "\\" in path or "\x00" in path:
        raise ValueError("Invalid original stage Lua source path")
    parts = PurePosixPath(path).parts
    if (len(parts) < 2 or any(part in ("", ".", "..") for part in parts)
            or path.startswith("/") or ":" in path or "//" in path):
        raise ValueError("Unsafe original stage Lua source path")
    return parts[0].casefold()


def build_original_native_readiness(
        stage_catalog: dict, elf_evidence: dict, source_registry: dict,
        output: Path) -> dict:
    """Correlate source-recovered original script calls and imported assets.

    Neither an observed original CreateBind registration nor a symbol present
    in our LuaStage proves that the corresponding original C++ callback works.
    Classify those separately from truly *missing* original registrations.
    """
    if stage_catalog.get("schema") != "getchu3-original-stage-catalog/v1":
        raise ValueError("Only the source-verified original stage catalog is supported")
    if elf_evidence.get("schema") != "getchu3-original-lua50-binding-evidence/v1":
        raise ValueError("Only the verified USA ELF/Lua opcode evidence is supported")
    if source_registry.get("schema") != "getchu3-source-asset-registry/v1":
        raise ValueError("Only the owner-original imported asset registry is supported")
    if any(x.get("gameplay_ready") is not False
           for x in (stage_catalog, source_registry)):
        raise ValueError("Input stage or source registry incorrectly claims gameplay readiness")
    source_stages = stage_catalog.get("stages")
    asset_stages = source_registry.get("original_stages")
    scripts = elf_evidence.get("original_stage_inputs")
    rows = elf_evidence.get("rows")
    if (not isinstance(source_stages, list) or not source_stages or
        len(source_stages) > MAX_STAGES or not isinstance(asset_stages, list) or
        not isinstance(scripts, list) or len(scripts) > MAX_LUA_SOURCES or
        not isinstance(rows, list)):
        raise ValueError("Missing or excessive verified original stage evidence")
    known_stages: dict[str, dict] = {}
    for entry in source_stages:
        name = entry.get("stage")
        if not isinstance(name, str) or not name or name.casefold() in known_stages:
            raise ValueError("Invalid or duplicate original stage identity")
        known_stages[name.casefold()] = entry
    imported = {}
    for entry in asset_stages:
        name = entry.get("stage")
        if not isinstance(name, str) or name.casefold() in imported:
            raise ValueError("Invalid or duplicate imported original stage")
        imported[name.casefold()] = entry
    if set(imported) != set(known_stages):
        raise ValueError("Imported source registry does not cover the original stages")
    source_calls: dict[str, list[dict]] = {name: [] for name in known_stages}
    for script in scripts:
        relative = script.get("path")
        stage = _stage_of_script(relative)
        if stage not in source_calls:
            raise ValueError(f"Original Lua audit references unknown stage {stage}")
        calls = script.get("direct_global_calls")
        if (not isinstance(calls, list) or len(calls) > MAX_GLOBALS_PER_SCRIPT or
            any(not isinstance(call, str) or not call for call in calls)):
            raise ValueError(f"Invalid original direct calls in {relative}")
        source_calls[stage].append(script)
    original_rows = {}
    for row in rows:
        name = row.get("name")
        if not isinstance(name, str) or not name or name in original_rows:
            raise ValueError("Invalid original native callback evidence identity")
        original_rows[name] = row
    results = []
    for stage in sorted(known_stages):
        entries = source_calls[stage]
        refs = imported[stage].get("resolved_source_placements")
        if not isinstance(refs, list):
            raise ValueError("Imported original stage is missing PLC placement audit")
        missing = []
        native_names = []
        script_defined = []
        unverified = []
        observed = sorted(set(call for item in entries
                              for call in item["direct_global_calls"]))
        for name in observed:
            row = original_rows.get(name)
            if row is None:
                missing.append({"name": name, "reason": "not_in_elf_reconciliation"})
                continue
            if row.get("defined_by_original_script"):
                script_defined.append(name)
            elif row.get("present_in_native_host"):
                # Name-only host registration is NOT original engine semantics.
                unverified.append(name)
            elif row.get("registered_at_scanned_original_CreateBind"):
                native_names.append({"name": name,
                                     "original_register_sites":
                                     row.get("original_register_sites", [])})
            else:
                missing.append({"name": name,
                                "reason": row.get("priority", "unresolved")})
        classes = sorted(set(name
                             for item in entries
                             for name in item.get("observed_actor_class_literals", [])))
        controllers = sorted(set(name
                                 for item in entries
                                 for name in item.get("observed_controller_command_literals", [])))
        linked = sum(item.get("resolution") == "linked_original_model_source"
                     for item in refs)
        unresolved_placements = sum(item.get("resolution") != "linked_original_model_source"
                                    for item in refs)
        results.append({
            "original_stage": stage,
            "original_scripts_analyzed": len(entries),
            "original_actor_classes_seen": classes,
            "original_controller_commands_seen": controllers,
            "direct_global_calls_observed": observed,
            "defined_in_original_lua": script_defined,
            "original_elf_registered_but_not_host_implemented": native_names,
            "native_host_name_exists_semantics_unverified": unverified,
            "unresolved_original_calls": missing,
            "plc_model_sources_linked": linked,
            "plc_model_sources_unresolved": unresolved_placements,
            "original_actor_manager_complete": False,
            "original_lua_callbacks_semantically_verified": False,
            "gameplay_ready": False,
        })
    report = {
        "schema": SCHEMA,
        "status": "original_native_binding_evidence_incomplete",
        "original_elf_sha256": elf_evidence.get("verified_usa_elf_sha256"),
        "stage_count": len(results),
        "scripts_analyzed": sum(len(s) for s in source_calls.values()),
        "stages_with_unresolved_calls": sum(
            bool(item["unresolved_original_calls"] or
                 item["original_elf_registered_but_not_host_implemented"])
            for item in results),
        "gameplay_ready": False,
        "stages": results,
        "critical_caveat": (
            "Static Lua callsites and CreateBind names are not original actor, "
            "physics, animation or gameplay implementations; none is certified playable."),
    }
    destination = Path(output).resolve()
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(
        json.dumps(report, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")
    return report
