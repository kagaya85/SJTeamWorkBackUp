#ifndef COMMON
#define COMMON

#include <signal.h>
#include <unistd.h>

#define SIG_CHSUM_ERR __SIGRTMIN + 1
#define SIG_FRAME_ARRIVAL __SIGRTMIN + 2
#define SIG_NETWORK_LAYER_READY __SIGRTMIN + 3
#define SIG_NETWORK_LAYER_ENABLE __SIGRTMIN + 4
#define SIG_NETWORK_LAYER_DISABLE __SIGRTMIN + 5
#define MAX_PKT 1024

#define inc(k) if(k<MAX_SEQ)

pid_t getPidByName(char *task_name);

typedef unsigned int seq_nr;    // 发送序号

typedef strcut {
    unsigned char data[MAX_PKT];
};

typedef enum {
    dataFrame,
    ackFrame,
    nakFrame
} frame_type;

typedef enum {
    frame_arrival,
    cksum_err,
    timeout,
    network_layer_ready,
    ack_timeout
} event_type;

typedef struct {
    frame_kind kind,
    seq_nr  seq,
    seq_nr  ack,
    packet  info;
} frame;

#endif // COMMON