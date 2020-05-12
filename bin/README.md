# Change List

## May 4, 2020 - 1.151.8922
- Adds notification of timeline grooming.
- Removes the option to disable the system database.

## May 3, 2020 - 1.150.8921
- Adds ability for the Engage Engine library to run side-by-side with the OS-provided distribution of OpenSSL.

## May 1, 2020 - 1.148.8914
- Adds timeline grooming based on occupied disk storage.

## April 28, 2020 - 1.147.8913
- Adds static multicast reflector configuration for Rallypoints.
- Adds the ability to import X.509 elements into a certificate store from another.
- Adds display of certificate information by Rallypoints.
- Adds publishing of additional headers to ease 3rd-party C++ development.
- Removes Rallypoint reciprocal subscription - replaced by static multicast reflectors.
- Corrects a namespace naming issue for ConfigurationObjects under certain GCC compiler versions.
- Corrects a syntax error in Rallypoint post-installation script.

## April 10, 2020 - 1.145.8908
- Adds information to TX event notifications.
- Improves transmit contention (glaring) for talkers with the same priority.
- Exposes the mission generation for the node.js binding.

## April 7, 2020 - 1.143.8906
- Adds sourcing of Rallypoint mesh configuration from an executed command.
- Adds command execution after updates to Rallypoint status file.
- Adds command execution after updates to Rallypoint links file.
- Adds thread name to Win32 logging output.
- Adds process termination logic in case of Rallypoint hung shutdown.

## April 2, 2020 - 1.142.8905
- Corrects a TLS packet buffer overflow on Rallypoint peer registration/deregistration messages.
- Adds the ability to throttle Rallypoint inbound connections to guard against Denial Of Service attacks.
- Adds the ability to throttle Rallypoint inbound connections based on system CPU utilization.
- Adds safeguards against Rallypoint congestion collapse due to CPU pressure.
- Adds additional Rallypoint and core Engine worker queue metrics.
- Adds Rallypoint process uptime to the status file.
- Adds Rallypoint system CPU usage to the status file.
- Adds support for importing certificates and keys from PKCS #12 archives.
- Optimizes logging in Rallypoint to increase performance.
- Offloads realtime timeline event ECDSA signing to a seperate thread so as not to impact core user experience.

## March 27, 2020 - 1.140.8903
- Corrects an Engine shutdown issue when running under node.js.
- Corrects an X.509 certificate exchange issue Rallypoint peer links under certain Linux distros.
- Adds the ability to disable multicast failover at the Engine policy level.

## March 22, 2020 - 1.133.8893
- Corrects an issue on some Windows network drivers related to multicast receive on a QoS-enabled socket.

## March 9, 2020 - 1.132.8892
- Adds multicast failover and failback of downed Rallypoint links as a standard feature.
- Adds JSON clarifier objects to all events fired by the Engine.

## March 5, 2020 - 1.126.8887
- Adds Engage-managed certificate stores
- Adds tx mute/unmute for groups - including the ability to begin tx in a muted state
- Improved handling of QoS settings for TX
- Addition of QoS and TTL for multicast reflectors in Rallypoints
- Corrects a bug related to the TTL value for multicast traffic

## February 18, 2020 - 1.121.8880
- Corrects audio issues related to the new multiplexing speaker logic
- Corrects scratchy G.711 audio
- Addresses FDX/HDX inconsistencies
- Adds whitelisted multicast groups on Rallypoints
- Adds Rallypoint reciprocal subscription (--beta--) 
- Adds Visual C/C++ runtimes to the npm module

## January 26, 2020 - 1.116.8874
- Corrects an issue with audio on certain versions of Android
- Corrects a random crash on all platforms during shutdown.  
- Corrects minor memory leaks
- Adds performance enhancements for networking

## December 17, 2019 - 1.109.8864
- Improved support for application-defined audio devices

## December 10, 2019 - 1.108.8863
- Corrects a number of minor bugs
- Adds ability to disable audio record but retain timeline metadata
- Improves performance in the audio resampler

## November 21, 2019 - 1.102.8857
- Corrects a buffer overflow in the audio resampler for G.711 framed larger than 40ms
- Corrects a jitter buffer miscalculation
- Corrects a problem with unknown talker(s) associated with audio

## November 7, 2019 - 1.95.8851
- Corrects blocked socket issues
- Corrects a crash on channel leave during RX

