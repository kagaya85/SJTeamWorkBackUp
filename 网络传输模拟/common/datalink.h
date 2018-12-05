#ifndef DATALINK
#define DATALINK

#define TIMEOUT_LIMIT 10000 // 单位ms 10s 
#include <time.h>
#include "common.h"

struct TimerNode {
    clock_t nowTime,
    frame_type ftype,
    seq_nr seq,
    TimerNode *next
};

class Datalink {
private:
    TimerNode *header;
public:
    Datalink();
    ~Datalink();
    void start_timer(seq_nr k);
    void stop_timer(seq_nr k);
    void enable_network_layer();
    void disable_network_layer();
    void from_network_layer(packet *pkt);
    void start_ack_timer();
    void stop_ack_timer();
    // signal
    void add_a_second(int signal);
    void timeout_handle(int signal);
};

#endif // DATALINK