#ifndef DATALINK
#define DATALINK

#include <stdio.h>
#include <time.h>
#include "common.h"

#define TIMEOUT_LIMIT 10000 // 单位ms 10s 
#define MAX_SHARE_SEQ 999   // 共享文件最大序号

using namespace std;

struct TimerNode {
    clock_t nowTime,
    frame_type ftype,
    seq_nr seq,
    TimerNode *next
};

class Datalink {
private:
    TimerNode *header;
    event_type datalinkEvent;
    seq_nr NetworkDatalinkSeq;
    // seq_nr DatalinkPhysicalSeq; // 链路层到物理层的发送序号
    int arrivedPacketNum;    // 来自网络层已经到达的包数量
    int arrivedFrameNum;    // 来自物理层已经到达的帧数量
    layer_status networkStatus;   // 网络层状态
public:
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
    void from_network_layer(packet *pkt);    // -1异常 0正常
    void to_network_layer(packet *pkt);
    void from_physical_layer(frame *frm);
    void to_physical_layer(frame *frm);

    /* 信号处理函数 */
    void sigalarm_handle(int signal);
    void sig_frame_arrival_handle(int signal);
    void sig_networklayer_ready_handle(int signal);
};

#endif // DATALINK