#ifndef COMMON
#define COMMON

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>

#define SIG_CHSUM_ERR (SIGRTMIN + 1)
#define SIG_FRAME_ARRIVAL (SIGRTMIN + 2)
#define SIG_NETWORKLAYER_READY (SIGRTMIN + 3)
#define SIG_NETWORKLAYER_ENABLE (SIGRTMIN + 4)
#define SIG_NETWORKLAYER_DISABLE (SIGRTMIN + 5)

#define MAX_PKT 1024
#define MSGBUFF_SIZE 1050
#define IPC_KEY 114514
#define FROM_PHYSICAL 1
#define FROM_DATALINK 2
#define inc(k) if(k<MAX_SEQ) k = k + 1; else k = 0;

#define MAX_SHARE_SEQ 999

#define SENDER  1
#define RECEIVER  0
#define TaihouDaisuki 1

pid_t getPidByName(const char * const task_name);

typedef unsigned int seq_nr;    // 发送序号

class packet {
public:
    unsigned char data[MAX_PKT];
    // packet();
    // ~packet();
    void operator=(const packet &p);
};

typedef enum {
    DataFrame,
    AckFrame,
    NakFrame
} frame_kind;

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
    frame_kind kind;// 帧类型
    seq_nr  seq;    // 发送序号
    seq_nr  ack;    // 接受序号
    packet  info;   // 数据
} frame;

struct Message {
    long int msg_type;
    unsigned char data[MSGBUFF_SIZE];
};

#endif // COMMON
