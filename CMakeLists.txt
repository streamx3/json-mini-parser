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

# Tests that run identically against both fixtures.
# cfg_thresh1_type accepts INT or FLOAT internally, so it belongs here too.
set(SHARED_TESTS
    valid
    id_type             name_type           active_type
    rating_type         address_type
    tags_type           tags0_type
    config_type         cfg_interval_type   cfg_debug_type
    cfg_thresh_type     cfg_thresh0_type    cfg_thresh1_type    cfg_thresh2_type
    nodes_type          nodes0_type
    nodes0_uid_type     nodes0_label_type   nodes0_online_type
    id_int_val          cfg_interval_int_val    nodes0_uid_int_val
    id_val              name_val            active_val          rating_val
    tags0_val           tags1_val           tags2_val
    cfg_interval_val    cfg_debug_val
    cfg_thresh0_val     cfg_thresh1_val     cfg_thresh2_val
    nodes0_uid_val      nodes0_label_val    nodes0_online_val
)

foreach(F raw1 raw2)
    foreach(T ${SHARED_TESTS})
        add_test(NAME ${F}.${T} COMMAND json-tests ${F} ${T})
    endforeach()
endforeach()

# raw2 only: bare 3 is INT, so get_int succeeds; raw1 has 3.0 which get_int rejects
add_test(NAME raw2.cfg_thresh1_int_val COMMAND json-tests raw2 cfg_thresh1_int_val)

# Edge-case tests need no fixture
foreach(T missing_path missing_nested missing_oob
          invalid_json invalid_unterminated
          int_rejects_float str_truncation null_data)
    add_test(NAME ${T} COMMAND json-tests ${T})
endforeach()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
