# json-mini-parser
A minimalistic JSON parser in C that requires no extra memory

## Usage

``` C
// Include
#include "json-mini-parser.h"

// Populating a structure
jdata_t js;
js.buf = raw_json;
js.used_sz = strlen(raw_json); // You might put here
// a result of read() here or something

// Validation
bool ret;
ret = json_valid(&js);

// Figuring a type
JSON_TYPE_T type;
type = get_type_by_path(&js, "/name");

// Fetching string like {"name":"something"}
char tmp[32];
ret = get_string_by_path(&js, "/name", tmp, 32);

// Fetch integer
int32_t uid2;
ret = get_int_by_path(&js, "/nodes[1]/uid", &uid2);
```
For more examples, look at `main.c`

## Build 
Do the usual thing:
``` Shell
mkdir build
cd build
cmake ..
make -j
./json-mini-parser
```

In order to run tests just to `ctest`.
