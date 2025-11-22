#!/usr/bin/env pwsh

if (Test-Path "$PSScriptRoot/ndkpath.txt")
{
    $NDKPath = Get-Content $PSScriptRoot/ndkpath.txt
} else {
    $NDKPath = $ENV:ANDROID_NDK_HOME
}

$stackScript = "$NDKPath/ndk-stack"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $stackScript += ".cmd"
}

Get-Content ./test.log | & $stackScript -sym ./obj/local/arm64-v8a/ > test_unstripped.log
