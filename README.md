# yabi
*YABI* is Yet Another Basic Interpolator (sorry, still a bad name). It is designed to do a _good enough_ job interpolating between points, over time, on one or more control 'channels'. Think fading between colors or repositioning a servo. It is the BOTTOM of the stack, and is designed to sit directly above the hardware.

If you'd like, say, some sort of _animation framework_, have a look at *BAF* [1], which I wrote as a higher level companion.

Remember, perfect is the enemy of good enough. Things (like the channel LERP are designed to work _well enough_, but not necessarily be perfectly accurate.

### Future Features
* Proper channel group control
* Fixed point math for LERP
* Non-linear interpolation (user selectable)
* Proper use of a user-owned state struct, to allow for multiple simultaneous YABIs
* An optimistic outlook on life

[1]: https://github.com/borgel/baf

