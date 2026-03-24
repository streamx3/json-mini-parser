#include "json-mini-parser.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Internal helpers — all work directly on the buffer, no allocation  */
/* ------------------------------------------------------------------ */

static const char *skip_ws(const char *p, const char *end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'))
        p++;
    return p;
}

/* p must point to the opening '"'.
   Returns pointer one past the closing '"', or NULL on error. */
static const char *skip_str(const char *p, const char *end)
{
    if (p >= end || *p != '"') return NULL;
    p++;
    while (p < end) {
        if (*p == '\\') {
            p += 2; /* skip escape + the following byte */
            if (p > end) return NULL;
        } else if (*p == '"') {
            return p + 1;
        } else {
            p++;
        }
    }
    return NULL; /* unterminated string */
}

/* Forward declaration needed by skip_obj / skip_arr */
static const char *skip_val(const char *p, const char *end);

static const char *skip_obj(const char *p, const char *end)
{
    if (p >= end || *p != '{') return NULL;
    p++;
    p = skip_ws(p, end);
    if (p < end && *p == '}') return p + 1;

    while (p < end) {
        p = skip_ws(p, end);
        p = skip_str(p, end);         /* key   */
        if (!p) return NULL;
        p = skip_ws(p, end);
        if (p >= end || *p != ':') return NULL;
        p++;
        p = skip_val(p, end);         /* value */
        if (!p) return NULL;
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == '}') return p + 1;
        if (*p != ',') return NULL;
        p++;
    }
    return NULL;
}

static const char *skip_arr(const char *p, const char *end)
{
    if (p >= end || *p != '[') return NULL;
    p++;
    p = skip_ws(p, end);
    if (p < end && *p == ']') return p + 1;

    while (p < end) {
        p = skip_val(p, end);
        if (!p) return NULL;
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == ']') return p + 1;
        if (*p != ',') return NULL;
        p++;
    }
    return NULL;
}

static const char *skip_num(const char *p, const char *end)
{
    if (*p == '-') p++;
    if (p >= end || *p < '0' || *p > '9') return NULL;
    while (p < end && *p >= '0' && *p <= '9') p++;
    if (p < end && *p == '.') {
        p++;
        while (p < end && *p >= '0' && *p <= '9') p++;
    }
    if (p < end && (*p == 'e' || *p == 'E')) {
        p++;
        if (p < end && (*p == '+' || *p == '-')) p++;
        while (p < end && *p >= '0' && *p <= '9') p++;
    }
    return p;
}

static const char *skip_val(const char *p, const char *end)
{
    p = skip_ws(p, end);
    if (p >= end) return NULL;
    switch (*p) {
        case '"': return skip_str(p, end);
        case '{': return skip_obj(p, end);
        case '[': return skip_arr(p, end);
        case 't': return (p + 4 <= end && strncmp(p, "true",  4) == 0) ? p + 4 : NULL;
        case 'f': return (p + 5 <= end && strncmp(p, "false", 5) == 0) ? p + 5 : NULL;
        case 'n': return (p + 4 <= end && strncmp(p, "null",  4) == 0) ? p + 4 : NULL;
        default:
            if (*p == '-' || (*p >= '0' && *p <= '9')) return skip_num(p, end);
            return NULL;
    }
}

/* Find the value for 'key' (length klen) inside the object at p.
   Returns pointer to the value, or NULL if not found. */
static const char *obj_find(const char *p, const char *end,
                             const char *key, size_t klen)
{
    if (p >= end || *p != '{') return NULL;
    p++;
    p = skip_ws(p, end);
    if (p < end && *p == '}') return NULL; /* empty */

    while (p < end) {
        p = skip_ws(p, end);
        if (p >= end || *p != '"') return NULL;
        const char *ks = p + 1;
        p = skip_str(p, end);
        if (!p) return NULL;
        const char *ke = p - 1; /* closing '"' of the key */

        p = skip_ws(p, end);
        if (p >= end || *p != ':') return NULL;
        p++;
        p = skip_ws(p, end);

        if ((size_t)(ke - ks) == klen && strncmp(ks, key, klen) == 0)
            return p; /* p now points to the value */

        p = skip_val(p, end);
        if (!p) return NULL;
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == '}') return NULL; /* exhausted, not found */
        if (*p != ',') return NULL;
        p++;
    }
    return NULL;
}

/* Return pointer to element at index idx inside the array at p, or NULL. */
static const char *arr_find(const char *p, const char *end, int idx)
{
    if (p >= end || *p != '[') return NULL;
    p++;
    p = skip_ws(p, end);
    if (p < end && *p == ']') return NULL; /* empty */

    int i = 0;
    while (p < end) {
        p = skip_ws(p, end);
        if (i == idx) return p;
        p = skip_val(p, end);
        if (!p) return NULL;
        p = skip_ws(p, end);
        if (p >= end) return NULL;
        if (*p == ']') return NULL; /* out of bounds */
        if (*p != ',') return NULL;
        p++;
        i++;
    }
    return NULL;
}

/* Parse a non-negative integer from digits at p (no bounds check needed:
   caller already verified the characters are digits). */
static int parse_idx(const char *p)
{
    int v = 0;
    while (*p >= '0' && *p <= '9')
        v = v * 10 + (*p++ - '0');
    return v;
}

/*
 * Walk the JSON buffer following 'path'.
 * Path format: "/key"  "/key1/key2"  "/key[0]"  "/key[0]/subkey"
 * Returns pointer to the target value, or NULL.
 */
static const char *navigate(const char *json, size_t json_len, const char *path)
{
    const char *end = json + json_len;
    const char *cur = skip_ws(json, end);

    if (!path || path[0] == '\0') return cur;

    const char *p = path;
    while (*p) {
        if (*p != '/') return NULL;
        p++;

        /* Extract the key segment (stops at '/', '[', or end-of-path) */
        const char *seg = p;
        while (*p && *p != '/' && *p != '[') p++;
        size_t seg_len = (size_t)(p - seg);

        if (seg_len > 0) {
            cur = skip_ws(cur, end);
            cur = obj_find(cur, end, seg, seg_len);
            if (!cur) return NULL;
        }

        /* Handle one or more array subscripts: [0], [1][2], … */
        while (*p == '[') {
            p++; /* skip '[' */
            int idx = parse_idx(p);
            while (*p >= '0' && *p <= '9') p++; /* advance past digits */
            if (*p != ']') return NULL;
            p++; /* skip ']' */

            cur = skip_ws(cur, end);
            cur = arr_find(cur, end, idx);
            if (!cur) return NULL;
        }
    }

    return skip_ws(cur, end);
}

/* Return the type of the JSON value at p. */
static JSON_TYPE_T type_at(const char *p, const char *end)
{
    if (!p || p >= end) return JSON_TYPE_INVALID;
    switch (*p) {
        case '"': return JSON_TYPE_STRING;
        case '{': return JSON_TYPE_OBJECT;
        case '[': return JSON_TYPE_ARRAY;
        case 't': return JSON_TYPE_BOOL;
        case 'f': return JSON_TYPE_BOOL;
        case 'n': return JSON_TYPE_NULL;
        default:
            if (*p == '-' || (*p >= '0' && *p <= '9')) {
                const char *q = (*p == '-') ? p + 1 : p;
                while (q < end && *q >= '0' && *q <= '9') q++;
                if (q < end && (*q == '.' || *q == 'e' || *q == 'E'))
                    return JSON_TYPE_FLOAT;
                return JSON_TYPE_INT;
            }
            return JSON_TYPE_INVALID;
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int json_valid(jdata_t *data)
{
    if (!data || !data->buf || data->used_sz == 0) return -1;

    const char *end   = data->buf + data->used_sz;
    const char *after = skip_val(data->buf, end);
    if (!after) return -1;

    after = skip_ws(after, end);
    return (after == end) ? 0 : -1;
}

JSON_TYPE_T get_type_by_path(jdata_t *data, const char *path)
{
    if (!data || !data->buf) return JSON_TYPE_INVALID;

    const char *val = navigate(data->buf, data->used_sz, path);
    if (!val) return JSON_TYPE_MISSING;

    return type_at(val, data->buf + data->used_sz);
}

int8_t get_string_by_path(jdata_t *data, const char *path, char *output, size_t n)
{
    if (!data || !data->buf || !output || n == 0) return -1;

    const char *end = data->buf + data->used_sz;
    const char *val = navigate(data->buf, data->used_sz, path);
    if (!val || val >= end || *val != '"') return -1;

    val++; /* skip opening '"' */
    size_t i = 0;

    while (val < end && *val != '"') {
        if (i >= n - 1) return -1; /* output buffer too small */

        if (*val == '\\') {
            val++;
            if (val >= end) return -1;
            switch (*val) {
                case '"':  output[i++] = '"';  break;
                case '\\': output[i++] = '\\'; break;
                case '/':  output[i++] = '/';  break;
                case 'n':  output[i++] = '\n'; break;
                case 'r':  output[i++] = '\r'; break;
                case 't':  output[i++] = '\t'; break;
                case 'b':  output[i++] = '\b'; break;
                case 'f':  output[i++] = '\f'; break;
                default:   output[i++] = *val; break;
            }
        } else {
            output[i++] = *val;
        }
        val++;
    }

    if (val >= end || *val != '"') return -1; /* unterminated string */
    output[i] = '\0';
    return 0;
}

int8_t get_float_by_path(jdata_t *data, const char *path, float *output)
{
    if (!data || !data->buf || !output) return -1;

    const char *end = data->buf + data->used_sz;
    const char *val = navigate(data->buf, data->used_sz, path);
    if (!val || val >= end) return -1;
    if (*val != '-' && (*val < '0' || *val > '9')) return -1;

    float result = 0.0f;
    int   sign   = 1;
    if (*val == '-') { sign = -1; val++; }

    while (val < end && *val >= '0' && *val <= '9') {
        result = result * 10.0f + (float)(*val - '0');
        val++;
    }
    if (val < end && *val == '.') {
        val++;
        float place = 0.1f;
        while (val < end && *val >= '0' && *val <= '9') {
            result += (float)(*val - '0') * place;
            place  *= 0.1f;
            val++;
        }
    }
    if (val < end && (*val == 'e' || *val == 'E')) {
        val++;
        int exp_sign = 1;
        if (val < end && *val == '-') { exp_sign = -1; val++; }
        else if (val < end && *val == '+') { val++; }
        int exp = 0;
        while (val < end && *val >= '0' && *val <= '9') {
            exp = exp * 10 + (*val - '0');
            val++;
        }
        for (int j = 0; j < exp; j++) {
            result = (exp_sign > 0) ? result * 10.0f : result / 10.0f;
        }
    }

    *output = (float)sign * result;
    return 0;
}

int8_t get_bool_by_path(jdata_t *data, const char *path, bool *output)
{
    if (!data || !data->buf || !output) return -1;

    const char *end = data->buf + data->used_sz;
    const char *val = navigate(data->buf, data->used_sz, path);
    if (!val || val >= end) return -1;

    if (val + 4 <= end && strncmp(val, "true", 4) == 0) {
        *output = true;
        return 0;
    }
    if (val + 5 <= end && strncmp(val, "false", 5) == 0) {
        *output = false;
        return 0;
    }
    return -1;
}
