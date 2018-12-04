#include <signal.h>
#include <unistd.h>

#define SIG_CHSUM_ERR __SIGRTMIN + 1
#define SIG_FRAME_ARRIVAL __SIGRTMIN + 2
#define SIG_NETWORK_LAYER_READY __SIGRTMIN + 3
#define SIG_NETWORK_LAYER_ENABLE __SIGRTMIN + 4
#define SIG_NETWORK_LAYER_DISABLE __SIGRTMIN + 5

pid_t getPidByName(char *task_name);
