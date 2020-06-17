# YayOS

![sorry, no pic for you](./Pictures/showcase.gif)

*Recorded from xterm. QEMU links stdin with COM1, that YayOS uses*

### What is this repo for?

YayOS is an operating system powered by YYSloth kernel. 

### What is the system capable of doing?

It is capable of running simple shell connected to a serial port. Shell can be used to run other applications on the ramdisk

### What is the kernel capable of doing?

The API provided by the kernel is extremely simple. It is now fully described in the file ```YYSloth/README.md```. Docs are coming in the future.

### How directory structure works

```YYSloth/``` - everything related to the kernel (source, docs, binary...)

```YYUserspace/``` - source code of the user space applications. One folder is used for each application. For the application folder structure see ```YYUserspace/README.md```

```initrd``` - static data that needs to be located at the root of filesystem

```grub``` - grub metadata

### What are shell scripts doing?

```build.sh``` - build the whole system

```buildkernel.sh``` - only build kernel

```builduserspace.sh``` - create ISO. Requires ```buildkernel.sh``` to be executed in advance

```test.sh``` - run the kernel in qemu. There are multiply lines. You can uncomment one that you need.

### Licensing

MIT License applies to every single file in the source code except YYSloth/src/boot/boot.s. This file is an updated version of boot.s from the first edition of blog_os ("https://github.com/phil-opp/blog_os") and it is licensed under YYSloth/src/boot/LICENSE-MIT
