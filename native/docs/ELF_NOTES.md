# Original SCUS_975.01 — reverse-engineering notes

**Research, not implementations.** Only publish addresses, field meanings,
cross-references and findings; never commit original executable bytes.

- Original retail USA ELF SHA-256:
  `5a8a73cf741b2e923f652f9bdf35d1798471ff31fb960b63ed010a78d1ac2d63`.
- Verified original native Lua registration helper: `0x00255BD8`.
- Seventeen registration call sites through that path and their string
  addresses are recorded in
  `native/reference/verified_original_retail_api_sites.json`.
- Many Lua game functions (including `new_ent`, `c_ctrl`,
  `c_order`) may use another registration path. Do **not** assume
  that absence from the 17 sites implies they are not original.

Track each recovered function with address, calling convention, argument
types, state reads/writes, evidence source and validation status before
porting its behavior to native C++.
