# Register complete, reviewed native specialist source sets as they are merged.
# This is build integration only: no game asset, demo scene or emulation source.
include_guard(GLOBAL)

option(AE3_REQUIRE_PARALLEL_INTEGRATION
    "Reject native builds until all five new specialist source sets are merged" OFF)

function(ae3_optional_parallel_group group)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs FILES SOURCES)
    cmake_parse_arguments(AE3 "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(AE3_UNPARSED_ARGUMENTS OR AE3_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Invalid native integration manifest for ${group}")
    endif()
    set(missing "")
    set(found 0)
    foreach(rel IN LISTS AE3_FILES)
        if(EXISTS "${PROJECT_SOURCE_DIR}/${rel}")
            math(EXPR found "${found}+1")
        else()
            list(APPEND missing "${rel}")
        endif()
    endforeach()
    list(LENGTH AE3_FILES expected)
    if(found EQUAL 0)
        if(AE3_REQUIRE_PARALLEL_INTEGRATION)
            message(FATAL_ERROR "Required native ${group} workstream has not yet been merged")
        endif()
        set(AE3_PARALLEL_${group}_READY FALSE PARENT_SCOPE)
        message(STATUS "Parallel ${group}: not yet merged; base native sources retained")
        return()
    endif()
    if(NOT found EQUAL expected)
        list(JOIN missing ", " missing_text)
        message(FATAL_ERROR "Partial parallel ${group} integration (${found}/${expected}); missing: ${missing_text}. Apply complete reviewed subsystem changes.")
    endif()
    foreach(rel IN LISTS AE3_SOURCES)
        target_sources(Getchu3Assets PRIVATE "${PROJECT_SOURCE_DIR}/${rel}")
    endforeach()
    set(AE3_PARALLEL_${group}_READY TRUE PARENT_SCOPE)
    message(STATUS "Parallel ${group}: complete source/test file set (${found}/${expected})")
endfunction()

function(ae3_parallel_test target source name)
    add_executable(${target} "${PROJECT_SOURCE_DIR}/${source}")
    target_link_libraries(${target} PRIVATE Getchu3Assets)
    add_test(NAME ${name} COMMAND ${target} ${ARGN})
endfunction()

# The integration owner must merge each specialist's MODIFIED base files,
# not just these newly added modules. Presence is not verification of retail
# correctness; original data testing still requires owner-private resources.
ae3_optional_parallel_group(ASSETS
    FILES
      include/ae3/original_resource_package.hpp
      include/ae3/original_resource_vfi_mount.hpp
      include/ae3/original_resource_sz.hpp
      src/original_resource_sz.cpp
      tests/original_resource_package_test.cpp
      tests/original_resource_sz_test.cpp
      tests/original_resource_package_retail_test.cpp
    SOURCES src/original_resource_sz.cpp)
if(AE3_PARALLEL_ASSETS_READY)
    ae3_parallel_test(Getchu3OriginalResourcePackageTests
        tests/original_resource_package_test.cpp OriginalResourceTypedPckOwnership)
    ae3_parallel_test(Getchu3OriginalResourceSzTests
        tests/original_resource_sz_test.cpp OriginalResourceSZToNativePck)
    if(AE3_RETAIL_WOODS_C_EXDB_PCK AND EXISTS "${AE3_RETAIL_WOODS_C_EXDB_PCK}")
        ae3_parallel_test(Getchu3OriginalResourcePackageRetailTests
            tests/original_resource_package_retail_test.cpp
            OriginalResourcePrivatePackageBinary "${AE3_RETAIL_WOODS_C_EXDB_PCK}")
    endif()
endif()

ae3_optional_parallel_group(NAVIGATION
    FILES
      include/ae3/original_nav_stage_route.hpp
      include/ae3/original_nav_stage_groups.hpp
      src/original_nav_stage_route.cpp
      src/original_nav_stage_groups.cpp
      tests/original_nav_stage_route_test.cpp
      tests/original_nav_stage_groups_test.cpp
    SOURCES src/original_nav_stage_route.cpp src/original_nav_stage_groups.cpp)
if(AE3_PARALLEL_NAVIGATION_READY)
    ae3_parallel_test(Getchu3OriginalNavStageRouteTests
        tests/original_nav_stage_route_test.cpp OriginalNavStageRouteOwnerType1Type2)
    ae3_parallel_test(Getchu3OriginalNavStageGroupsTests
        tests/original_nav_stage_groups_test.cpp OriginalNavStageRangeAndRouteBindings)
    if(AE3_RETAIL_WOODS_C_NAVIGATION_PCK AND EXISTS "${AE3_RETAIL_WOODS_C_NAVIGATION_PCK}")
        add_test(NAME OriginalNavStageRouteRetailWoodsC
            COMMAND Getchu3OriginalNavStageRouteTests "${AE3_RETAIL_WOODS_C_NAVIGATION_PCK}")
    endif()
endif()

ae3_optional_parallel_group(VU0
    FILES
      include/ae3/original_actor_fallback_vu0_native.hpp
      src/original_actor_fallback_vu0_native.cpp
      tests/original_actor_fallback_vu0_native_test.cpp
    SOURCES src/original_actor_fallback_vu0_native.cpp)
if(AE3_PARALLEL_VU0_READY)
    ae3_parallel_test(Getchu3OriginalActorFallbackNativeVU0Tests
        tests/original_actor_fallback_vu0_native_test.cpp OriginalActorFallbackNativeVU0ExactSourceSequence)
endif()

ae3_optional_parallel_group(GRAPHICS
    FILES
      include/ae3/original_render_instances.hpp
      include/ae3/original_render_visibility.hpp
      include/ae3/original_render_selector.hpp
      src/original_render_instances.cpp
      src/original_render_visibility.cpp
      src/original_render_selector.cpp
      tests/original_render_instances_test.cpp
      tests/original_render_visibility_test.cpp
      tests/original_render_selector_test.cpp
    SOURCES src/original_render_instances.cpp src/original_render_visibility.cpp
            src/original_render_selector.cpp)
if(AE3_PARALLEL_GRAPHICS_READY)
    ae3_parallel_test(Getchu3OriginalRenderInstancesTests
        tests/original_render_instances_test.cpp OriginalRenderInstances)
    ae3_parallel_test(Getchu3OriginalRenderVisibilityTests
        tests/original_render_visibility_test.cpp OriginalRetailRenderEarlyGate)
    ae3_parallel_test(Getchu3OriginalRenderSelectorTests
        tests/original_render_selector_test.cpp OriginalRetailRenderSelectorAndDispatch)
endif()

ae3_optional_parallel_group(INPUT_AUDIO
    FILES
      include/ae3/original_pad_runtime.hpp
      include/ae3/original_audio_runtime.hpp
      src/original_pad_runtime.cpp
      src/original_pad_win32.cpp
      src/original_audio_runtime.cpp
      tests/original_input_audio_runtime_test.cpp
    SOURCES src/original_pad_runtime.cpp src/original_pad_win32.cpp
            src/original_audio_runtime.cpp)
if(AE3_PARALLEL_INPUT_AUDIO_READY)
    ae3_parallel_test(Getchu3OriginalInputAudioTests
        tests/original_input_audio_runtime_test.cpp OriginalInputAudioSourceBoundaries)
endif()
