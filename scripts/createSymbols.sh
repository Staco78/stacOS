# /bin/bash

file=$(pwd)/initrd/symbols

echo -en "\x00\x00\x00\x00" > $file

i=0

while read line
do
    arr=($line);
    echo ${arr[0]} | xxd -r -p | xxd -g 8 -e | xxd -r >> $file
    echo -n ${arr[2]} >> $file
    echo -n -e "\x00" >> $file
    let "i+=1"
done

printf "%08x" $i | xxd -r -p | xxd -g 4 -e | xxd -r | dd of="$file" conv=notrunc bs=1 count=4 status=none