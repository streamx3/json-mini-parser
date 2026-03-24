/**
 * tests.c — CTest-compatible test runner for json-mini-parser.
 *
 * Usage: json-tests <test-name>
 *   Returns 0 (PASS) or 1 (FAIL).
 *
 * CMakeLists registers each name individually with add_test() so that
 * ctest can run, filter, and report them independently.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "json-mini-parser.h"

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

/* Pretty-printed variant (raw1) */
static const char raw1[] =
    "{"
    "  \"id\": 42,"
    "  \"name\": \"Acme Corp\","
    "  \"active\": true,"
    "  \"rating\": 4.7,"
    "  \"address\": null,"
    "  \"tags\": [\"embedded\", \"iot\", \"sensor\"],"
    "  \"config\": {"
    "    \"interval_ms\": 500,"
    "    \"debug\": false,"
    "    \"thresholds\": [1.5, 3.0, 7.2]"
    "  },"
    "  \"nodes\": ["
    "    {\"uid\": 1, \"label\": \"alpha\", \"online\": true},"
    "    {\"uid\": 2, \"label\": \"bravo\", \"online\": false}"
    "  ]"
    "}";

/* Minified variant (raw2).
 * NOTE: thresholds[1] is written as 3 (integer) not 3.0, so
 * get_type_by_path returns JSON_TYPE_INT for that element in raw2. */
static const char raw2[] =
    "{\"id\":42,\"name\":\"Acme Corp\",\"active\":true,\"rating\":4.7,"
    "\"address\":null,\"tags\":[\"embedded\",\"iot\",\"sensor\"],"
    "\"config\":{\"interval_ms\":500,\"debug\":false,\"thresholds\":[1.5,3,7.2]},"
    "\"nodes\":[{\"uid\":1,\"label\":\"alpha\",\"online\":true},"
    "{\"uid\":2,\"label\":\"bravo\",\"online\":false}]}";

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

#define PASS 0
#define FAIL 1

static int feq(float a, float b) { return fabsf(a - b) < 0.001f; }

static jdata_t jd(const char *s)
{
    jdata_t d = { (char *)s, strlen(s) };
    return d;
}

/* ------------------------------------------------------------------ */
/* Test-definition macros                                              */
/* ------------------------------------------------------------------ */

#define DEF_VALID(name, raw) \
static int t_##name(void) { \
    jdata_t d = jd(raw); \
    return json_valid(&d) == 0 ? PASS : FAIL; \
}

#define DEF_TYPE(name, raw, path, expected) \
static int t_##name(void) { \
    jdata_t d = jd(raw); \
    return get_type_by_path(&d, path) == (expected) ? PASS : FAIL; \
}

#define DEF_STR(name, raw, path, expected) \
static int t_##name(void) { \
    jdata_t d = jd(raw); \
    char buf[128]; \
    if (get_string_by_path(&d, path, buf, sizeof(buf)) != 0) return FAIL; \
    return strcmp(buf, (expected)) == 0 ? PASS : FAIL; \
}

#define DEF_FLOAT(name, raw, path, expected) \
static int t_##name(void) { \
    jdata_t d = jd(raw); \
    float v; \
    if (get_float_by_path(&d, path, &v) != 0) return FAIL; \
    return feq(v, (expected)) ? PASS : FAIL; \
}

#define DEF_BOOL(name, raw, path, expected) \
static int t_##name(void) { \
    jdata_t d = jd(raw); \
    bool v; \
    if (get_bool_by_path(&d, path, &v) != 0) return FAIL; \
    return v == (expected) ? PASS : FAIL; \
}

/* ------------------------------------------------------------------ */
/* Tests — raw1 (pretty-printed)                                      */
/* ------------------------------------------------------------------ */

DEF_VALID(r1_valid, raw1)

/* types */
DEF_TYPE(r1_id_type,            raw1, "/id",                   JSON_TYPE_INT)
DEF_TYPE(r1_name_type,          raw1, "/name",                 JSON_TYPE_STRING)
DEF_TYPE(r1_active_type,        raw1, "/active",               JSON_TYPE_BOOL)
DEF_TYPE(r1_rating_type,        raw1, "/rating",               JSON_TYPE_FLOAT)
DEF_TYPE(r1_address_type,       raw1, "/address",              JSON_TYPE_NULL)
DEF_TYPE(r1_tags_type,          raw1, "/tags",                 JSON_TYPE_ARRAY)
DEF_TYPE(r1_tags0_type,         raw1, "/tags[0]",              JSON_TYPE_STRING)
DEF_TYPE(r1_config_type,        raw1, "/config",               JSON_TYPE_OBJECT)
DEF_TYPE(r1_cfg_interval_type,  raw1, "/config/interval_ms",  JSON_TYPE_INT)
DEF_TYPE(r1_cfg_debug_type,     raw1, "/config/debug",        JSON_TYPE_BOOL)
DEF_TYPE(r1_cfg_thresh_type,    raw1, "/config/thresholds",   JSON_TYPE_ARRAY)
DEF_TYPE(r1_cfg_thresh0_type,   raw1, "/config/thresholds[0]",JSON_TYPE_FLOAT)
DEF_TYPE(r1_cfg_thresh1_type,   raw1, "/config/thresholds[1]",JSON_TYPE_FLOAT)
DEF_TYPE(r1_cfg_thresh2_type,   raw1, "/config/thresholds[2]",JSON_TYPE_FLOAT)
DEF_TYPE(r1_nodes_type,         raw1, "/nodes",               JSON_TYPE_ARRAY)
DEF_TYPE(r1_nodes0_type,        raw1, "/nodes[0]",            JSON_TYPE_OBJECT)
DEF_TYPE(r1_nodes0_uid_type,    raw1, "/nodes[0]/uid",        JSON_TYPE_INT)
DEF_TYPE(r1_nodes0_label_type,  raw1, "/nodes[0]/label",      JSON_TYPE_STRING)
DEF_TYPE(r1_nodes0_online_type, raw1, "/nodes[0]/online",     JSON_TYPE_BOOL)

/* values */
DEF_FLOAT(r1_id_val,            raw1, "/id",                   42.0f)
DEF_STR  (r1_name_val,          raw1, "/name",                 "Acme Corp")
DEF_BOOL (r1_active_val,        raw1, "/active",               true)
DEF_FLOAT(r1_rating_val,        raw1, "/rating",               4.7f)
DEF_STR  (r1_tags0_val,         raw1, "/tags[0]",              "embedded")
DEF_STR  (r1_tags1_val,         raw1, "/tags[1]",              "iot")
DEF_STR  (r1_tags2_val,         raw1, "/tags[2]",              "sensor")
DEF_FLOAT(r1_cfg_interval_val,  raw1, "/config/interval_ms",  500.0f)
DEF_BOOL (r1_cfg_debug_val,     raw1, "/config/debug",        false)
DEF_FLOAT(r1_cfg_thresh0_val,   raw1, "/config/thresholds[0]",1.5f)
DEF_FLOAT(r1_cfg_thresh1_val,   raw1, "/config/thresholds[1]",3.0f)
DEF_FLOAT(r1_cfg_thresh2_val,   raw1, "/config/thresholds[2]",7.2f)
DEF_FLOAT(r1_nodes0_uid_val,    raw1, "/nodes[0]/uid",        1.0f)
DEF_STR  (r1_nodes0_label_val,  raw1, "/nodes[0]/label",      "alpha")
DEF_BOOL (r1_nodes0_online_val, raw1, "/nodes[0]/online",     true)

/* ------------------------------------------------------------------ */
/* Tests — raw2 (minified)                                            */
/* ------------------------------------------------------------------ */

DEF_VALID(r2_valid, raw2)

/* types */
DEF_TYPE(r2_id_type,            raw2, "/id",                   JSON_TYPE_INT)
DEF_TYPE(r2_name_type,          raw2, "/name",                 JSON_TYPE_STRING)
DEF_TYPE(r2_active_type,        raw2, "/active",               JSON_TYPE_BOOL)
DEF_TYPE(r2_rating_type,        raw2, "/rating",               JSON_TYPE_FLOAT)
DEF_TYPE(r2_address_type,       raw2, "/address",              JSON_TYPE_NULL)
DEF_TYPE(r2_tags_type,          raw2, "/tags",                 JSON_TYPE_ARRAY)
DEF_TYPE(r2_tags0_type,         raw2, "/tags[0]",              JSON_TYPE_STRING)
DEF_TYPE(r2_config_type,        raw2, "/config",               JSON_TYPE_OBJECT)
DEF_TYPE(r2_cfg_interval_type,  raw2, "/config/interval_ms",  JSON_TYPE_INT)
DEF_TYPE(r2_cfg_debug_type,     raw2, "/config/debug",        JSON_TYPE_BOOL)
DEF_TYPE(r2_cfg_thresh_type,    raw2, "/config/thresholds",   JSON_TYPE_ARRAY)
DEF_TYPE(r2_cfg_thresh0_type,   raw2, "/config/thresholds[0]",JSON_TYPE_FLOAT)
/* raw2 has bare 3 (not 3.0), so the parser correctly identifies it as INT */
DEF_TYPE(r2_cfg_thresh1_type,   raw2, "/config/thresholds[1]",JSON_TYPE_INT)
DEF_TYPE(r2_cfg_thresh2_type,   raw2, "/config/thresholds[2]",JSON_TYPE_FLOAT)
DEF_TYPE(r2_nodes_type,         raw2, "/nodes",               JSON_TYPE_ARRAY)
DEF_TYPE(r2_nodes0_type,        raw2, "/nodes[0]",            JSON_TYPE_OBJECT)
DEF_TYPE(r2_nodes0_uid_type,    raw2, "/nodes[0]/uid",        JSON_TYPE_INT)
DEF_TYPE(r2_nodes0_label_type,  raw2, "/nodes[0]/label",      JSON_TYPE_STRING)
DEF_TYPE(r2_nodes0_online_type, raw2, "/nodes[0]/online",     JSON_TYPE_BOOL)

/* values */
DEF_FLOAT(r2_id_val,            raw2, "/id",                   42.0f)
DEF_STR  (r2_name_val,          raw2, "/name",                 "Acme Corp")
DEF_BOOL (r2_active_val,        raw2, "/active",               true)
DEF_FLOAT(r2_rating_val,        raw2, "/rating",               4.7f)
DEF_STR  (r2_tags0_val,         raw2, "/tags[0]",              "embedded")
DEF_STR  (r2_tags1_val,         raw2, "/tags[1]",              "iot")
DEF_STR  (r2_tags2_val,         raw2, "/tags[2]",              "sensor")
DEF_FLOAT(r2_cfg_interval_val,  raw2, "/config/interval_ms",  500.0f)
DEF_BOOL (r2_cfg_debug_val,     raw2, "/config/debug",        false)
DEF_FLOAT(r2_cfg_thresh0_val,   raw2, "/config/thresholds[0]",1.5f)
DEF_FLOAT(r2_cfg_thresh1_val,   raw2, "/config/thresholds[1]",3.0f)  /* INT value, float read */
DEF_FLOAT(r2_cfg_thresh2_val,   raw2, "/config/thresholds[2]",7.2f)
DEF_FLOAT(r2_nodes0_uid_val,    raw2, "/nodes[0]/uid",        1.0f)
DEF_STR  (r2_nodes0_label_val,  raw2, "/nodes[0]/label",      "alpha")
DEF_BOOL (r2_nodes0_online_val, raw2, "/nodes[0]/online",     true)

/* ------------------------------------------------------------------ */
/* Edge-case tests                                                     */
/* ------------------------------------------------------------------ */

/* Non-existent key must return MISSING, not INVALID */
static int t_missing_path(void)
{
    jdata_t d = jd(raw1);
    return get_type_by_path(&d, "/nonexistent") == JSON_TYPE_MISSING ? PASS : FAIL;
}

/* Nested missing key */
static int t_missing_nested(void)
{
    jdata_t d = jd(raw1);
    return get_type_by_path(&d, "/config/nosuchkey") == JSON_TYPE_MISSING ? PASS : FAIL;
}

/* Out-of-bounds array index */
static int t_missing_oob(void)
{
    jdata_t d = jd(raw1);
    return get_type_by_path(&d, "/tags[99]") == JSON_TYPE_MISSING ? PASS : FAIL;
}

/* Syntactically broken JSON must fail validation */
static int t_invalid_json(void)
{
    jdata_t d = jd("{bad json}");
    return json_valid(&d) != 0 ? PASS : FAIL;
}

/* Unterminated object must fail validation */
static int t_invalid_unterminated(void)
{
    jdata_t d = jd("{\"a\":1");
    return json_valid(&d) != 0 ? PASS : FAIL;
}

/* Output buffer too small for the string value → must return error */
static int t_str_truncation(void)
{
    jdata_t d = jd(raw1);
    char small[4]; /* "Acme Corp" won't fit */
    return get_string_by_path(&d, "/name", small, sizeof(small)) != 0 ? PASS : FAIL;
}

/* NULL jdata_t pointer must not crash */
static int t_null_data(void)
{
    return get_type_by_path(NULL, "/id") == JSON_TYPE_INVALID ? PASS : FAIL;
}

/* ------------------------------------------------------------------ */
/* Dispatch table                                                      */
/* ------------------------------------------------------------------ */

typedef int (*test_fn)(void);
typedef struct { const char *name; test_fn fn; } test_entry_t;

static const test_entry_t tests[] = {
    /* --- raw1 validity --- */
    { "r1_valid",              t_r1_valid              },
    /* --- raw1 types --- */
    { "r1_id_type",            t_r1_id_type            },
    { "r1_name_type",          t_r1_name_type          },
    { "r1_active_type",        t_r1_active_type        },
    { "r1_rating_type",        t_r1_rating_type        },
    { "r1_address_type",       t_r1_address_type       },
    { "r1_tags_type",          t_r1_tags_type          },
    { "r1_tags0_type",         t_r1_tags0_type         },
    { "r1_config_type",        t_r1_config_type        },
    { "r1_cfg_interval_type",  t_r1_cfg_interval_type  },
    { "r1_cfg_debug_type",     t_r1_cfg_debug_type     },
    { "r1_cfg_thresh_type",    t_r1_cfg_thresh_type    },
    { "r1_cfg_thresh0_type",   t_r1_cfg_thresh0_type   },
    { "r1_cfg_thresh1_type",   t_r1_cfg_thresh1_type   },
    { "r1_cfg_thresh2_type",   t_r1_cfg_thresh2_type   },
    { "r1_nodes_type",         t_r1_nodes_type         },
    { "r1_nodes0_type",        t_r1_nodes0_type        },
    { "r1_nodes0_uid_type",    t_r1_nodes0_uid_type    },
    { "r1_nodes0_label_type",  t_r1_nodes0_label_type  },
    { "r1_nodes0_online_type", t_r1_nodes0_online_type },
    /* --- raw1 values --- */
    { "r1_id_val",             t_r1_id_val             },
    { "r1_name_val",           t_r1_name_val           },
    { "r1_active_val",         t_r1_active_val         },
    { "r1_rating_val",         t_r1_rating_val         },
    { "r1_tags0_val",          t_r1_tags0_val          },
    { "r1_tags1_val",          t_r1_tags1_val          },
    { "r1_tags2_val",          t_r1_tags2_val          },
    { "r1_cfg_interval_val",   t_r1_cfg_interval_val   },
    { "r1_cfg_debug_val",      t_r1_cfg_debug_val      },
    { "r1_cfg_thresh0_val",    t_r1_cfg_thresh0_val    },
    { "r1_cfg_thresh1_val",    t_r1_cfg_thresh1_val    },
    { "r1_cfg_thresh2_val",    t_r1_cfg_thresh2_val    },
    { "r1_nodes0_uid_val",     t_r1_nodes0_uid_val     },
    { "r1_nodes0_label_val",   t_r1_nodes0_label_val   },
    { "r1_nodes0_online_val",  t_r1_nodes0_online_val  },
    /* --- raw2 validity --- */
    { "r2_valid",              t_r2_valid              },
    /* --- raw2 types --- */
    { "r2_id_type",            t_r2_id_type            },
    { "r2_name_type",          t_r2_name_type          },
    { "r2_active_type",        t_r2_active_type        },
    { "r2_rating_type",        t_r2_rating_type        },
    { "r2_address_type",       t_r2_address_type       },
    { "r2_tags_type",          t_r2_tags_type          },
    { "r2_tags0_type",         t_r2_tags0_type         },
    { "r2_config_type",        t_r2_config_type        },
    { "r2_cfg_interval_type",  t_r2_cfg_interval_type  },
    { "r2_cfg_debug_type",     t_r2_cfg_debug_type     },
    { "r2_cfg_thresh_type",    t_r2_cfg_thresh_type    },
    { "r2_cfg_thresh0_type",   t_r2_cfg_thresh0_type   },
    { "r2_cfg_thresh1_type",   t_r2_cfg_thresh1_type   },
    { "r2_cfg_thresh2_type",   t_r2_cfg_thresh2_type   },
    { "r2_nodes_type",         t_r2_nodes_type         },
    { "r2_nodes0_type",        t_r2_nodes0_type        },
    { "r2_nodes0_uid_type",    t_r2_nodes0_uid_type    },
    { "r2_nodes0_label_type",  t_r2_nodes0_label_type  },
    { "r2_nodes0_online_type", t_r2_nodes0_online_type },
    /* --- raw2 values --- */
    { "r2_id_val",             t_r2_id_val             },
    { "r2_name_val",           t_r2_name_val           },
    { "r2_active_val",         t_r2_active_val         },
    { "r2_rating_val",         t_r2_rating_val         },
    { "r2_tags0_val",          t_r2_tags0_val          },
    { "r2_tags1_val",          t_r2_tags1_val          },
    { "r2_tags2_val",          t_r2_tags2_val          },
    { "r2_cfg_interval_val",   t_r2_cfg_interval_val   },
    { "r2_cfg_debug_val",      t_r2_cfg_debug_val      },
    { "r2_cfg_thresh0_val",    t_r2_cfg_thresh0_val    },
    { "r2_cfg_thresh1_val",    t_r2_cfg_thresh1_val    },
    { "r2_cfg_thresh2_val",    t_r2_cfg_thresh2_val    },
    { "r2_nodes0_uid_val",     t_r2_nodes0_uid_val     },
    { "r2_nodes0_label_val",   t_r2_nodes0_label_val   },
    { "r2_nodes0_online_val",  t_r2_nodes0_online_val  },
    /* --- edge cases --- */
    { "missing_path",          t_missing_path          },
    { "missing_nested",        t_missing_nested        },
    { "missing_oob",           t_missing_oob           },
    { "invalid_json",          t_invalid_json          },
    { "invalid_unterminated",  t_invalid_unterminated  },
    { "str_truncation",        t_str_truncation        },
    { "null_data",             t_null_data             },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: json-tests <test-name>\navailable tests:\n");
        for (int i = 0; tests[i].name; i++)
            fprintf(stderr, "  %s\n", tests[i].name);
        return 1;
    }

    for (int i = 0; tests[i].name; i++) {
        if (strcmp(argv[1], tests[i].name) == 0)
            return tests[i].fn();
    }

    fprintf(stderr, "unknown test: %s\n", argv[1]);
    return 1;
}
