# How to use
1. Copy and paste the amalgamation (aes_128_gcm.c/.h) into your source code.   
2. Configure the .h file. Mainly define (or not) the `HARDWARE_SPEED` macro. This will turn hardware acceration on/off. If it is on (define it) make sure to compile with the flags `-maes -msse4 -mpclmul` OR `-maes -msse4 -mpclmul -O3 -lm --march=native`. And only turn on it compiling for intel/AMD x84. 

# Compatiblility
I don't have ARM or AMD so I have no idea if it works on that.

# Warning
This is just some random libary you found on the internet, and probaly shouldn't be used in anything important.   
It should be branching/timing attack safe but I DO NOT KNOW WHAT I AM DOING.

# USE AT YOUR OWN RISK