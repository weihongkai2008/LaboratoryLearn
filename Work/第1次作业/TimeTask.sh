#!/bin/bash

at -f TimeTask.sh 00:01

BakDir=/home/frank/frankbak
DataBakDir=/home/frank
DAYS=7
date=` date +%Y%m%d `
tar -zcvf $BakDir/frank$date.tar.gz $DataBakDir
cd $BakDir
find $BakDir -name “frank*” -type f -mtime +$DAYS -exec rm {} \;  
deldate=` date -d -7day +%Y%m%d `
ftp 192.168.8.10
user dzkjdxaz 753
binary
cd frankbak
put frank$date.tar.gz
delete frank$deldate.tar.gz
close
bye
