#pragma once

#include <cassert>
#include <hr4c/core/vector.hpp>
#include <json.h>
#include <map>
#include <variant>

#define HR4C_ASSERT(cond) assert(cond)

namespace hr4c {

using PropertyDict = json::jobject;

} // namespace hr4c
