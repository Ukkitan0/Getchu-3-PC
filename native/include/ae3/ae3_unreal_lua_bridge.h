#pragma once
/* C ABI for the existing authentic Ape Escape 3 Lua 5.0 native runtime.
 *
 * Source: native/src/lua_stage.cpp and native/src/retail_lua50.cpp.
 * This is NOT a Lua replacement, MIPS interpreter, or substitute engine.
 *
 * All data bytes belong to the owner and stay local to the process. Host
 * callbacks MUST implement original source-backed game manager behavior.
 * Reject a callback until that behavior is available; never acknowledge
 * synthetic actors, no-op flags or invented stage parameters as success.
 */
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#  if defined(AE3_LUA_BRIDGE_BUILD)
#    define AE3_LUA_EXPORT __declspec(dllexport)
#  else
#    define AE3_LUA_EXPORT __declspec(dllimport)
#  endif
#else
#  define AE3_LUA_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AE3_LUA_BRIDGE_API_VERSION 5u

typedef struct Ae3LuaRuntime Ae3LuaRuntime;

/* Every user string is copied into this library; callbacks must not keep the
 * borrowed pointers past the call. Return 1 only after the real manager
 * successfully accepts the operation. Zero rejects the operation and makes
 * original Lua stop with a genuine binding error.
 *
 * Entity class/name/tag and flag scope/key are supplied as exact byte hex.
 * Lua field JSON uses a LOSSLESS typed representation:
 *  nil: {"kind":"nil"}
 *  boolean: {"kind":"boolean","value":true}
 *  number: {"kind":"number","value":1.5}
 *  text: {"kind":"text","hex":"..."} (exact original string bytes)
 *  array: {"kind":"array","items":[...]}
 *  object: {"kind":"object","entries":[{"key_hex":"...","value":...}]}
 * This is not a guessed gameplay schema. No source bytes are lost by Unicode
 * transcoding, embedded NUL, or JSON key normalization.
 */
typedef struct Ae3LuaManagerV5 {
    uint32_t api_version;
    uint32_t struct_size;
    void* user;

    int (*create_entity)(void*, const char* original_class_hex,
                         const char* original_name_hex, const char* original_tag_hex,
                         const char* original_fields_json,
                         const char* original_controllers_json);
    int (*submit_parameters)(void*, const char* original_parameters_json);
    int (*write_flag)(void*, const char* scope_hex, const char* key_hex, int value);
    /* 1 means known, 0 means original manager data unavailable. */
    int (*read_flag)(void*, const char* scope_hex, const char* key_hex, int* value);
    int (*read_stage_int)(void*, const char* typed_args_json, int* value);
    int (*read_survival)(void*, int* value);
    int (*select_weapon)(void*, int original_selection);
    /* Returns 1 only when 1033 original source bytes were provided. */
    int (*read_original_player_progress)(void*, void* destination, size_t exact_size);
    /* 1033 bytes; manager MUST atomically commit source-backed player state.
     * Return 0 if player manager/write ownership is missing or cannot commit.
     * The native bridge will NEVER synthesize or cache a successful write.
     */
    int (*write_original_player_progress)(void*, const void* source, size_t exact_size);
    /* ONLY source-recovered, verified original-ELF void-return Lua callbacks.
     * original_name is an ASCII Lua global, native address is an independently
     * verified retail function VA (NOT a CreateBind JAL site).
     * typed_arguments_json preserves source Lua byte values. Return 1 only
     * after the original native behavior has actually been performed.
     */
    int (*invoke_original_void)(void*, uint32_t original_va,
                                const char* original_name,
                                const char* typed_arguments_json);
} Ae3LuaManagerV5;

/* All API functions return 1 on success, 0 on failure.
 * On failure ae3_lua_last_error provides the error text while the handle
 * remains alive. All operations and callback invocations are synchronous
 * and must run on the Unreal game thread. Callbacks MUST NOT re-enter.
 */
AE3_LUA_EXPORT uint32_t ae3_lua_api_version(void);
AE3_LUA_EXPORT Ae3LuaRuntime* ae3_lua_create(const Ae3LuaManagerV5* manager);
/* Returns 0 without destroying if Lua or a manager callback is executing.
 * Callers MUST NOT unload the host DLL or discard the handle after 0.
 */
AE3_LUA_EXPORT int ae3_lua_destroy(Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT int ae3_lua_reset(Ae3LuaRuntime* runtime);
/* Register one *actually implemented* original source Lua function before
 * executing its stage chunk. Registration requires an original verified VA,
 * a valid global name, and a live invoke_original_void manager. Never
 * auto-register functions merely because a name appears in the ELF audit.
 * Call fails on duplicate or built-in globals, invalid VA, or after execution.
 */
AE3_LUA_EXPORT int ae3_lua_register_original_void(
    Ae3LuaRuntime* runtime, const char* original_name, uint32_t original_va);
AE3_LUA_EXPORT int ae3_lua_execute_original_chunk(
    Ae3LuaRuntime* runtime, const void* bytes, size_t length,
    const char* original_chunk_path);
AE3_LUA_EXPORT int ae3_lua_setup_area(Ae3LuaRuntime* runtime,
                                       const char* original_spawn_tag);
/* Native port of the original ChangeHP delta-and-clamp routine.
 * Reuses recovered 1033-byte PlayerProgress and refuses mutations unless
 * the real host provides both read and atomic write managers. On success,
 * out_health is the committed original player_life, not a simulated value.
 */
AE3_LUA_EXPORT int ae3_lua_apply_original_health_delta(
    Ae3LuaRuntime* runtime, float delta, float* out_health);
AE3_LUA_EXPORT const char* ae3_lua_last_error(const Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT size_t ae3_lua_declared_entity_count(const Ae3LuaRuntime* runtime);
AE3_LUA_EXPORT size_t ae3_lua_parameter_count(const Ae3LuaRuntime* runtime);

#ifdef __cplusplus
}
#endif
