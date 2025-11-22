# Builds a .zip file for loading with BMBF
$NDKPath = Get-Content ./ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk

Compress-Archive -Path "./libs/arm64-v8a/libnoodleextensions.so", "./libs/arm64-v8a/libcustom-types.so", "./libs/arm64-v8a/libbeatsaber-hook_0_8_4.so", "./bmbfmod.json" -DestinationPath "./NoodleExtensions.zip" -Update