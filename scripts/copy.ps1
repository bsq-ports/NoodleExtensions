#!/usr/bin/env pwsh

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$args,

    [Parameter(Mandatory = $false)]
    [Switch] $log,

    [Parameter(Mandatory = $false)]
    [Switch]$release
)

if ($release) {
    & $PSScriptRoot/build.ps1 -release
} else {
    & $PSScriptRoot/build.ps1
}
if ($?) {
    # adb push build/libchroma.so /sdcard/ModData/com.beatgames.beatsaber/Modloader/mods/libchroma.so
    if ($?) {
        adb shell am force-stop com.beatgames.beatsaber
        adb shell am start com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
        if ($log.IsPresent) {
            $timestamp = Get-Date -Format "MM-dd HH:mm:ss.fff"
            adb logcat -c
            adb logcat > log.txt
        }
    }
}