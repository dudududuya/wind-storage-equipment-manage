#!/bin/bash
INPUT="./data/dev.csv"
TMP="./data/tmp_dev.csv"

awk '
{
    # 1. 所有连续空白替换为逗号
    gsub(/[ \t]+/,",");
    # 2. 分割每行所有字段，清除每个字段首尾空格
    split($0,arr,",");
    line="";
    for(i in arr){
        gsub(/^[ \t]+|[ \t]+$/,"",arr[i]);
        if(line=="") line=arr[i];
        else line=line","arr[i];
    }
    print line;
}' $INPUT > $TMP

mv $TMP $INPUT
echo "CSV转换完成：已清除所有字段首尾空白字符"

