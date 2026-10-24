#include "beatsaber-hook/shared/rapidjson.hpp"
#include "tracks/shared/Vector.h"
#include "NELogger.h"

#include "tracks/shared/Json.h"

//
// namespace NEJSON {
//
// static std::optional<bool> ReadOptionalBool(const rapidjson::Value &object, const char *key) {
//    auto itr = object.FindMember(key);
//    if (itr != object.MemberEnd()) {
//        if (itr->value.IsString()) {
//            std::string boolS = itr->value.GetString();
//
//            if (boolS == "true") {
//                return true;
//            }
//
//            NELogger::GetLogger().error("ReadOptionalBool: THE VALUE IS A STRING WHY! value: \"%s\"", boolS.c_str());
//            return false;
//        }
//
//        return itr->value.GetBool();
//    }
//    return std::nullopt;
//}
//
// static std::optional<float> ReadOptionalFloat(const rapidjson::Value &object, const char *key) {
//    auto itr = object.FindMember(key);
//    if (itr != object.MemberEnd()) {
//        return itr->value.GetFloat();
//    }
//    return std::nullopt;
//}
//
// static std::optional<NEVector::Vector2> ReadOptionalVector2(const rapidjson::Value &object,
//                                                     const char *key) {
//    auto itr = object.FindMember(key);
//    if (itr != object.MemberEnd() && itr->value.Size() >= 2) {
//        float x = itr->value[0].GetFloat();
//        float y = itr->value[1].GetFloat();
//        return NEVector::Vector2(x, y);
//    }
//    return std::nullopt;
//}
//
//// Used for note flip
// static std::optional<NEVector::Vector2> ReadOptionalVector2_emptyY(const rapidjson::Value &object,
//                                                          const char *key) {
//     auto itr = object.FindMember(key);
//
//     if (itr != object.MemberEnd() && itr->value.Size() >= 1) {
//         float x = itr->value[0].GetFloat();
//         float y = 0;
//
//         if (itr->value.Size() > 1) {
//             y = itr->value[1].GetFloat();
//         }
//         return NEVector::Vector2(x, y);
//     }
//     return std::nullopt;
// }
//
// using OptPair = std::pair<std::optional<float>, std::optional<float>>;
//
// static OptPair ReadOptionalPair(const rapidjson::Value &object,const char *key) {
//     auto itr = object.FindMember(key);
//
//     if (itr != object.MemberEnd() && itr->value.Size() >= 1) {
//         float x = itr->value[0].GetFloat();
//         float y = 0;
//
//         if (itr->value.Size() >= 2) {
//             y = itr->value[1].GetFloat();
//             return OptPair(std::optional<float>(x), std::optional<float>(y));
//         }
//         return OptPair(std::optional<float>(x), std::nullopt);
//     }
//     return OptPair(std::nullopt, std::nullopt);
// }
//
// static std::optional<NEVector::Quaternion> ReadOptionalRotation(const rapidjson::Value &object,
//                                                       const char *key) {
//     auto itr = object.FindMember(key);
//     if (itr != object.MemberEnd()) {
//         NEVector::Vector3 rot;
//         if (itr->value.IsArray() && itr->value.Size() >= 3) {
//             float x = itr->value[0].GetFloat();
//             float y = itr->value[1].GetFloat();
//             float z = itr->value[2].GetFloat();
//             rot = NEVector::Vector3(x, y, z);
//         } else if (itr->value.IsNumber()) {
//             rot = NEVector::Vector3(0, itr->value.GetFloat(), 0);
//         }
//
//         return NEVector::Quaternion::Euler(rot);
//     }
//     return std::nullopt;
// }
//
// static std::optional<NEVector::Vector3> ReadOptionalVector3(const rapidjson::Value &object,
//                                                      const char *key) {
//     auto itr = object.FindMember(key);
//     if (itr != object.MemberEnd() && itr->value.Size() >= 3) {
//         float x = itr->value[0].GetFloat();
//         float y = itr->value[1].GetFloat();
//         float z = itr->value[2].GetFloat();
//         return NEVector::Vector3(x, y, z);
//     }
//     return std::nullopt;
// }
//
// static std::optional<std::array<std::optional<float>, 3>> ReadOptionalScale(const rapidjson::Value &object,
//                                                                      const char *key) {
//     auto itr = object.FindMember(key);
//     if (itr != object.MemberEnd() && itr->value.IsArray()) {
//         rapidjson::SizeType size = itr->value.Size();
//         std::optional<float> x = size >= 1 ? std::optional{itr->value[0].GetFloat()} : std::nullopt;
//         std::optional<float> y = size >= 2 ? std::optional{itr->value[1].GetFloat()} : std::nullopt;
//         std::optional<float> z = size >= 3 ? std::optional{itr->value[2].GetFloat()} : std::nullopt;
//         return {{x, y, z}};
//     }
//     return std::nullopt;
// }
//
// } // namespace NEJSON