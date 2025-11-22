#!/usr/bin/env pwsh

adb shell "setprop debug.oculus.capture.width 1920 & setprop debug.oculus.textureWidth 1920 & setprop debug.oculus.capture.height 1080 & setprop debug.oculus.textureHeight 1080 & setprop debug.oculus.capture.bitrate 40000000 & setprop debug.oculus.foveation.level 0"
