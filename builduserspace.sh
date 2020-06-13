#!/bin/bash

echo -e "\033[1;37mBuilding userspace\033[0m"

function cleanup {
    local error_code="$?"

    test "$error_code" == 0 && echo -e -n "\033[1;32mDone. "; 
    test "$error_code" == 0 || echo -e -n "\033[1;31mError ($error_code). ";
    echo -e "Cleaning up\033[0m"
    echo -e "\033[1;32m[ RM ]\033[0m \033[1;37misotree\033[0m"
    rm -rf isotree 
    echo -e "\033[1;32m[ RM ]\033[0m \033[1;37mtmpinitrd\033[0m"
    rm -rf tmpinitrd
    programsPaths="eval find YYUserspace -maxdepth 1 -mindepth 1 -type d -name '*'"
    for path in YYUserspace/*/
    do
        echo -e "\033[1;32m[ SH ]\033[0m \033[1;37m$path/cleanup.sh\033[0m"
        cd "$path"
        eval "./cleanup.sh" || exit
        cd ../..
    done
    return;
}

trap cleanup EXIT

mkdir isotree

mkdir isotree/boot || exit
mkdir isotree/boot/grub || exit
cp grub/grub.cfg isotree/boot/grub/grub.cfg || exit
cp YYSloth/kernel.bin isotree/boot/kernel.bin || exit

cp -r initrd tmpinitrd

for path in YYUserspace/*/
do
    echo -e "\033[1;32m[ SH ]\033[0m \033[1;37m$path/build.sh\033[0m"
    cd "$path"
    eval "./build.sh" || exit
    cd ../..
    rsync -a -v --ignore-existing "$path/fsroot/" tmpinitrd/ 2> /dev/null > /dev/null || exit
done


echo -e "\033[1;32m[ RD ]\033[0m \033[1;37minitrd\033[0m"
tar -cf isotree/initrd.img tmpinitrd/ --format=v7 || exit

echo -e "\033[1;32m[ BL ]\033[0m \033[1;37mgrub/grub.cfg\033[0m"
grub-mkrescue isotree -o YayOS.iso 2> /dev/null || exit

true