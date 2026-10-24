
#pragma once
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/rapidjson.hpp"

namespace NoodleExtensions {
static inline const std::u16string REQUIREMENTNAME = u"Noodle Extensions";
static inline const std::string U8_REQUIREMENTNAME = "Noodle Extensions";

static inline const std::string U8_ME_REQUIREMENTNAME = "Mapping Extensions";
static inline const std::u16string ME_REQUIREMENTNAME = u"Mapping Extensions";
// TODO: Move all JSON property references to constants?
} // namespace NoodleExtensions

class NELogger {
public:
  static constexpr auto Logger = Paper::ConstLoggerContext("NoodleExtensions");
};

void PrintJSONValue(rapidjson::Value const& json);
void PrintBacktrace(size_t maxLines);