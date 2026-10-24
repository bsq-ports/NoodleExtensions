#!/usr/bin/env pwsh

if (Test-Path "$PSScriptRoot/../ndkpath.txt")
{
    $NDKPath = Get-Content $PSScriptRoot/../ndkpath.txt
} else {
    $NDKPath = $ENV:ANDROID_NDK_HOME
}

echo "Using NDK Path: $NDKPath"
python3 $NDKPath/simpleperf/app_profiler.py -p com.beatgames.beatsaber -a com.unity3d.player.UnityPlayerGameActivity -lib ./build/debug/ --ndk_path $NDKPath --record_options=--exit-with-parent -r "--call-graph fp"

if ($?) {
    python3 $NDKPath/simpleperf/report_html.py
}