#!/bin/bash
# author:kagaya
if [ $# -lt 2 ]
then
    echo "error����������С��2" >&2
    exit -1
elif [ $# -gt 2 ]
then
    echo "error��������������2" >&2/*
    exit -1
fi

password="Yzh@1650275"

if [ -e $1 ]
then
    # ���ж�ȡ�ı�����
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
                    echo "error:�û�u${str[0]}[${str[1]}]����ʧ��" >&2
                else
                    echo "�û�u${str[0]}[${str[1]}]�ѽ���"
                    
                    # add initial password
                    echo "u${str[0]}":$password | chpasswd
                    if [ $? -ne 0 ]
                    then
                        echo "error:�û�${str[0]}��������ʧ��" >&2
                    else
                        echo "u${str[0]}:$password" >> $2
                    fi

                    # enforce to change password
                    chage -d 0 "u${str[0]}"
                    if [ $? -ne 0 ]; then 
                        echo "error:�û�${str[0]}ǿ�Ƹ�������ʧ��" >&2
                    fi
                fi
            fi
        fi
    done < $1
else
    echo "error:�ļ�${1}������" >&2
    exit -1
fi
