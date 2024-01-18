# alsa-ndi-streamer

This source code is a combination of a basic alsa recorder coupled with the NDIlib_Send_Audio_16bpp example. It is written and tested on a RPi3.

Once compiled, the program takes two arguments: The target alsa device, and a name for the NDI stream.

`./alsa-ndi-streamer hw:0,0 CoolStreamName`

Also included is a sample udev ruleset for both naming (ATTR) and starting a systemd service based on a specific USB sound card. In my use case, I have three identical sound cards each plugged in to a USB hub. The udev ruleset names them corresponding to their physical location.


Resources used:  
https://gist.github.com/albanpeignier/104902
https://www.linuxjournal.com/article/6735
https://alsa.opensrc.org/Udev