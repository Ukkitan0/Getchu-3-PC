"""Non-destructive package namespace for each owner-original ISO import.

The original source SHA identifies the disc; the unique private workspace
identifies an import attempt. Neither user-edited previous Unreal assets nor
a partially failed import are overwritten by later runs.
"""
from __future__ import annotations

from pathlib import Path
import re

DISC_HASH = re.compile(r"[0-9a-f]{64}\Z")
WORKSPACE = re.compile(r"([0-9a-f]{16})-([a-z0-9_]{6,32})\Z")


def original_import_package_root(disc_sha256: str, workspace: Path) -> str:
    """Return an Unreal-valid package root scoped to one original ISO import."""
    if not isinstance(disc_sha256, str) or not DISC_HASH.fullmatch(disc_sha256):
        raise ValueError("Original ISO must have a verified lower-case SHA-256 digest")
    workspace = Path(workspace)
    if workspace.name != str(workspace.name) or not workspace.name:
        raise ValueError("Original import workspace name is invalid")
    match = WORKSPACE.fullmatch(workspace.name)
    if not match or match.group(1) != disc_sha256[:16]:
        raise ValueError("Original import workspace does not belong to verified disc")
    suffix = match.group(2)
    return ("/Game/ApeEscape3/Original/Imports/"
            + "Disc_" + disc_sha256[:16] + "_Run_" + suffix)
