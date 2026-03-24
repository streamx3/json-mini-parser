#ifndef JSON_MINI_PARSER_H
#define JSON_MINI_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// This is an example JSON to be used for testing end description
/*
{
  "id": 42,
  "name": "Acme Corp",
  "active": true,
  "rating": 4.7,
  "address": null,
  "tags": ["embedded", "iot", "sensor"],
  "config": {
    "interval_ms": 500,
    "debug": false,
    "thresholds": [1.5, 3.0, 7.2]
  },
  "nodes": [
    {"uid": 1, "label": "alpha", "online": true},
    {"uid": 2, "label": "bravo", "online": false}
  ]
}
*/

typedef struct json_data_t
{
    char   *buf;      // Pointer to JSON text buffer
    size_t  used_sz;  // Number of valid bytes in buffer

} jdata_t;

typedef enum JSON_TYPE_T {
    JSON_TYPE_INVALID = 0, // Something went wrong (bad input, parse error)
    JSON_TYPE_MISSING,     // Path not found in the document
    JSON_TYPE_NULL,        // Value is JSON null
    JSON_TYPE_BOOL,        // Value is true or false
    JSON_TYPE_STRING,      // Value is a quoted string
    JSON_TYPE_INT,         // Value is an integer number
    JSON_TYPE_FLOAT,       // Value is a floating-point number
    JSON_TYPE_OBJECT,      // Value is a JSON object  { }
    JSON_TYPE_ARRAY,       // Value is a JSON array   [ ]
} JSON_TYPE_T;

/**
 * @brief Validate a JSON document
 *
 * Checks that buf is non-NULL, used_sz > 0, and the JSON syntax is valid.
 *
 * @param[in] data  Pointer to the jdata_t structure
 * @return 0 if valid, -1 otherwise
 */
int json_valid(jdata_t *data);

/**
 * @brief Get the type of a value at a given path
 *
 * @param[in] data  Pointer to the jdata_t structure
 * @param[in] path  Path string, e.g. "/id", "/config/debug", "/nodes[0]/uid"
 * @return The JSON_TYPE_T of the value, or JSON_TYPE_MISSING / JSON_TYPE_INVALID
 */
JSON_TYPE_T get_type_by_path(jdata_t *data, const char *path);

/**
 * @brief Copy a string value at a given path into output
 *
 * @param[in]  data    Pointer to the jdata_t structure
 * @param[in]  path    Path string
 * @param[out] output  Destination buffer
 * @param[in]  n       Size of destination buffer (including NUL terminator)
 * @return 0 on success, -1 on error or truncation
 */
int8_t get_string_by_path(jdata_t *data, const char *path, char *output, size_t n);

/**
 * @brief Get an integer value at a given path
 *
 * Accepts JSON_TYPE_INT values only (no decimal point / exponent).
 * Returns -1 if the value is a floating-point literal.
 *
 * @param[in]  data    Pointer to the jdata_t structure
 * @param[in]  path    Path string
 * @param[out] output  Where to store the result
 * @return 0 on success, -1 on error
 */
int8_t get_int_by_path(jdata_t *data, const char *path, int32_t *output);

/**
 * @brief Get a float value at a given path
 *
 * Accepts both JSON_TYPE_FLOAT and JSON_TYPE_INT values — an integer
 * literal such as 42 is returned as 42.0f without error.
 *
 * @param[in]  data    Pointer to the jdata_t structure
 * @param[in]  path    Path string
 * @param[out] output  Where to store the result
 * @return 0 on success, -1 on error
 */
int8_t get_float_by_path(jdata_t *data, const char *path, float *output);

/**
 * @brief Get a bool value at a given path
 *
 * @param[in]  data    Pointer to the jdata_t structure
 * @param[in]  path    Path string
 * @param[out] output  Where to store the result
 * @return 0 on success, -1 on error
 */
int8_t get_bool_by_path(jdata_t *data, const char *path, bool *output);

#endif // JSON_MINI_PARSER_H
