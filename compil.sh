#!/bin/bash
icpc offload_on_mic.cpp -o offload_on_mic -mmic /opt/intel/mic/coi/device-linux-release/lib/libcoi_device.so -I/opt/intel/mic/coi/include -lpthread -rdynamic -Wl,--enable-new-dtags -O0
icpc upanddown.c -o upanddown /opt/intel/mic/coi/host-linux-release/lib/libcoi_host.so -I/opt/intel/mic/coi/include -lpthread -rdynamic -Wl,--enable-new-dtags -O0
