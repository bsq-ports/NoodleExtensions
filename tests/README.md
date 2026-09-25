# Gameplay lifetime regression checks

Run from the repository root with Python 3 and a C++20 compiler in PATH:

```sh
python3 tests/gameplay-lifetime.py
python3 tests/gameplay-roots.py
```

The scene test compiles the production lifetime helper and exercises repeated
level transitions, additive scene unloads, duplicate unloads, and replacement
GameCore scenes loaded before or during the original unload callback.

The cleanup test compiles the production cleanup function bodies against host
stand-ins. It checks that all retained references are released and cleanup is
idempotent, with deliberately omitted resets as negative controls. These tests
do not execute Unity, IL2CPP hooks, materials, or the game. An upstream Android
build and device validation are still required.
