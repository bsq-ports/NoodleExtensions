param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$args,
    [switch]$log = $false,
    [switch]$release = $false
)

if ($?) {
    adb push build/libnoodleextensions.so /sdcard/ModData/com.beatgames.beatsaber/Modloader/mods/libnoodleextensions.so
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
