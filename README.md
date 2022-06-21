## Companion Satellite

A small libary to allow for connecting a microcontroller to [Bitfocus Companion](https://github.com/bitfocus/companion) over a network.

Companion 2.2.0 and newer are supported

Each device will appear in companion as its own 'satellite' device, and can be configured as if they are local.

Note: This depends on C++17 or higher, use 
```
build_unflags = -std=gnu++11
build_flags = -std=gnu++17
```

Note: This connects over the satellite device api which uses TCP port 16622.