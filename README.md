LZW
=======

This program compresses and decompresses files using the LZW algorithm

Help
------

    Usage ./lzw [options] [file]
    Options:
        -c: Compress file (appends .lzw)
        -d: Decompress file (takes off .lzw or adds .orig)

Example:
```shell
$ ./lzw -c src/LZW.c
Elapsed: 0.002306s
$ ls -l src/ | grep LZW
-rw-r--r-- 1 fsmv users 4404 Mar  2 22:19 LZW.c
-rw-r--r-- 1 fsmv users 2904 Mar  2 22:38 LZW.c.lzw
$ mv src/LZW.c src/LZW.c.old
$ ./lzw -d src/LZW.c.lzw
Elapsed: 0.006399s
$ diff src/LZW.c src/LZW.c.old
$
```

Building
========

```shell
git clone git@github.com:Fsmv/life-gif.git
mkdir build
cd build
cmake ..
make
```
