#!/usr/bin/env pwsh

# Thanks to konk rame rack: https://github.com/kodenamekrak/Cinema/blob/main/scripts/addr2line.ps1
# Usage:
# 1. Copy debug so from release to build/debug/libanytext.so
# 2. qpm s a2l <address such as 00000000000f12c8>

param($p1, $p2)

if(Test-Path "./ndkpath.txt") {
    $ndkpath = Get-Content "./ndkpath.txt"
}
else
{
    $ndkpath = $env:ANDROID_NDK_HOME
}
$child = Get-ChildItem -Path $ndkpath/toolchains/llvm/prebuilt -Name
& $ndkpath/toolchains/llvm/prebuilt/$child/bin/llvm-addr2line -e ./build/debug/$p1 $p2