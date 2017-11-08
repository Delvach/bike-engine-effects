# bike-engine-effects
This is Arduino code that powers a (faux) turbine engine that sits on the back of a bike, with interval timing-based special effects that use lights, sounds, fans, heating pad, a pump, hot water reservoir and silicone tubing to agitate dry ice for the effect of a rocket strapped to the back of a bike.

## Disclaimer
This code is very much under development. I have been working on it in various forms for the past few years, and while I've made some fun progress and learned a lot, I do _not_ bill myself as a C++ programmer and this code is full of things that I both realize need to be refactored, and of which I am entirely ignorant. Feedback and criticism are both appreciated as I'm hoping to eventually re-factor most of it once I have a better fundamental understanding of C++ beyond what I'm trying to accomplish within the Arduino IDE as I go along.

## Getting Started
There is a single dependency, a small [Arduino library](https://github.com/Delvach/Animation) that needs to be included in your /Arduino/libraries directory. It is from an older version of this code and will likely be deprecated in the near future as the lighting behavior matures.

There are several .WAV files that are currently used for the sequencing. I have not included them in this repo at the moment to avoid any potential copyright infringement. As this is a very specific hardware setup, I'm not sure how likely it is that anyone will be attempting to replicate it. I will be expanding my documentation of this project over time, but have fielded enough questions about the project that I've been motivated to provide some information on how everything works.

## Authors
* **Ben Del Vacchio** - *Initialization* - [delvach.com](https://delvach.com)