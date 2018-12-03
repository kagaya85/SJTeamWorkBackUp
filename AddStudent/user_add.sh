#!/bin/bash
# author:kagaya
if [ $# -lt 2 ]
then
    echo "error：参数个数小于2" >&2
    exit -1
elif [ $# -gt 2 ]
then
    echo "error：参数个数大于2" >&2/*
    exit -1
fi

password="Yzh@1650275"

if [ -e $1 ]
then
    # 逐行读取文本处理
    while read line
    do
        if [ ${line:0:1} != '#' ]
        then
            str=(${line})    # change to list
            if [ ${#str[@]} -ge 2 ]
            then
                # adduser
                adduser "u${str[0]}"
                if [ $? -ne 0 ]; then
                    echo "error:用户u${str[0]}[${str[1]}]建立失败" >&2
                else
                    echo "用户u${str[0]}[${str[1]}]已建立"
                    
                    # add initial password
                    echo "u${str[0]}":$password | chpasswd
                    if [ $? -ne 0 ]
                    then
                        echo "error:用户${str[0]}密码设置失败" >&2
                    else
                        echo "u${str[0]}:$password" >> $2
                    fi

                    # enforce to change password
                    chage -d 0 "u${str[0]}"
                    if [ $? -ne 0 ]; then 
                        echo "error:用户${str[0]}强制改密设置失败" >&2
                    fi
                fi
            fi
        fi
    done < $1
else
    echo "error:文件${1}不存在" >&2
    exit -1
fi
