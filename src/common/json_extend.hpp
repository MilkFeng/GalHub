#ifndef JSON_EXTEND_H
#define JSON_EXTEND_H

#include <json.hpp>

using Json = configor::wjson;

#define REQUIRED_WIDE(var) REQUIRED(var, L#var)

#endif //JSON_EXTEND_H
