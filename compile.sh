#!/bin/bash
g++ alsa-ndi-stream.cpp -lasound -lndi -I/home/pi/alsa-ndi-stream/ndi-sdk-linux/include -L/home/pi/alsa-ndi-stream/ndi-sdk-linux/lib/arm-rpi3-linux-gnueabihf -o alsa-ndi-stream
