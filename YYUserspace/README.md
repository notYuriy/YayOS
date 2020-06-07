#YayOS userspace projects

####What is this folder for?

This folder hosts source of all YayOS userspace applications

####What is a structure of application directory?

Assuming that project is named ```x```

```YYUserspace/x/build.sh``` - shell script launched on build
```YYUserspace/x/cleanup.sh``` - shell script launched after system build. Beware that ```YYUserspace/x/cleanup.sh``` may be launched even if ```YYUserspace/x/build.sh``` was not (this happens if some other build in ```YYUserspace/``` failed).
```YYUserspace/x/fsroot``` - Contains files that needs to be transferred to filesystem root. After a call to ```YYUserspace/x/build.sh```, ```builduserspace.sh``` will merge contents of ```YYUserspace/x/fsroot``` with ```tmpinitrd/``` that is used to create ramdisk