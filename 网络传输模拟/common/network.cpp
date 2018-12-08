#include <iostream>
#include <cstdio>

#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "network.h"

layer_status Network::NetworkStatus;

Network::Network()
{
    NetworkDatalinkSeq = 0;
    DatalinkNetworkSeq = 0;
    NetworkStatus = Enable;
    /* 装载信号 */
    signal(SIG_NETWORKLAYER_ENABLE, Network::sig_enable_handle);
}

Network::~Network()
{
    return;
}

void Network::seq_inc(seq_nr k)
{
    if(k < MAX_SHARE_SEQ) 
        k = k + 1; 
    else 
        k = 0;
}

layer_status Network::status()
{
    return NetworkStatus;
}

void Network::to_datalink_layer(packet *pkt)
{
    char fileName[50];

    sprintf(fileName, "network_datalink.share.%04d", NetworkDatalinkSeq);

    int fd;
    do
    {
        errno = 0;
        fd = open(fileName, O_WRONLY | O_CREAT);
    } while (fd < 0 && errno == EINTR);

    if (fd < 0)
    {
        cerr << "write file error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    flock(fd, LOCK_EX);

    int ret;
    do
    {
        errno = 0;
        ret = write(fd, pkt->data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "write " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    seq_inc(NetworkDatalinkSeq);
    return;
}

int Network::from_datalink_layer(packet *pkt)
{
    char fileName[50];
    
    sprintf(fileName, "datalink_network.share.%04d", DatalinkNetworkSeq);
    
    int fd;
    
    // 文件不存在 返回-1
    while (access(fileName, F_OK) < 0)
        return -1

    do
    {
        errno = 0;
        fd = open(fileName, O_RDONLY);
    } while (fd < 0 && errno == EINTR);

    if (fd < 0)
    {
        cerr << "read file error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    flock(fd, LOCK_EX);

    int ret;
    do
    {
        errno = 0;
        ret = read(fd, pkt->data, MAX_PKT);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        cerr << "read " << fileName << " error: " << strerror(errno) << endl;
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    seq_inc(DatalinkNetworkSeq);
    return 0;
}

void Network::network_layer_ready()
{
    pid_t pid;
    pid = getPidByName("datalink");
    kill(pid, SIG_NETWORKLAYER_READY);
}

void Network::sig_enable_handle(int signal)
{
    NetworkStatus = Enable;
}

void Network::sig_disable_handle(int signal)
{
    NetworkStatus = Disable;
}


//----------common function----------
int isEndPacket(const packet &Packet)
{
    for(int i = 0; i < MAX_PKT; ++i)
        if(Packet.data[i] != '\0')
            return 0;
    return 1;
}
void RemovePAD(packet &Packet, int &len)
{
    for(int i = MAX_PKT - 1; i; --i)
    {
        if(Packet.data[i] == PADbyte)
            --len;
        else
            break;
    }
    return;
}

void FillPAD(unsigned char * const Packet, const int startPosition)
{
    for(int i = startPosition; i < MAX_PKT; ++i)
        Packet[i] = PADbyte;
}
