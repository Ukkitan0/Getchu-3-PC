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

/* Original Player virtual slot 2 at retail VA 0x34A014.
 * These borrowed pointers MUST refer to genuine source-recovered Player
 * subobjects. No allocation, fallback instance or guessed component occurs.
 * The secondary factory pointer must refer to the genuine original owner;
 * null preserves the original feature gate for the last six entries.
 */
typedef struct Ae3OriginalPlayerResourcePointers {
    void* player_at_5c;
    void* main_player_at_60;
    void* item_chatcher_at_64;
    void* force_at_180;
    void* bar_interface_at_70;
    void* iZone_at_110;
    void* iHanger_at_188;
    void* iRope_at_1a0;
    void* iDashBall_at_1d0;
    void* iHeroSwitch_at_258;
} Ae3OriginalPlayerResourcePointers;

/* key16 is exactly sixteen original bytes (zero padded where required).
 * Returns a borrowed genuine original subobject pointer or null, directly
 * using the recovered source function originalPlayerResourceLookup_34a014.
 */
AE3_LUA_EXPORT void* ae3_original_player_resource_lookup(
    const Ae3OriginalPlayerResourcePointers* pointers,
    const uint8_t* original_key16, void* original_secondary_factory);

/* Original actor controller initializer at SCUS_975.01 VA 0x57BCB8.
 * Records are the exact 0x70-byte original stage spatial records; route
 * polygon indices and navigation bytes must be taken from owner-source data.
 * All native allocations and RNG lifetime belong to this native session.
 * This invokes recovered source math, NOT a fabricated Unreal Actor spawn.
 */
typedef struct Ae3OriginalManagerInitSession Ae3OriginalManagerInitSession;
typedef struct Ae3OriginalVec4 {
    float x,y,z,w;
} Ae3OriginalVec4;
typedef struct Ae3OriginalManagerInitStateV1 {
    uint32_t source_method08, source_method0c;
    uint8_t source_flag1c, source_alternative30;
    uint16_t reserved;
    Ae3OriginalVec4 controller20, target40, midpoint60;
    uint32_t target_polygon_index; /* UINT32_MAX if no target. */
} Ae3OriginalManagerInitStateV1;
typedef struct Ae3OriginalManagerInitSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    uint32_t original_actor_polygon_index;
    const uint32_t* original_route_indices;
    size_t original_route_count;
    const void* original_navigation_bytes;
    size_t original_navigation_size;
    Ae3OriginalVec4 original_owner_position40;
    Ae3OriginalVec4 original_owner_direction50;
} Ae3OriginalManagerInitSourceV1;
typedef struct Ae3OriginalManagerInitCallsV1 {
    void* user;
    /* Original unresolved 0x21B854 must provide a real projected vector. */
    int (*project_original_21b854)(
        void* user, const void* original_polygon70,
        const Ae3OriginalVec4* start, const Ae3OriginalVec4* end,
        Ae3OriginalVec4* projected);
    /* Original unresolved 0x57C7CC must advance real actor/route owner. */
    int (*continue_original_57c7cc)(
        void* user, const Ae3OriginalVec4* original_midpoint);
} Ae3OriginalManagerInitCallsV1;

AE3_LUA_EXPORT Ae3OriginalManagerInitSession*
ae3_original_manager_init_create(uint32_t original_game_seed);
AE3_LUA_EXPORT int ae3_original_manager_init_destroy(
    Ae3OriginalManagerInitSession* session);
/* Return original branch 1 near-owner, 2 kind-two, 3 general projection;
 * 0 on missing owner, malformed source, unresolved callback or exception.
 * output state is written ONLY on success and retains no source pointers.
 * Callers must atomically roll back original manager effects on failure.
 */
AE3_LUA_EXPORT int ae3_original_manager_init_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* original,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV1* real_calls);
/* Version 2 runs recovered 0x57C7CC internally, using its exact original
 * target/route branch logic. Only the independently original-coded
 * 0x21B854, 0x4C1B3C and 0x57C8EC owner operations remain callbacks.
 */
typedef struct Ae3OriginalManagerInitCallsV2 {
    void* user;
    int (*project_original_21b854)(
        void* user, const void* original_polygon70,
        const Ae3OriginalVec4* start, const Ae3OriginalVec4* end,
        Ae3OriginalVec4* projected);
    int (*original_project_4c1b3c)(
        void* user, uint32_t actor_polygon, uint32_t* projected_polygon);
    int (*original_apply_target_57c8ec)(
        void* user, uint32_t target_polygon);
} Ae3OriginalManagerInitCallsV2;
AE3_LUA_EXPORT int ae3_original_manager_init_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalManagerInitSourceV1* original,
    Ae3OriginalManagerInitStateV1* state,
    const Ae3OriginalManagerInitCallsV2* original_calls);
AE3_LUA_EXPORT const char* ae3_original_manager_init_last_error(
    const Ae3OriginalManagerInitSession* session);

/* Original actor-stage polygon selection, direct native versions of
 * 0x21D898 (0x4E3C14 grid-hit list) and 0x21DA2C (all records).
 * The caller supplies ORIGINAL stage-loader 0x70 records, per-polygon
 * 0x1E descriptor point counts and, for the hits-based API, the
 * actual original 16-bit grid candidate indices. The higher-level
 * grid-based API below now executes recovered 0x4E3C14 internally.
 * No spatial grid/candidate source is synthesized by this ABI.
 * Both use recovered 0x21B468 centroid and VU0 XYZ distance; 0x21D898
 * additionally applies the source 0x21B788 XZ plane eligibility check.
 * Point count must be 1..3, record_count 1..8192. The source buffers
 * are read during this synchronous call; none are retained afterward.
 */
typedef struct Ae3OriginalNearestPolygonSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    const uint16_t* original_point_counts;
    const uint16_t* original_hit_indices; /* exact source 0x4E3C14 hits */
    size_t original_hit_count;
    Ae3OriginalVec4 original_query;
} Ae3OriginalNearestPolygonSourceV1;
typedef struct Ae3OriginalNearestPolygonResultV1 {
    uint32_t original_polygon_index; /* UINT32_MAX: no eligible result */
} Ae3OriginalNearestPolygonResultV1;

/* A return of 1 means a source query ran (including a no-hit result);
 * 0 means invalid original inputs or an exception. The output is never
 * modified on failure. Session last_error describes failed validation.
 * Reject reentrant use, including attempts to destroy the active session.
 */
AE3_LUA_EXPORT int ae3_original_polygon_nearest_hits_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalNearestPolygonSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output);
AE3_LUA_EXPORT int ae3_original_polygon_nearest_all_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalNearestPolygonSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output);

/* Higher-level source 0x4E3C14 -> 0x21D898 native query.
 * Accepts genuine grid +0x00/+0x04 origins, +0x10/+0x14 steps,
 * +0x18/+0x1C dimensions, +0x50 u16 cell offsets and +0x08
 * u16 hit pool, all recovered in original_spatial_support.cpp.
 * resolved_x/y are the original two scalar operands supplied to the
 * spatial grid routine AFTER game-specific VU0 operand extraction;
 * they are NOT inferred from Unreal world X/Y or collision geometry.
 * original.original_hit_indices/count are ignored for this entrypoint:
 * the native source grid routine computes the u16 candidate list.
 */
typedef struct Ae3OriginalGridNearestSourceV1 {
    Ae3OriginalNearestPolygonSourceV1 original;
    float original_origin_x00;
    float original_origin_y04;
    float original_step_x10;
    float original_step_y14;
    int32_t original_dimension_x18;
    int32_t original_dimension_y1c;
    const uint16_t* original_cell_offsets50;
    size_t original_cell_offset_count;
    const uint16_t* original_hit_pool08;
    size_t original_hit_pool_count;
    float original_resolved_x;
    float original_resolved_y;
} Ae3OriginalGridNearestSourceV1;
AE3_LUA_EXPORT int ae3_original_polygon_nearest_grid_4e3c14_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalGridNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output);

/* Higher-confidence original spatial-data path: no detached caller
 * point counts. Source 0x21B468 reads the guest header pointer at each
 * 0x70-byte record+0x30, then the 16-bit count at header+0x16.
 * The original stage loader supplies the true allocation base VA and
 * unmodified allocation bytes. These are SOURCE addresses, not pointers
 * to PS2 emulated memory or host process addresses.
 *
 * In both entry points, actual source polygon vertices are read from
 * each record's +0x40/+0x50/+0x60 16-byte VU0 fields. Headers with
 * count 0 or >3, unresolved/ambiguous source addresses, and invalid
 * allocation data are rejected. Original source records/allocations
 * remain owned by the caller and are only borrowed during the call.
 */
typedef struct Ae3OriginalSourceAllocationV1 {
    uint32_t original_base_va;
    const void* original_bytes;
    size_t original_byte_count;
} Ae3OriginalSourceAllocationV1;
typedef struct Ae3OriginalRawNearestSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    const Ae3OriginalSourceAllocationV1* original_allocations;
    size_t original_allocation_count;
    Ae3OriginalVec4 original_query;
} Ae3OriginalRawNearestSourceV1;
typedef struct Ae3OriginalRawGridNearestSourceV1 {
    Ae3OriginalRawNearestSourceV1 original;
    float original_origin_x00, original_origin_y04;
    float original_step_x10, original_step_y14;
    int32_t original_dimension_x18, original_dimension_y1c;
    const uint16_t* original_cell_offsets50;
    size_t original_cell_offset_count;
    const uint16_t* original_hit_pool08;
    size_t original_hit_pool_count;
    /* Original 0x4E3C14 source scalar grid operands after the
     * still-owner-dependent actor VU0 packed-coordinate extraction. */
    float original_resolved_x, original_resolved_y;
} Ae3OriginalRawGridNearestSourceV1;

/* Both APIs execute the actual recovered source-header resolution and
 * source VU0 selector. Outputs are INDEX-only; success with no polygon
 * returns UINT32_MAX. Failed calls leave output unchanged and preserve
 * the session. No guessed Player/Monkey ownership or Unreal raycasting.
 */
AE3_LUA_EXPORT int ae3_original_polygon_nearest_original_data_all_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRawNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output);
AE3_LUA_EXPORT int ae3_original_polygon_nearest_original_data_grid_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRawGridNearestSourceV1* original,
    Ae3OriginalNearestPolygonResultV1* output);

/* Native 0x21D46C stage spatial loader -> recovered 0x21D898 / 0x21DA2C.
 * No reconstructed stage records, detached header counts or arbitrary
 * original allocation list are needed: genuine loaded stage bytes,
 * the real 32-bit runtime base VA, and original query operands suffice.
 * This does NOT load game assets from the disc, implement the source
 * 0x36EDAC grid allocator, or construct game actors for type 1/2 actions.
 */
typedef struct Ae3OriginalStageNearestSourceV1 {
    const void* original_loaded_stage_bytes;
    size_t original_loaded_stage_size;
    uint32_t original_runtime_base_va;
    uint32_t original_registry_owner_va; /* 0 if genuinely absent */
    Ae3OriginalVec4 original_query;
} Ae3OriginalStageNearestSourceV1;
typedef struct Ae3OriginalStageGridNearestSourceV1 {
    Ae3OriginalStageNearestSourceV1 original;
    float original_origin_x00, original_origin_y04;
    float original_step_x10, original_step_y14;
    int32_t original_dimension_x18, original_dimension_y1c;
    const uint16_t* original_cell_offsets50;
    size_t original_cell_offset_count;
    const uint16_t* original_hit_pool08;
    size_t original_hit_pool_count;
    float original_resolved_x, original_resolved_y;
} Ae3OriginalStageGridNearestSourceV1;
/* 1 for completed source query, including empty (UINT32_MAX) result;
 * 0 on invalid genuine stage/owner data. No result is published on
 * failure. The session rejects reentrancy and stays valid afterward.
 */
AE3_LUA_EXPORT int ae3_original_stage_nearest_all_21d46c_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageNearestSourceV1* source,
    Ae3OriginalNearestPolygonResultV1* output);
AE3_LUA_EXPORT int ae3_original_stage_nearest_grid_21d46c_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageGridNearestSourceV1* source,
    Ae3OriginalNearestPolygonResultV1* output);

/* Persistent original-stage spatial owner. The session decodes the
 * source 0x21D46C spatial registry exactly once, preserving source
 * record identity throughout an actual stage lifetime. No Player/
 * Monkey actors or gameplay callbacks are manufactured. Stage input
 * bytes are borrowed only until successful mount returns; afterward
 * the decoded 0x70-byte records and header bindings are session-owned.
 * All calls are synchronous and intended for the game manager thread.
 */
typedef struct Ae3OriginalStageSpatialMountSourceV1 {
    const void* original_loaded_stage_bytes;
    size_t original_loaded_stage_size;
    uint32_t original_runtime_base_va;
    uint32_t original_registry_owner_va;
} Ae3OriginalStageSpatialMountSourceV1;
typedef struct Ae3OriginalStageSpatialGridSourceV1 {
    float original_origin_x00, original_origin_y04;
    float original_step_x10, original_step_y14;
    int32_t original_dimension_x18, original_dimension_y1c;
    const uint16_t* original_cell_offsets50;
    size_t original_cell_offset_count;
    const uint16_t* original_hit_pool08;
    size_t original_hit_pool_count;
    float original_resolved_x, original_resolved_y;
} Ae3OriginalStageSpatialGridSourceV1;
/* Replaces the mounted source stage atomically on success; on failure
 * the previous mounted stage (if any) is still queryable. The source RNG
 * is NOT reseeded or consumed by the stage spatial mount.
 */
AE3_LUA_EXPORT int ae3_original_stage_spatial_mount_21d46c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageSpatialMountSourceV1* original);
AE3_LUA_EXPORT int ae3_original_stage_spatial_unmount(
    Ae3OriginalManagerInitSession* session);
AE3_LUA_EXPORT int ae3_original_stage_spatial_nearest_all_21da2c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalVec4* original_query,
    Ae3OriginalNearestPolygonResultV1* output);
AE3_LUA_EXPORT int ae3_original_stage_spatial_nearest_grid_21d898(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalStageSpatialGridSourceV1* original_grid,
    const Ae3OriginalVec4* original_query,
    Ae3OriginalNearestPolygonResultV1* output);

/* Expose only source-recovered 0x21D46C action descriptors retained
 * during the mounted original stage's 0x1E-byte header traversal.
 * These describe ORIGINAL unresolved type-1/type-2 action requests;
 * reading them does NOT spawn a substitute Unreal entity.
 *
 * The caller first obtains exact count, then provides count-sized buffer
 * to copy all action records in original source order. If capacity is
 * insufficient or stage has been unmounted, the function returns 0
 * without publishing any action records or mutating out_count.
 * For zero pending actions, items may be NULL and capacity zero.
 * The returned records contain no pointers and remain independent of
 * the session after this copy. Calls are synchronous on one owner thread.
 */
typedef struct Ae3OriginalStagePendingActionV1 {
    uint32_t original_record_index;
    uint16_t original_action_index;
    uint16_t original_action_type;
    uint16_t original_argument;
    uint16_t reserved;
} Ae3OriginalStagePendingActionV1;
AE3_LUA_EXPORT int ae3_original_stage_spatial_pending_action_count(
    Ae3OriginalManagerInitSession* session,
    size_t* out_count);
AE3_LUA_EXPORT int ae3_original_stage_spatial_pending_actions(
    Ae3OriginalManagerInitSession* session,
    Ae3OriginalStagePendingActionV1* output,
    size_t output_capacity,
    size_t* out_count);

/* Exact 0x21D46C constructed 0x70-byte spatial record snapshot.
 * Each record's +0x30 source header identity, +0x40..+0x6F VU0
 * vertices, and owner fields remain unchanged bytes from the native
 * recovered stage loader. Header metadata is the verified original
 * descriptor VA / +0x16 source point count, not guessed geometry.
 * All copies are by VALUE: nothing may retain native session pointers.
 */
typedef struct Ae3OriginalStageSpatialRecordV1 {
    uint8_t original_record70[0x70];
    uint32_t original_header_va;
    uint16_t original_point_count;
    uint16_t reserved;
} Ae3OriginalStageSpatialRecordV1;
AE3_LUA_EXPORT int ae3_original_stage_spatial_record_count(
    Ae3OriginalManagerInitSession* session,
    size_t* out_count);
AE3_LUA_EXPORT int ae3_original_stage_spatial_record_copy(
    Ae3OriginalManagerInitSession* session,
    uint32_t original_record_index,
    Ae3OriginalStageSpatialRecordV1* output);

/* Full source 0x30FFD0 actor current-polygon resolution using an
 * already mounted ORIGINAL 0x21D46C stage. Source original a1 is
 * incoming_polygon_index; UINT32_MAX represents original NULL.
 *
 * The real owner provides the original VU0 query vector loaded from
 * its actor and original 0x4E3C14 grid operands on nearest fallback.
 * grid may be NULL only if the source 0x30FFD0 branch never needs
 * nearest. The original 0x21B788 uses source 0.1f margin for direct
 * incoming/current checks; 0x21D898 retains its source-zero margin.
 * The caller must atomically publish result.original_current_index
 * to the REAL actor+0x30 on success, and clear original owner+0x50
 * at its recovered 0x57C620 point. This API never creates an Actor.
 */
typedef struct Ae3OriginalActorPolygonSelectSourceV1 {
    uint32_t original_current_index;
    uint32_t original_incoming_index;
    Ae3OriginalVec4 original_actor_query;
    const Ae3OriginalStageSpatialGridSourceV1* original_grid;
} Ae3OriginalActorPolygonSelectSourceV1;
typedef struct Ae3OriginalActorPolygonSelectResultV1 {
    uint32_t original_current_index; /* UINT32_MAX source NULL */
} Ae3OriginalActorPolygonSelectResultV1;
AE3_LUA_EXPORT int ae3_original_actor_polygon_select_30ffd0(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorPolygonSelectSourceV1* source,
    Ae3OriginalActorPolygonSelectResultV1* output);

/* Legacy LOW64-only view of original 0x57C8EC target dispatch.
 * Genuine 0x21B788 eligibility, 0x57C98C three-vertex random blend,
 * persistent 0x36F3C0 RNG, and 0x57CA68 fallback VU0 transformations
 * are recovered in the full-width V2 implementation below.
 * The V1 ABI cannot carry genuine 128-bit QMFC2 source output and
 * rejects results with a nonzero upper half; use V2 for actor steering.
 * All point counts/0x70 records must come from the original stage
 * 0x1E descriptors and 0x21D46C loader, never from Unreal geometry.
 */
typedef struct Ae3OriginalActorTargetRequestV1 {
    const void* original_record70;
    uint16_t original_point_count;
    uint16_t reserved;
    int original_variant30;
    Ae3OriginalVec4 original_controller20;
} Ae3OriginalActorTargetRequestV1;
typedef struct Ae3OriginalActorTargetResultV1 {
    uint64_t original_low64;
    int original_branch; /* 1 = passthrough, 2 = triangle random. */
} Ae3OriginalActorTargetResultV1;
AE3_LUA_EXPORT int ae3_original_actor_target_57c8ec(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV1* source,
    Ae3OriginalActorTargetResultV1* out);
/* The first source target API is low64-only. Original 0x57C8EC's real
 * QMFC2 result is 128 bits: V1 will now reject any nonzero upper half
 * instead of silently corrupting actor steering. Prefer V2 below.
 * The source 0x57CA68 fixed VU0 candidate transform now exists.
 */
typedef struct Ae3OriginalActorTargetRequestV2 {
    Ae3OriginalActorTargetRequestV1 original;
    Ae3OriginalVec4 original_owner14_vector40;
    uint32_t original_q_bits;
    uint32_t original_vu0_r_bits;
    int original_fallback_operands_available;
} Ae3OriginalActorTargetRequestV2;
typedef struct Ae3OriginalActorTargetResultV2 {
    uint64_t original_low64;
    uint64_t original_high64;
    int original_branch; /* 1 = pass-through, 2 = triangle, 3 = fallback */
    uint32_t original_vu0_r_bits;
} Ae3OriginalActorTargetResultV2;
AE3_LUA_EXPORT int ae3_original_actor_target_57c8ec_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalActorTargetRequestV2* source,
    Ae3OriginalActorTargetResultV2* out);

/* Fully recovered original polygon projection chain:
 * 0x4C1B3C ray preparation -> 0x21BD3C traversal -> 0x21EE00 edge clip.
 * Original stage loader must provide 0x70-byte decoded records AND their
 * genuine navigation adjacency. No invented graph or Unreal raycast.
 * A projected_polygon of UINT32_MAX means no accepted source polygon.
 */
typedef struct Ae3OriginalPolygonRaySourceV1 {
    const void* original_records70;
    size_t original_record_count;
    const uint8_t* original_degree;
    const uint32_t* original_neighbor3;
    const uint16_t* original_action_kind3;
    uint32_t original_start_polygon;
    Ae3OriginalVec4 original_ray_start;
    Ae3OriginalVec4 original_ray_end;
    uint32_t original_max_recursions;
} Ae3OriginalPolygonRaySourceV1;
typedef struct Ae3OriginalPolygonRayResultV1 {
    uint32_t last_original_polygon;
    uint32_t original_polygons_tested;
    uint32_t original_edge_tests;
    uint32_t original_zero_kind_recursions;
    int original_hit_point_available;
    Ae3OriginalVec4 original_hit_point;
} Ae3OriginalPolygonRayResultV1;
/* Source raycast through exact original math, no host-supplied result.
 * Shares initializer session error/lifetime guard. Output untouched on
 * failure. Valid source polygon IDs and 1..4096 recursion budget required.
 */
AE3_LUA_EXPORT int ae3_original_polygon_project_4c1b3c(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalPolygonRaySourceV1* original,
    Ae3OriginalPolygonRayResultV1* output);

/* Source recovered 0x57C7CC original route traversal. This is the real
 * source actor's route control flow, not a newly invented path follower.
 * Polygon IDs and 0x70-byte records belong to the source-decoded stage.
 * UINT32_MAX is the original missing polygon sentinel at the host ABI.
 * Calls to unavailable original 0x4C1B3C/0x57C8EC owner operations reject.
 */
typedef struct Ae3OriginalRouteSourceV1 {
    const void* original_records70;
    size_t original_record_count;
    uint32_t actor_polygon_index;
    uint32_t active_target_polygon_index;
    const uint32_t* original_route_indices;
    size_t original_route_count;
    int original_has_multiple_polygons1c;
} Ae3OriginalRouteSourceV1;
typedef struct Ae3OriginalRouteCallsV1 {
    void* user;
    /* Genuine original projection must return a valid source polygon ID
     * or UINT32_MAX for no hit; returning zero rejects the original call.
     */
    int (*original_project_4c1b3c)(void* user,
        uint32_t source_actor_polygon, uint32_t* projected_polygon);
    /* Genuine original 0x57C8EC target update, 1 only after performed. */
    int (*original_apply_target_57c8ec)(void* user,
        uint32_t target_polygon);
} Ae3OriginalRouteCallsV1;
typedef struct Ae3OriginalRouteResultV1 {
    uint32_t active_target_polygon_index;
    uint32_t original_targets_applied;
    uint32_t original_projections_performed;
    uint32_t original_following_indices;
    int original_early_current_target_match;
} Ae3OriginalRouteResultV1;
/* Uses the same native session guard as source initializer, forbidding
 * reentrant route operations and protecting real game manager ownership.
 * On success, output reflects exact original 0x57C7CC branch behavior.
 */
AE3_LUA_EXPORT int ae3_original_actor_route_step(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV1* original,
    const Ae3OriginalRouteCallsV1* genuine_manager,
    Ae3OriginalRouteResultV1* output);

/* Legacy route v2 executes original 0x4C1B3C / 0x21BD3C /
 * 0x21EE00 projection, but its external target callback exposes LOW64
 * only. Source 0x57C894 uses the target's HIGH64 for follow-up rays,
 * so this ABI rejects routes requiring that unavailable information.
 * Use the full-width, transactional v3 for those routes.
 */
typedef struct Ae3OriginalRouteSourceV2 {
    Ae3OriginalRouteSourceV1 original;
    const uint8_t* original_degree;
    const uint32_t* original_neighbor3;
    const uint16_t* original_action_kind3;
    Ae3OriginalVec4 original_incoming_a1;
    Ae3OriginalVec4 original_controller40;
    uint32_t original_max_recursions;
} Ae3OriginalRouteSourceV2;
typedef struct Ae3OriginalRouteCallsV2 {
    void* user;
    int (*original_apply_target_57c8ec)(
        void* user, uint32_t original_polygon,
        uint64_t* original_target_low64);
} Ae3OriginalRouteCallsV2;
AE3_LUA_EXPORT int ae3_original_actor_route_step_v2(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV2* original,
    const Ae3OriginalRouteCallsV2* actual_target_manager,
    Ae3OriginalRouteResultV1* output);

/* Version 3: source-derived full 128-bit target vectors from the
 * native 0x57C8EC dispatch, including original 0x21B788 eligibility,
 * 0x57C98C game RNG and recovered 0x57CA68 fallback VU0 math.
 * The fallback requires genuine actor owner vector and Q/R operands;
 * missing original inputs reject rather than using invented steering.
 * point_counts[] MUST match original 0x1E-byte stage descriptors.
 * The invocation snapshots source records, counts and route indices
 * before calling the external manager's Begin callback, then atomically
 * commits target low64, high64 and VU0 R with original RNG advancement.
 * On failure, the manager must Abort; result and shared RNG stay unchanged.
 */
typedef struct Ae3OriginalRouteSourceV3 {
    Ae3OriginalRouteSourceV2 original;
    const uint16_t* original_polygon_point_counts;
    Ae3OriginalVec4 original_controller20;
    int original_controller_variant30;
    Ae3OriginalVec4 original_owner14_vector40;
    uint32_t original_q_bits;
    uint32_t original_vu0_r_bits;
    int original_fallback_operands_available;
} Ae3OriginalRouteSourceV3;
typedef struct Ae3OriginalRouteCallsV3 {
    void* user;
    /* BEGIN, stage individual updates, COMMIT or ABORT as one actual
     * actor-owner transaction. Manager must not expose partial updates.
     * Missing transaction callbacks reject original actor route.
     */
    int (*begin_original_actor_update)(void* user);
    int (*stage_original_target_57c8ec)(
        void* user, uint32_t original_polygon,
        uint64_t original_target_low64,
        uint64_t original_target_high64,
        uint32_t original_vu0_r_bits);
    int (*commit_original_actor_update)(void* user);
    void (*abort_original_actor_update)(void* user);
} Ae3OriginalRouteCallsV3;
AE3_LUA_EXPORT int ae3_original_actor_route_step_v3(
    Ae3OriginalManagerInitSession* session,
    const Ae3OriginalRouteSourceV3* source,
    const Ae3OriginalRouteCallsV3* real_actor_manager,
    Ae3OriginalRouteResultV1* result);




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
