#ifndef DATALINK
#define DATALINK

#include <time.h>
#include "common.h"

#define TIMEOUT_LIMIT 10000 // 单位ms 10s 
#define MAX_SHARE_SEQ 999   // 共享文件最大序号

struct TimerNode {
    clock_t nowTime,
    frame_type ftype,
    seq_nr seq,
    TimerNode *next
};

class Datalink {
private:
    TimerNode *header;
    event_type dataLinkEvent;
    seq_nr NetworkDatalinkSeq;
    seq_nr DatalinkPhysicalSeq;
    layer_status networkStatus;   // 网络层状态
public:
    Datalink();
    ~Datalink();
    void start_timer(seq_nr k);
    void stop_timer(seq_nr k);
    void enable_network_layer();
    void disable_network_layer();
    int from_network_layer(packet *pkt);
    void start_ack_timer();
    void stop_ack_timer();
    void wait_for_event(event_type *event);
    void seq_inc(seq_nr k);
    // signal
    void add_a_second(int signal);
    void timeout_handle(int signal);
};

#endif // DATALINK