#ifndef PHYSICAL
#define PHYSICAL
#include "common.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/msg.h>

const int DatapackLen = 1036;
const int NODatapackLen = 12;
const int FramkindLen = 4;
const int SndNoLen = 4;
const int RecvNoLen = 4;

const unsigned int PureSIGpack = 0xFFFFFFFF;

enum Link_State
{
    LINK_ERROR, LINK_OK
};
enum Socket_Exchange_State
{
    SOCKET_ERROR, SOCKET_CLOSE, FROM_DATALINK_ERROR, TO_DATALINK_ERROR, SOCKET_OK
};
enum Read_State
{
    READ_ERROR, READ_CLOSE, READ_OK
};
enum Write_State
{
    WRITE_ERROR, WRITE_CLOSE, WRITE_OK
};

//----------Prework Part----------
int get_pid_by_name(const char* const proc_name);
//----------Msgqueue Part----------
int link_to_DatalinkLayer(const int KEY);
//----------Socket Part----------
int data_exchange(const int side, const int pid, const int msgid, const int sockfd);

unsigned int calc_bitstream(const char* const Bitstream, const int bitstream_length);

int read_bitstream(const int side, const int fd, const int buffer_len, char* const buffer);
int write_bitstream(const int side, const int fd, const int buffer_len, const char* const buffer);

#endif
