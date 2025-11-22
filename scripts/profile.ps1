#!/usr/bin/env pwsh

if (Test-Path "$PSScriptRoot/ndkpath.txt")
{
    $NDKPath = Get-Content $PSScriptRoot/ndkpath.txt
} else {
    $NDKPath = $ENV:ANDROID_NDK_HOME
}

python3.10 $NDKPath/simpleperf/app_profiler.py -p com.beatgames.beatsaber -a com.unity3d.player.UnityPlayerActivity -lib .\build\debug\ --ndk_path $NDKPath --record_options=--exit-with-parent -r "--call-graph fp"

if ($?) {
    python3.10 $NDKPath/simpleperf/report_html.py
}