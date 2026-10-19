// CI link smoke ONLY: intentionally does not contain game code or images.
// The native ps2run linker needs the two registration hooks emitted by eerecomp.
// Actual game builds use translated owner-supplied ELF code instead.
void register_generated_code() {}
void register_generated_vu() {}