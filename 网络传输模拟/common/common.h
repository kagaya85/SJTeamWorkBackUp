#ifndef COMMON
#define COMMON

#include <signal.h>
#include <unistd.h>

#define SIG_CHSUM_ERR (SIGRTMIN + 1)
#define SIG_FRAME_ARRIVAL (SIGRTMIN + 2)
#define SIG_NETWORK_LAYER_READY (SIGRTMIN + 3)
#define SIG_NETWORK_LAYER_ENABLE (SIGRTMIN + 4)
#define SIG_NETWORK_LAYER_DISABLE (SIGRTMIN + 5)
#define MAX_PKT 1024

#define inc(k) if(k<MAX_SEQ) k = k + 1; else k = 0;

pid_t getPidByName(char *task_name);

typedef unsigned int seq_nr;    // 发送序号

typedef strcut {
    unsigned char data[MAX_PKT];
} packet;

typedef enum {
    dataFrame,
    ackFrame,
    nakFrame
} frame_type;

typedef enum {
    no_event,
    frame_arrival,
    cksum_err,
    timeout,
    network_layer_ready,
    ack_timeout
} event_type;

typedef enum {
    Enable,
    Disable
} layer_status;
typedef struct {
    frame_kind kind,
    seq_nr  seq,
    seq_nr  ack,
    packet  info;
} frame;

#endif // COMMON