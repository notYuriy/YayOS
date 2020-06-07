# YayOS

#####What is this repo for?

YayOS is a monolitic hobby operating system powered by YYSloth kernel. 

#####What is system capable of doing?

For now, it is only capable of runnning one executable from ramdisk (stored in the file ```/bin/init```).

#####What is kernel capable of doing?

The API provided by the kernel is extremely simple. It is now fully described in ```YYSloth/README.md```. Docs are coming in the future.

#####How directory structure works

```YYSloth/``` - everything related to kernel (source, docs, binary...)

```YYUserspace/``` - source code of userspace applications. One folder is used for each application. For the application folder structure see ```YYUserspace/README.md```

```initrd``` - static data that needs to be located at the root of filesystem

```grub``` - grub metadata

#####What shell scripts are doing?

```build.sh``` - build the whole system
```buildkernel.sh``` - only build kernel
```builduserspace.sh``` - create ISO. Requires ```buildkernel.sh``` to be executed in advance
```test.sh``` - run the kernel in qemu. There are multiply lines. You can uncomment one that you need.

#####Licensing

MIT License applies to every single file in the source code except YYSloth/src/boot/boot.s. This file is an updated version of boot.s from the first edition of blog_os ("https://github.com/phil-opp/blog_os") and it is licensed under YYSloth/src/boot/LICENSE-MIT
