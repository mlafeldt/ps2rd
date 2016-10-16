PS2rd - PS2 remote debugger
===========================

[![Build Status](https://travis-ci.org/mlafeldt/ps2rd.svg?branch=master)](https://travis-ci.org/mlafeldt/ps2rd)

PS2rd is a collection of Open Source tools to remotely debug PS2 applications,
including commercial PS2 games.

Features:

 - Dump contents of PS2 memory to PC
 - [Planned] Write data to PS2 memory
 - [Planned] Set breakpoints/watchpoints
 - Powerful cheat system similar to CB or AR
 - Video mode patcher
 - Compatible with Open PS2 Loader

Requirements:

 - PS2 game console with Ethernet port/adapter
 - Ability to run homebrew on PS2 (FMCB, Independence Exploit, etc.)
 - PC with Linux or Windows OS
 - Ethernet cable for PC-PS2 connection

Instructions on how to compile and install PS2rd can be found in the [INSTALL]
file. The directory [Documentation] is generally the first place to look for
further information.


Disclaimer
----------

PS2RD IS NOT LICENSED, ENDORSED, NOR SPONSORED BY SONY COMPUTER ENTERTAINMENT,
INC. ALL TRADEMARKS ARE PROPERTY OF THEIR RESPECTIVE OWNERS.

PS2rd comes with ABSOLUTELY NO WARRANTY. It is covered by the GNU General Public
License. Please see file [COPYING] for further information.

Besides the homebrew [PS2SDK], PS2rd makes use of these libraries:

- [libconfig] by Mark Lindner et al.
- [libcheats] by Mathias Lafeldt

Both libraries are covered by the GNU Lesser General Public License. Please see
file [COPYING.LESSER] for further information.


Contact
-------

- [PS2rd project site](https://github.com/mlafeldt/ps2rd)
- [Official PS2rd forum at PSX-Scene](http://psx-scene.com/forums/forumdisplay.php?f=173)


[COPYING.LESSER]: https://github.com/mlafeldt/ps2rd/blob/master/COPYING.LESSER
[COPYING]: https://github.com/mlafeldt/ps2rd/blob/master/COPYING
[Documentation]: https://github.com/mlafeldt/ps2rd/tree/master/Documentation
[INSTALL]: https://github.com/mlafeldt/ps2rd/blob/master/INSTALL
[PS2SDK]: https://github.com/ps2dev/ps2sdk
[libcheats]: https://github.com/mlafeldt/libcheats
[libconfig]: http://www.hyperrealm.com/libconfig/
