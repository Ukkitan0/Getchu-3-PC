"""Private, source-attested Lua -> original ELF registration recovery gate.

Runs the existing native original bytecode and SCUS_975.01 registration
reconciliation tools on an owner-verified ISO extraction, without including
original scripts, ELF or guessed gameplay callbacks in the Git repository.
"""
from __future__ import annotations

from collections import Counter
import hashlib
import json
from pathlib import Path
import sys

TARGET_SCHEMA = "getchu3-original-lua50-binding-evidence/v1"
MAX_STAGES = 1000


def create_original_binding_report(
        extracted_root: Path, repo: Path, output: Path) -> dict:
    """Verify original bytes, inventory real Lua calls, rank missing bindings.

    The generated owner-private report is research EVIDENCE, not an
    executable Unreal game manager or an assertion of gameplay readiness.
    """
    extracted = Path(extracted_root).resolve(strict=True)
    repo = Path(repo).resolve(strict=True)
    output = Path(output).resolve()
    original_elf = extracted / "SCUS_975.01"
    scripts_root = extracted / "vfi" / "debug" / "us" / "stage"
    host_source = repo / "native" / "src" / "lua_stage.cpp"
    tools = repo / "native" / "tools"
    if not original_elf.is_file() or not scripts_root.is_dir() or not host_source.is_file():
        raise FileNotFoundError(
            "The original USA executable, original stage Lua or native host source is missing")
    if extracted == output or extracted in output.parents:
        raise ValueError("Original ELF and Lua source tree cannot contain the binding report")
    if output == repo / "native" / "src" / "lua_stage.cpp":
        raise ValueError("Original native host source cannot be overwritten")
    scripts = [p for p in scripts_root.rglob("area.luc") if p.is_file() and not p.is_symlink()]
    if not scripts or len(scripts) > MAX_STAGES:
        raise ValueError("No bounded original area.luc stage corpus found")
    # Import the original/native parsers, rather than approximate their ABI.
    # The caller's sys.path is not changed permanently.
    sys.path.insert(0, str(tools))
    try:
        import audit_original_lua50_calls as script_audit
        import map_original_lua_registrations as elf_audit
        import reconcile_original_lua_bindings as reconciliation
        digest = hashlib.sha256(original_elf.read_bytes()).hexdigest()
        if digest != elf_audit.US_SHA256:
            raise ValueError("Original USA executable hash differs from pinned retail source")
        original_calls = script_audit.audit(scripts_root, host_source)
        original_regs = elf_audit.inspect(original_elf.read_bytes())
        evidence = reconciliation.reconcile(
            original_calls, original_regs, script_audit.host_callbacks(host_source))
    finally:
        sys.path.remove(str(tools))
    if evidence.get("schema") != TARGET_SCHEMA:
        raise RuntimeError("Native source Lua binding analysis returned an unexpected schema")
    if evidence.get("retail_scripts_count") != len(scripts):
        raise RuntimeError("Native Lua stage audit did not cover every original area.luc")
    priority = Counter(item["priority"] for item in evidence["rows"])
    actionable = [
        {
            "name": item["name"],
            "priority": item["priority"],
            "original_registration_sites": item["original_register_sites"],
            "original_vtable_evidence": item["published_original_vtable"],
        }
        for item in evidence["rows"] if item["priority"] in (
            "original_direct_call_missing_native_host",
            "original_direct_call_has_published_vtable_not_bound",
            "retail_registration_not_yet_in_native_host",
            "unresolved_direct_call_requires_additional_source_proof")
    ]
    # Save the native tool's complete provenance; don't commit owner bytes.
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(evidence, indent=2) + "\n", encoding="utf-8")
    return {
        "schema": "getchu3-unreal-original-binding-audit/v1",
        "retail_area_scripts_analyzed": evidence["retail_scripts_count"],
        "verified_original_elf_sha256": evidence["verified_usa_elf_sha256"],
        "unresolved_original_bindings": actionable,
        "missing_by_priority": dict(sorted(priority.items())),
        "source_original_actor_class_literals": evidence["observed_actor_class_literals"],
        "source_original_controller_literals": evidence["observed_controller_command_literals"],
        "source_verified_behavior_implemented": False,
        "gameplay_ready": False,
        "owner_private_binding_report": str(output),
        "caveat": "Callsites, registration sites and vtable pointers are separate static evidence; not verified game behavior"
    }
