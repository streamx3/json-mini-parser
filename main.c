#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "json-mini-parser.h"

char *raw1 = "{"\
"  \"id\": 42,"\
"  \"name\": \"Acme Corp\","\
"  \"active\": true,"
"  \"rating\": 4.7,"\
"  \"address\": null,"\
"  \"tags\": [\"embedded\", \"iot\", \"sensor\"],"\
"  \"config\": {"\
"    \"interval_ms\": 500,"\
"    \"debug\": false,"\
"    \"thresholds\": [1.5, 3.0, 7.2]"\
"  },"\
"  \"nodes\": ["\
"    {\"uid\": 1, \"label\": \"alpha\", \"online\": true},"\
"    {\"uid\": 2, \"label\": \"bravo\", \"online\": false}"\
"  ]"\
"}";

char *raw2 = "{\"id\":42,\"name\":\"Acme Corp\",\"active\":true,\"rating\":4.7,\"address\":null,\"tags\":[\"embedded\",\"iot\",\"sensor\"],\"config\":{\"interval_ms\":500,\"debug\":false,\"thresholds\":[1.5,3,7.2]},\"nodes\":[{\"uid\":1,\"label\":\"alpha\",\"online\":true},{\"uid\":2,\"label\":\"bravo\",\"online\":false}]}";
int main()
{
  printf("JSON mini parser:\n");
  bool ret;

  jdata_t js;
  js.buf = raw1;
  js.used_sz = strlen(raw1);

  ret = json_valid(&js);
  if (ret != 0) {
    printf("JSON is invalid\n");
    return -1;
  } else {
    printf("JSON is valid\n");
  }

  JSON_TYPE_T type;

  type = get_type_by_path(&js, "/name");
  if (type != JSON_TYPE_STRING) {
    printf("type of 'name' is invalid\n");
    return -1;
  } else {
    printf("type of 'name' is valid\n");
  }

#define TMPSZ 32
  char tmp[TMPSZ];

  ret = get_string_by_path(&js, "/name", tmp, TMPSZ);
  if (ret != 0) {
    printf("failed to fetch string\n");
    return -1;
  } else {
    printf("string fetch is ok: '%s'\n", tmp);
  }

  if (strcmp(tmp, "Acme Corp") != 0) {
    printf("copied string invalid\n");
    return -1;
  } else {
    printf("copied string is ok: '%s'\n", tmp);
  }

  int32_t uid2;

  ret = get_int_by_path(&js, "/nodes[1]/uid", &uid2);
  if (ret != 0) {
    printf("failed to fetch int\n");
    return -1;
  } else {
    printf("int fetch is ok: %d\n", uid2);
  }

  return 0;
}
