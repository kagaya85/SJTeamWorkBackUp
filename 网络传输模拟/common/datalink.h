#ifndef DATALINK
#define DATALINK

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <cstring>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <errno.h>
#include "common.h"

#define TIMEOUT_LIMIT 10000 // ��λms 10s 

using namespace std;

struct TimerNode {
    clock_t nowTime;
    frame_kind fkind;
    seq_nr seq;
    TimerNode *next;
};

class Datalink {
private:
    static TimerNode *header;
    static event_type datalinkEvent;
    seq_nr NetworkDatalinkSeq;
    seq_nr DatalinkNetworkSeq; // ��·�㷢�������ķ������
    static unsigned int arrivedPacketNum;    // ����������Ѿ�����İ�����
    static unsigned int arrivedFrameNum;    // ����������Ѿ������֡����
    static seq_nr timeoutSeq;
    static layer_status NetworkStatus;   // �����״̬
    int msgid;
public:
    bool no_nak;
    Datalink();
    ~Datalink();
    void start_timer(seq_nr k);
    void stop_timer(seq_nr k);
    void enable_network_layer();
    void disable_network_layer();
    void start_ack_timer();
    void stop_ack_timer();
    void wait_for_event(event_type *event);
    void seq_inc(seq_nr k);
    void from_network_layer(packet *pkt);    // -1�쳣 0����
    void to_network_layer(packet *pkt);
    void from_physical_layer(frame *frm);
    void to_physical_layer(frame *frm);
    void send_data(seq_nr frame_nr, seq_nr frame_expected, packet bufferp[], int max_seq);
    void send_data(frame_kind fk, seq_nr frame_nr, seq_nr frame_expected, packet buffer[], int max_seq, int nr_bufs;
    seq_nr get_timeout_seq();
    static bool between(seq_nr a, seq_nr b, seq_nr c);
    /* �źŴ����� */
    static void sigalarm_handle(int signal);
    static void sig_frame_arrival_handle(int signal);
    static void sig_network_layer_ready_handle(int signal);
};

const char* To_Datalink_Dir = "./toDatalinkCache";
const char* To_Network_Dir = "./toNetworkCache";

#endif // DATALINK