cmake_minimum_required(VERSION 3.16)

project(json-mini-parser LANGUAGES C)

# ---- shared sources (no separate library target needed at this scale) ----
set(PARSER_SRC json-mini-parser.c json-mini-parser.h)

# ---- demo executable ----
add_executable(${PROJECT_NAME} main.c ${PARSER_SRC})

# ---- test executable ----
add_executable(json-tests tests.c ${PARSER_SRC})
# fabsf lives in libm on Linux; harmless on macOS (it's in libSystem)
target_link_libraries(json-tests PRIVATE m)

# ---- CTest ----
enable_testing()

set(ALL_TESTS
    # raw1 — validity
    r1_valid
    # raw1 — types
    r1_id_type            r1_name_type          r1_active_type
    r1_rating_type        r1_address_type
    r1_tags_type          r1_tags0_type
    r1_config_type        r1_cfg_interval_type  r1_cfg_debug_type
    r1_cfg_thresh_type    r1_cfg_thresh0_type   r1_cfg_thresh1_type   r1_cfg_thresh2_type
    r1_nodes_type         r1_nodes0_type
    r1_nodes0_uid_type    r1_nodes0_label_type  r1_nodes0_online_type
    # raw1 — values
    r1_id_val             r1_name_val           r1_active_val         r1_rating_val
    r1_tags0_val          r1_tags1_val          r1_tags2_val
    r1_cfg_interval_val   r1_cfg_debug_val
    r1_cfg_thresh0_val    r1_cfg_thresh1_val    r1_cfg_thresh2_val
    r1_nodes0_uid_val     r1_nodes0_label_val   r1_nodes0_online_val

    # raw2 — validity
    r2_valid
    # raw2 — types
    r2_id_type            r2_name_type          r2_active_type
    r2_rating_type        r2_address_type
    r2_tags_type          r2_tags0_type
    r2_config_type        r2_cfg_interval_type  r2_cfg_debug_type
    r2_cfg_thresh_type    r2_cfg_thresh0_type   r2_cfg_thresh1_type   r2_cfg_thresh2_type
    r2_nodes_type         r2_nodes0_type
    r2_nodes0_uid_type    r2_nodes0_label_type  r2_nodes0_online_type
    # raw2 — values
    r2_id_val             r2_name_val           r2_active_val         r2_rating_val
    r2_tags0_val          r2_tags1_val          r2_tags2_val
    r2_cfg_interval_val   r2_cfg_debug_val
    r2_cfg_thresh0_val    r2_cfg_thresh1_val    r2_cfg_thresh2_val
    r2_nodes0_uid_val     r2_nodes0_label_val   r2_nodes0_online_val

    # edge cases
    missing_path          missing_nested        missing_oob
    invalid_json          invalid_unterminated
    str_truncation        null_data
)

foreach(T ${ALL_TESTS})
    add_test(NAME ${T} COMMAND json-tests ${T})
endforeach()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
