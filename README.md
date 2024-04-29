# 📃 json-parser
json-parser - C++ header-only library for working with JSON.

## Usage

### Parsing
``file.json``
```json
[
    {
        "name": "User1",
        "password": "ultrahardpassword",
        "administrator": false
    },
    {
        "name": "Admin1",
        "password": "veryultrahardpassword",
        "administrator": true
    }
]
```

```cpp
#include <iostream>
#include "JSONParser.h"

int main() {
    JSON::Parser parser = JSON::Parser::fromFile("file.json");
    JSON::Value json = parser.parse();

    if (!parser.isOK()) {
        std::cout << "Failed to parse JSON\n";
        return -1;
    }

    std::cout << json.getRepresentation(4) << "\n";
    
    for (JSON::Value user : json.toArray()) {
        std::cout << user["name"].toString() << "\n";
        std::cout << user["password"].toString() << "\n";
        std::cout << user["administrator"].toBoolean() << "\n\n";
    }

    /*
        JSON::Value::getType() can be used to check value type

        if (json.getType() == JSON::ValueType_Object) {
            // ...
        } else {
            // ...
        }
    */

    return 0;
}
```
#### Result
```
[
    {
        "name": "User1",
        "password": "ultrahardpassword",
        "administrator": false
    },
    {
        "name": "Admin1",
        "password": "veryultrahardpassword",
        "administrator": true
    }
]

User1
ultrahardpassword
0

Admin1
veryultrahardpassword
1
```

### Generating
```cpp
#include <iostream>
#include "JSONParser.h"

int main() {
    JSON::Value json = JSON::Value::makeArray();
    
    for (int i = 0; i < 2; ++i) {
        JSON::Value post = JSON::Value::makeObject();
        post["id"] = JSON::Value::makeNumber(i);
        post["title"] = JSON::Value::makeString("Post" + std::to_string(i));
        post["body"] = JSON::Value::makeString("Post" + std::to_string(i) + " body");
        json.pushToArray(post);
    }

    // Second argument is indent (optional)
    if (!json.saveToFile("file.json", 4)) {
        std::cout << "Failed to save data to \"file.json\"\n";
        return -1;
    }

    std::cout << "Saved data to \"file.json\"\n";
    return 0;
}
```

#### Result
```json
[
    {
        "id": 0,
        "title": "Post0",
        "body": "Post0 body"
    },
    {
        "id": 1,
        "title": "Post1",
        "body": "Post1 body"
    }
]
```

## TODOs
- [ ] Better error handling
- [ ] Code refactoring
