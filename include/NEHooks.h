#pragma once
#include "NELogger.h"

class Hooks {
private:
/// Store function name and install function pointer for all hooks, so we can sort and install them in the correct order
  static inline std::vector<std::pair<std::string, void (*)()>> installFuncs;

  static inline bool NoodleHookEnabled;

public:
  static void AddInstallFunc(std::string name, void (*installFunc)()) {
    installFuncs.emplace_back(name, installFunc);
  }

  static void InstallHooks() {
    std::sort(installFuncs.begin(), installFuncs.end(),
              [](const std::pair<std::string, void (*)()>& a, const std::pair<std::string, void (*)()>& b) {
                return a.first < b.first;
              });

    for (auto installFunc : installFuncs) {
      installFunc.second();
    }
  }

  static bool isNoodleHookEnabled() {
    return NoodleHookEnabled;
  }

  static constexpr void setNoodleHookEnabled(bool noodleHookEnabled) {
    NoodleHookEnabled = noodleHookEnabled;
  }
};

#define NEInstallHooks(func)                                                                                           \
  struct __NERegister##func {                                                                                          \
    __NERegister##func() {                                                                                             \
      Hooks::AddInstallFunc(std::string(#func), func);                                                                 \
      NELogger::Logger.info("NEHooks Registered install func: " #func);                                                \
    }                                                                                                                  \
  };                                                                                                                   \
  static __NERegister##func __NERegisterInstance##func;

void InstallAndRegisterAll();
