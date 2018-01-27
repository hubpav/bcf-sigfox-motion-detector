<a href="https://www.bigclown.com/"><img src="https://bigclown.sirv.com/logo.png" width="200" height="59" alt="BigClown Logo" align="right"></a>

# Firmware for BigClown Sigfox Motion Detector

[![Travis](https://img.shields.io/travis/bigclownlabs/bcp-sigfox-motion-detector/master.svg)](https://travis-ci.org/bigclownlabs/bcp-sigfox-motion-detector)
[![Release](https://img.shields.io/github/release/bigclownlabs/bcp-sigfox-motion-detector.svg)](https://github.com/bigclownlabs/bcp-sigfox-motion-detector/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcp-sigfox-motion-detector.svg)](https://github.com/bigclownlabs/bcp-sigfox-motion-detector/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/BigClownLabs.svg?style=social&label=Follow)](https://twitter.com/BigClownLabs)

![Photo of Sigfox Motion Detector assembly](doc/sigfox-motion-detector-enclosures.png)

This repository contains firmware for Sigfox Motion Detector based on PIR Module.
Firmware is programmed into [Core Module](https://shop.bigclown.com/core-module).
Binary version is available in section [Releases](https://github.com/bigclownlabs/bcp-sigfox-motion-detector/releases).

> Detailed information about this project can be found in [BigClown Documentation](https://doc.bigclown.com).

**TODO** Link to documentation article

## Introduction

Sigfox Motion Detector is a battery-operated indoor device integrating digital PIR sensor (passive infrared sensing technology).
This device is able to run from two AAA Alkaline batteries for at least 1 year.
It reports motion event to Sigfox network.
The minimum report period for the motion event is 15 minutes.
You can route this event as HTTP POST request with JSON body to your own web app via [MySigfox](https://www.mysigfox.com) service.

## Hardware

The following hardware components are used for this project:

* **[PIR Module](https://shop.bigclown.com/pir-module)**
* **[Core Module](https://shop.bigclown.com/core-module)**
* **[Sigfox Module](https://shop.bigclown.com/sigfox-module)**
* **[Mini Battery Module](https://shop.bigclown.com/mini-battery-module)**

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
