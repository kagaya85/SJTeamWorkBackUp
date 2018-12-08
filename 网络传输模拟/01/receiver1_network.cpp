#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/common.h"
#include "../common/network.h"

using namespace std;

int main(const int argc, const char* argv[])
{
    if(argc != 2)
	{
		printf("Usage:%s [filename]\n", argv[0]);
		return 1;
	}

    int sourcefd = open(argv[1], O_WRONLY | O_CREATE | O_TRUNC);
    if(sourcefd < 0)
    {
        cerr << "File Create Failed: " << strerror(errno) << endl;
        return -1;
    }

    Network NetworkLayer;
    packet Filepacket;
    size_t _rs, _ws;
    while(TaihouDaisuki)
    {

    }

    return 0;
}
