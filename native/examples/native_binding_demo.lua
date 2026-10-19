-- Standalone game-free AE3-style script to verify actual Lua 5.0 -> C++
-- bindings. This is NOT a recovered Sony game script or a game level.
script_description = "Getchu 3 native Lua 5.0 binding demonstration"

lua_param({ namespace = "stage_common", stage_id = "development" })
lua_param({ namespace = "props", piece_type = "CUBIC", piece_num = 9 })

function setupArea(opts)
    new_ent("Player", { name = "player", tag_name = opts.spawn_tag })
    new_ent("Monkey", { name = "ape_test_01", tag_name = "ape_01", speed = 4.5 })
    new_ent("Breakable", {
        name = "crate", model_name = "wbox_l", flg_save = false })
    new_ent("Controller", { name = "crate_sequencer" }, c_order(1,
        c_ctrl("trig_wait", {name="crate", flag=true}),
        c_ctrl("wait", {time=1.0}),
        c_ctrl("se", {name="st_brk_woodbox",target_name="crate"})
    ))
    gw_set_flag("stage_initialized", true)
    assert(gw_get_flag("stage_initialized"))
end
