#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include "common.h"

using namespace std;

pid_t getPidByName(const char * const task_name)
{
    const int BUF_SIZE = 1024;

    DIR *dir;
    pid_t pid;
    struct dirent *ptr;
    FILE *fp;
    char filepath[50];
    char cur_task_name[50];
    char buf[BUF_SIZE];

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) //循环读取/proc下的每一个文件/文件夹
        {
            //如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            sprintf(filepath, "/proc/%s/status", ptr->d_name);//生成要读取的文件的路径
            fp = fopen(filepath, "r");
            if (NULL != fp)
            {
                if(fgets(buf, BUF_SIZE-1, fp)== NULL ){
                    fclose(fp);
                    continue;
                }
                sscanf(buf, "%*s %s", cur_task_name);

                //如果文件内容满足要求则打印路径的名字（即进程的PID）
                if (strstr(cur_task_name, task_name))
                {
                    // cout << "Datalink: cur_task_name " << cur_task_name << endl;
                    // cout << "Datalink: task_name " << task_name << endl;
                    pid = atoi(ptr->d_name);
                    // sscanf(ptr->d_name, "%d", pid);
                    return pid;
                }
                fclose(fp);
            }
        }
        closedir(dir);
    }
    pid = -1;
    return pid;
}

void packet::operator=(const packet &p)
{
    memcpy(data, p.data, MAX_PKT);
}
