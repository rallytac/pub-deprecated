# Change List

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

