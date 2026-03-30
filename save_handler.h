#pragma once

#include <string>

#include "classes/library.h"

namespace SaveHandler {
bool saveAll(Library& library, const std::string& directory = "savefiles");
bool loadAll(Library& library, const std::string& directory = "savefiles");
}
