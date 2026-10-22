# Integrator-reviewed assets/Lua additions beyond the original split manifest.
# These are actual checkout targets, never source-equivalent mirrors.
if(AE3_PARALLEL_ASSETS_READY)
    foreach(component IN ITEMS
        directory_order named_lookup record_registration registration_filter
        stage_catalog stage_scope stage_sequence vfi_folder_owner)
        ae3_parallel_test(Getchu3Resource_${component}Tests
            tests/original_resource_${component}_test.cpp
            OriginalResource_${component})
    endforeach()
    add_executable(Getchu3OriginalStageResourceProbe src/original_resource_stage_probe.cpp)
    target_link_libraries(Getchu3OriginalStageResourceProbe PRIVATE Getchu3Assets)

    set(AE3_RETAIL_USA_ISO "" CACHE FILEPATH "PRIVATE verified original USA ISO or DATA.BIN")
    set(AE3_RETAIL_WOODS_C_FOLDER "" CACHE PATH "PRIVATE extracted original woods_c folder")
    set(AE3_RETAIL_LUA_CORPUS "" CACHE PATH "PRIVATE original 168-stage Lua folder")
    if(AE3_RETAIL_LUA_CORPUS AND NOT AE3_RETAIL_USA_ISO)
        message(FATAL_ERROR "AE3_RETAIL_LUA_CORPUS also requires AE3_RETAIL_USA_ISO; corpus checks must not disappear")
    endif()
    foreach(private_path IN ITEMS AE3_RETAIL_USA_ISO AE3_RETAIL_WOODS_C_FOLDER AE3_RETAIL_LUA_CORPUS)
        if(${private_path} AND NOT EXISTS "${${private_path}}")
            message(FATAL_ERROR "${private_path} refers to an absent owner-private input")
        endif()
    endforeach()
    if(AE3_RETAIL_USA_ISO)
        ae3_parallel_test(Getchu3ResourceStageScopeRetailTests
            tests/original_resource_stage_scope_retail_test.cpp
            OriginalResourceStageScopePrivateISO "${AE3_RETAIL_USA_ISO}")
        if(AE3_RETAIL_LUA_CORPUS)
            ae3_parallel_test(Getchu3OriginalLuaCorpusTests
                tests/original_stage_lua_corpus_retail_test.cpp
                OriginalStageLua168PrivateISOAndChunks
                "${AE3_RETAIL_USA_ISO}" "${AE3_RETAIL_LUA_CORPUS}" --validate-only)
            add_test(NAME OriginalStageLua168ExecutionRequiresOriginalManagers
                COMMAND Getchu3OriginalLuaCorpusTests
                "${AE3_RETAIL_USA_ISO}" "${AE3_RETAIL_LUA_CORPUS}")
            set_tests_properties(OriginalStageLua168ExecutionRequiresOriginalManagers
                PROPERTIES SKIP_RETURN_CODE 77)
        endif()
    endif()
    if(AE3_RETAIL_WOODS_C_FOLDER)
        ae3_parallel_test(Getchu3ResourceRecordRetailTests
            tests/original_resource_record_registration_retail_test.cpp
            OriginalResourceRecordPrivateWoodsC "${AE3_RETAIL_WOODS_C_FOLDER}")
        ae3_parallel_test(Getchu3PckStageRetailTests
            tests/original_pck_stage_retail_test.cpp
            OriginalPckFivePrivateWoodsCPackages "${AE3_RETAIL_WOODS_C_FOLDER}")
    endif()

    if(Python3_Interpreter_FOUND)
        if(AE3_RETAIL_USA_ISO)
            add_test(NAME OriginalStageRegistryToLuaPrivateISO
                COMMAND "${Python3_EXECUTABLE}"
                "${PROJECT_SOURCE_DIR}/tools/test_original_stage_registry_lua.py"
                "$<TARGET_FILE:Getchu3RetailStageAudit>" "${AE3_RETAIL_USA_ISO}")
        endif()
        if(AE3_RETAIL_USA_ELF AND AE3_RETAIL_WOODS_C_FOLDER)
            foreach(audit IN ITEMS resource_record_registration stage_resource_initialization)
                add_test(NAME OriginalSourceAudit_${audit}
                    COMMAND "${Python3_EXECUTABLE}"
                    "${PROJECT_SOURCE_DIR}/tools/verify_original_${audit}.py"
                    "${AE3_RETAIL_USA_ELF}" "${AE3_RETAIL_WOODS_C_FOLDER}")
            endforeach()
        endif()
        add_test(NAME OriginalLuaAuditToolRegressions
            COMMAND "${Python3_EXECUTABLE}" -m unittest
            test_audit_original_lua50_calls test_map_original_lua_registrations
            test_reconcile_original_lua_bindings)
        set_tests_properties(OriginalLuaAuditToolRegressions PROPERTIES
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tools")
    endif()
endif()

# Release-mode focused tests must execute their existing assertions.
ae3_parallel_test(Getchu3OriginalManagerInitializerTests
    tests/original_manager_initializer_test.cpp OriginalManagerInitializerSourceControlFlow)
if(AE3_RETAIL_WOODS_C_NAVIGATION_PCK)
    add_test(NAME OriginalManagerInitializerPrivate15TypeTwoActions
        COMMAND Getchu3OriginalManagerInitializerTests "${AE3_RETAIL_WOODS_C_NAVIGATION_PCK}")
endif()
if(Python3_Interpreter_FOUND AND AE3_RETAIL_USA_ELF)
    add_test(NAME OriginalManagerInitializerPrivateOpcodeProof
        COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tools/verify_original_manager_initializer.py"
        "${AE3_RETAIL_USA_ELF}")
endif()
target_sources(Getchu3Assets PRIVATE src/original_manager_initializer.cpp)
if(MSVC)
    set_source_files_properties(src/original_manager_initializer.cpp
        src/original_actor_random_triangle.cpp src/original_game_random.cpp
        PROPERTIES COMPILE_OPTIONS /fp:strict)
else()
    set_source_files_properties(src/original_manager_initializer.cpp
        src/original_actor_random_triangle.cpp src/original_game_random.cpp
        PROPERTIES COMPILE_OPTIONS "-ffp-contract=off;-fno-fast-math")
endif()
get_property(native_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(native_target IN LISTS native_targets)
    if(native_target MATCHES "Tests$")
        if(MSVC)
            target_compile_options(${native_target} PRIVATE /UNDEBUG)
        else()
            target_compile_options(${native_target} PRIVATE -UNDEBUG)
        endif()
    endif()
endforeach()
