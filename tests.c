/**
 * tests.c — CTest-compatible test runner for json-mini-parser.
 *
 * Fixture tests:   json-tests <raw1|raw2> <test-name>
 * Edge-case tests: json-tests <test-name>
 *
 * CMakeLists registers each combination individually so that ctest
 * can run, filter, and report them independently.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "json-mini-parser.h"

/* ------------------------------------------------------------------ */
/* Fixtures                                                            */
/* ------------------------------------------------------------------ */

/* Pretty-printed */
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

/* Minified — identical content, but thresholds[1] is bare 3 (INT) not 3.0 */
static const char raw2[] =
    "{\"id\":42,\"name\":\"Acme Corp\",\"active\":true,\"rating\":4.7,"
    "\"address\":null,\"tags\":[\"embedded\",\"iot\",\"sensor\"],"
    "\"config\":{\"interval_ms\":500,\"debug\":false,\"thresholds\":[1.5,3,7.2]},"
    "\"nodes\":[{\"uid\":1,\"label\":\"alpha\",\"online\":true},"
    "{\"uid\":2,\"label\":\"bravo\",\"online\":false}]}";

static jdata_t make_fixture(const char *name)
{
    if (strcmp(name, "raw1") == 0) { jdata_t d = { (char *)raw1, sizeof(raw1) - 1 }; return d; }
    if (strcmp(name, "raw2") == 0) { jdata_t d = { (char *)raw2, sizeof(raw2) - 1 }; return d; }
    jdata_t d = { NULL, 0 };
    return d;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

#define PASS 0
#define FAIL 1

static int feq(float a, float b) { return fabsf(a - b) < 0.001f; }

/* ------------------------------------------------------------------ */
/* Fixture test definitions                                            */
/* ------------------------------------------------------------------ */

#define DEF_TYPE(name, path, expected) \
static int t_##name(jdata_t *d) { \
    return get_type_by_path(d, path) == (expected) ? PASS : FAIL; \
}

#define DEF_STR(name, path, expected) \
static int t_##name(jdata_t *d) { \
    char buf[128]; \
    if (get_string_by_path(d, path, buf, sizeof(buf)) != 0) return FAIL; \
    return strcmp(buf, (expected)) == 0 ? PASS : FAIL; \
}

#define DEF_FLOAT(name, path, expected) \
static int t_##name(jdata_t *d) { \
    float v; \
    if (get_float_by_path(d, path, &v) != 0) return FAIL; \
    return feq(v, (expected)) ? PASS : FAIL; \
}

#define DEF_INT(name, path, expected) \
static int t_##name(jdata_t *d) { \
    int32_t v; \
    if (get_int_by_path(d, path, &v) != 0) return FAIL; \
    return v == (int32_t)(expected) ? PASS : FAIL; \
}

#define DEF_BOOL(name, path, expected) \
static int t_##name(jdata_t *d) { \
    bool v; \
    if (get_bool_by_path(d, path, &v) != 0) return FAIL; \
    return v == (expected) ? PASS : FAIL; \
}

/* validity */
static int t_valid(jdata_t *d) { return json_valid(d) == 0 ? PASS : FAIL; }

/* types */
DEF_TYPE(id_type,              "/id",                   JSON_TYPE_INT)
DEF_TYPE(name_type,            "/name",                 JSON_TYPE_STRING)
DEF_TYPE(active_type,          "/active",               JSON_TYPE_BOOL)
DEF_TYPE(rating_type,          "/rating",               JSON_TYPE_FLOAT)
DEF_TYPE(address_type,         "/address",              JSON_TYPE_NULL)
DEF_TYPE(tags_type,            "/tags",                 JSON_TYPE_ARRAY)
DEF_TYPE(tags0_type,           "/tags[0]",              JSON_TYPE_STRING)
DEF_TYPE(config_type,          "/config",               JSON_TYPE_OBJECT)
DEF_TYPE(cfg_interval_type,    "/config/interval_ms",   JSON_TYPE_INT)
DEF_TYPE(cfg_debug_type,       "/config/debug",         JSON_TYPE_BOOL)
DEF_TYPE(cfg_thresh_type,      "/config/thresholds",    JSON_TYPE_ARRAY)
DEF_TYPE(cfg_thresh0_type,     "/config/thresholds[0]", JSON_TYPE_FLOAT)
/* thresholds[1] is 3.0 (FLOAT) in raw1 but bare 3 (INT) in raw2 — accept either */
static int t_cfg_thresh1_type(jdata_t *d) {
    JSON_TYPE_T t = get_type_by_path(d, "/config/thresholds[1]");
    return (t == JSON_TYPE_INT || t == JSON_TYPE_FLOAT) ? PASS : FAIL;
}
DEF_TYPE(cfg_thresh2_type,     "/config/thresholds[2]", JSON_TYPE_FLOAT)
DEF_TYPE(nodes_type,           "/nodes",                JSON_TYPE_ARRAY)
DEF_TYPE(nodes0_type,          "/nodes[0]",             JSON_TYPE_OBJECT)
DEF_TYPE(nodes0_uid_type,      "/nodes[0]/uid",         JSON_TYPE_INT)
DEF_TYPE(nodes0_label_type,    "/nodes[0]/label",       JSON_TYPE_STRING)
DEF_TYPE(nodes0_online_type,   "/nodes[0]/online",      JSON_TYPE_BOOL)

/* int values */
DEF_INT (id_int_val,           "/id",                   42)
DEF_INT (cfg_interval_int_val, "/config/interval_ms",   500)
DEF_INT (nodes0_uid_int_val,   "/nodes[0]/uid",         1)
/* Only registered for raw2: raw1 has 3.0 which get_int rejects */
DEF_INT (cfg_thresh1_int_val,  "/config/thresholds[1]", 3)

/* float and other values */
DEF_FLOAT(id_val,              "/id",                   42.0f)
DEF_STR  (name_val,            "/name",                 "Acme Corp")
DEF_BOOL (active_val,          "/active",               true)
DEF_FLOAT(rating_val,          "/rating",               4.7f)
DEF_STR  (tags0_val,           "/tags[0]",              "embedded")
DEF_STR  (tags1_val,           "/tags[1]",              "iot")
DEF_STR  (tags2_val,           "/tags[2]",              "sensor")
DEF_FLOAT(cfg_interval_val,    "/config/interval_ms",   500.0f)
DEF_BOOL (cfg_debug_val,       "/config/debug",         false)
DEF_FLOAT(cfg_thresh0_val,     "/config/thresholds[0]", 1.5f)
DEF_FLOAT(cfg_thresh1_val,     "/config/thresholds[1]", 3.0f)
DEF_FLOAT(cfg_thresh2_val,     "/config/thresholds[2]", 7.2f)
DEF_FLOAT(nodes0_uid_val,      "/nodes[0]/uid",         1.0f)
DEF_STR  (nodes0_label_val,    "/nodes[0]/label",       "alpha")
DEF_BOOL (nodes0_online_val,   "/nodes[0]/online",      true)

/* ------------------------------------------------------------------ */
/* Fixture test dispatch table                                         */
/* ------------------------------------------------------------------ */

typedef int (*fixture_fn)(jdata_t *);
typedef struct { const char *name; fixture_fn fn; } fx_entry_t;

static const fx_entry_t fx_tests[] = {
    { "valid",               t_valid               },
    { "id_type",             t_id_type             },
    { "name_type",           t_name_type           },
    { "active_type",         t_active_type         },
    { "rating_type",         t_rating_type         },
    { "address_type",        t_address_type        },
    { "tags_type",           t_tags_type           },
    { "tags0_type",          t_tags0_type          },
    { "config_type",         t_config_type         },
    { "cfg_interval_type",   t_cfg_interval_type   },
    { "cfg_debug_type",      t_cfg_debug_type      },
    { "cfg_thresh_type",     t_cfg_thresh_type     },
    { "cfg_thresh0_type",    t_cfg_thresh0_type    },
    { "cfg_thresh1_type",    t_cfg_thresh1_type    },
    { "cfg_thresh2_type",    t_cfg_thresh2_type    },
    { "nodes_type",          t_nodes_type          },
    { "nodes0_type",         t_nodes0_type         },
    { "nodes0_uid_type",     t_nodes0_uid_type     },
    { "nodes0_label_type",   t_nodes0_label_type   },
    { "nodes0_online_type",  t_nodes0_online_type  },
    { "id_int_val",          t_id_int_val          },
    { "cfg_interval_int_val",t_cfg_interval_int_val},
    { "nodes0_uid_int_val",  t_nodes0_uid_int_val  },
    { "cfg_thresh1_int_val", t_cfg_thresh1_int_val },
    { "id_val",              t_id_val              },
    { "name_val",            t_name_val            },
    { "active_val",          t_active_val          },
    { "rating_val",          t_rating_val          },
    { "tags0_val",           t_tags0_val           },
    { "tags1_val",           t_tags1_val           },
    { "tags2_val",           t_tags2_val           },
    { "cfg_interval_val",    t_cfg_interval_val    },
    { "cfg_debug_val",       t_cfg_debug_val       },
    { "cfg_thresh0_val",     t_cfg_thresh0_val     },
    { "cfg_thresh1_val",     t_cfg_thresh1_val     },
    { "cfg_thresh2_val",     t_cfg_thresh2_val     },
    { "nodes0_uid_val",      t_nodes0_uid_val      },
    { "nodes0_label_val",    t_nodes0_label_val    },
    { "nodes0_online_val",   t_nodes0_online_val   },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* Edge-case tests (no fixture)                                        */
/* ------------------------------------------------------------------ */

static int t_missing_path(void)
{
    jdata_t d = { (char *)raw1, sizeof(raw1) - 1 };
    return get_type_by_path(&d, "/nonexistent") == JSON_TYPE_MISSING ? PASS : FAIL;
}

static int t_missing_nested(void)
{
    jdata_t d = { (char *)raw1, sizeof(raw1) - 1 };
    return get_type_by_path(&d, "/config/nosuchkey") == JSON_TYPE_MISSING ? PASS : FAIL;
}

static int t_missing_oob(void)
{
    jdata_t d = { (char *)raw1, sizeof(raw1) - 1 };
    return get_type_by_path(&d, "/tags[99]") == JSON_TYPE_MISSING ? PASS : FAIL;
}

static int t_invalid_json(void)
{
    jdata_t d = { (char *)"{bad json}", 10 };
    return json_valid(&d) != 0 ? PASS : FAIL;
}

static int t_invalid_unterminated(void)
{
    jdata_t d = { (char *)"{\"a\":1", 6 };
    return json_valid(&d) != 0 ? PASS : FAIL;
}

static int t_int_rejects_float(void)
{
    jdata_t d = { (char *)raw1, sizeof(raw1) - 1 };
    int32_t v;
    return get_int_by_path(&d, "/rating", &v) != 0 ? PASS : FAIL;
}

static int t_str_truncation(void)
{
    jdata_t d = { (char *)raw1, sizeof(raw1) - 1 };
    char small[4];
    return get_string_by_path(&d, "/name", small, sizeof(small)) != 0 ? PASS : FAIL;
}

static int t_null_data(void)
{
    return get_type_by_path(NULL, "/id") == JSON_TYPE_INVALID ? PASS : FAIL;
}

/* ------------------------------------------------------------------ */
/* Edge-case dispatch table                                            */
/* ------------------------------------------------------------------ */

typedef int (*edge_fn)(void);
typedef struct { const char *name; edge_fn fn; } edge_entry_t;

static const edge_entry_t edge_tests[] = {
    { "missing_path",         t_missing_path         },
    { "missing_nested",       t_missing_nested       },
    { "missing_oob",          t_missing_oob          },
    { "invalid_json",         t_invalid_json         },
    { "invalid_unterminated", t_invalid_unterminated },
    { "int_rejects_float",    t_int_rejects_float    },
    { "str_truncation",       t_str_truncation       },
    { "null_data",            t_null_data            },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    if (argc == 3) {
        jdata_t d = make_fixture(argv[1]);
        if (!d.buf) { fprintf(stderr, "unknown fixture: %s\n", argv[1]); return 1; }
        for (int i = 0; fx_tests[i].name; i++)
            if (strcmp(argv[2], fx_tests[i].name) == 0)
                return fx_tests[i].fn(&d);
        fprintf(stderr, "unknown test: %s\n", argv[2]);
        return 1;
    }

    if (argc == 2) {
        for (int i = 0; edge_tests[i].name; i++)
            if (strcmp(argv[1], edge_tests[i].name) == 0)
                return edge_tests[i].fn();
        fprintf(stderr, "unknown edge test: %s\n", argv[1]);
        return 1;
    }

    fprintf(stderr, "usage:\n"
            "  json-tests <raw1|raw2> <test-name>   run a fixture test\n"
            "  json-tests <test-name>               run an edge-case test\n");
    return 1;
}
