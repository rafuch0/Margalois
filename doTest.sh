#!/bin/bash
rm text.txt
for((i=0;i<33;i++))
do
echo -n "A" >> text.txt;
./a.out < runScriptEncrypt > /dev/null;
./a.out < runScriptDecrypt > /dev/null;
ls -l *.txt;
md5sum *.txt;
done
