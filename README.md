# kmod-symvers

This PoC changes symbol checksums that a given module:
 - exports using an (`EXPORT_SYMBOL`-family) macros (`symtab`),
 - requires from/expects to be provided by another module/the kernel (`__versions`).

When called w/ a path to an ELF file, it will simply dump the relevant checksums.
One may optionally pass a number of symbol, checksum pairs to override present checksum
values.

## Demo

### Changing the checksum value

```
$ grep xor_blocks Module.symvers
0x5b6c00e6      xor_blocks      crypto/xor      EXPORT_SYMBOL

$ cp $kmod_xor /tmp/xor.ko

$ nm /tmp/xor.ko | grep __crc_xor_blocks
000000005b6c00e6 A __crc_xor_blocks

$ kmod-symvers /tmp/xor.ko __crc_xor_blocks 12345678 | grep __crc_xor_blocks
0x12345678      __crc_xor_blocks                                symtab

$ nm /tmp/xor.ko | grep __crc_xor_blocks
0000000012345678 A __crc_xor_blocks

$ cmp -l $kmod_xor /tmp/xor.ko  | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}'
cmp: EOF on /tmp/xor.ko after byte 23960
00005159 E6 78
0000515A 00 56
0000515B 6C 34
0000515C 5B 12

$ kmod-symvers /tmp/xor.ko printk 66666666 | grep printk
0x66666666      printk                                          __versions

$ cmp -l $kmod_xor /tmp/xor.ko  | gawk '{printf "%08X %02X %02X\n", $1, strtonum(0$2), strtonum(0$3)}'
cmp: EOF on /tmp/xor.ko after byte 23960
000043C1 49 66
000043C2 A0 66
000043C3 E1 66
000043C4 27 66
00005159 E6 78
0000515A 00 56
0000515B 6C 34
0000515C 5B 12
```

### insmod test

```
$ insmod ./xor.ko
$ rmmod xor
$ lsmod | grep xor
$ ./kmod-symvers/bin/kmod-symvers ./xor.ko
0x15244c9d      boot_cpu_data                                   __versions
$ ./kmod-symvers/bin/kmod-symvers ./xor.ko boot_cpu_data 15244c9e
0x15244c9e      boot_cpu_data                                   __versions
$ insmod ./xor.ko
insmod: ERROR: could not insert module ./xor.ko: Invalid parameters
$ dmesg | tail
[2618670.121990] xor: disagrees about version of symbol boot_cpu_data
[2618670.126643] xor: Unknown symbol boot_cpu_data (err -22)
$ ./kmod-symvers/bin/kmod-symvers ./xor.ko boot_cpu_data 15244c9d
0x15244c9d      boot_cpu_data                                   __versions
$ insmod ./xor.ko
$ lsmod | grep xor -q && echo OK
OK
                        
```

## Remarks

 - Requires `CONFIG_MODVERSIONS=y`.
 - Tested on `CONFIG_MODULE_REL_CRCS=n` and the PoC will likely need to be modified to work on `=y`.
 - Although the code to deal with different endianness is present and ready to be used, it is not necessary for this PoC.
