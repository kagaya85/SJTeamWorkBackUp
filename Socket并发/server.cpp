#include <iostream>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

#define blockModeDefaultStep 0
#define MaxConnNum 1024
#define TaihouDaisuki 1
#define Recv_Max_buffer 100010
#define TimeStamplen 19
#define server_sid 1650275
#define MaxBuffSize 100000

const int totStep = 5;
const string std_string_snd[totStep] = {
		"StuNo", "pid", "TIME", "str", "end"
};
const int lim_Max = 67232;
const int lim_Min = 32768;

static void sig_child(int);

class Sock 
{
private:

	enum ExchangeStep{
		Sid, Pid, Time, String, Close
	};

	int sid[MaxConnNum];
	pid_t pid[MaxConnNum];
	char timestamp[MaxConnNum][TimeStamplen];
	int randstrlen[MaxConnNum];
	string strbuff[MaxConnNum];
	int curstep[MaxConnNum];		//accpet时对应位清Sid
	int writeEnable[MaxConnNum];	//accept时对应位置1
	char *tcpbuffer[MaxConnNum] = { NULL };
	int bufferlen[MaxConnNum];
	int bufferp[MaxConnNum]; 		//accept时对应位赋值0
	int fdList[MaxConnNum];
	// int seed;

	sockaddr_in servaddr;
	int listenfd;
	bool forkFlag;
	bool blockFlag;
	int port;
    string targetIpAddr;
	void clearState(int index);
public:
	Sock(const char* Ipaddr = NULL, const int Port = -1, const bool BlockFlag = false, const bool ForkFlag = false);
	~Sock();

	void fork_sock();
    void nofork_sock();
    void block_exchange(const int);
    int nonblock_exchange(const int, const int);
		//return： 0——continue 1——finish -1——write/read error -2——file error
    bool is_fork();
};

/* main 负责合法性检查 */
int main(int argc, const char** argv) 
{
	int port = -1;
	bool blockFlag = false, forkFlag = false;
	char ipaddr[20] = "0.0.0.0";

	if (argc < 2)
	{
		cerr << "Need parameter port at least" << endl;
		exit(EXIT_FAILURE);
	}

	// 读入参数
	string param;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] == '-')
		{
			param.assign(&argv[i][2]);
			if (param == "ip")
			{
				strcpy(ipaddr, argv[++i]);
			}
			else if (param == "port")
			{
				port = atoi(argv[++i]);
				if (port == 0)
				{
					cerr << "Port number error" << endl;
					exit(EXIT_FAILURE);
				}
			}
			else if (param == "block")
			{
				blockFlag = true;
			}
			else if (param == "nonblock")
			{
				blockFlag = false;
			}
			else if (param == "fork")
			{
				forkFlag = true;
			}
			else if (param == "nofork")
			{
				forkFlag = false;
			}
			else
			{
				cerr << "无法识别的参数：" << param << endl;
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			cerr << "无法识别的参数：" << argv[i] << endl;
			exit(EXIT_FAILURE);
		}
	}

    if(port < 0)
    {
        cerr << "Need parameter port at least" << endl;
		exit(EXIT_FAILURE);
    }

    // 不分支的情况下一定是非阻塞的
    if (!forkFlag)
        blockFlag = false;

	Sock sock(ipaddr, port, blockFlag, forkFlag);

    if(sock.is_fork())
    {
        sock.fork_sock();
    }
    else
	{
        sock.nofork_sock();
    }

	return 0;
}

Sock::Sock(const char* Ipaddr, const int Port, const bool BlockFlag, const bool ForkFlag)
{	
	srand((unsigned int)(time(NULL)));
	forkFlag = ForkFlag;
	blockFlag = BlockFlag;
	port = Port;
    targetIpAddr.assign(Ipaddr);
	for(int i = 0; i < MaxConnNum; i++)
		fdList[i] = -1;

	//==========sock==========
	int flag;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Create socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(EXIT_FAILURE);
	}
	if (!blockFlag)  // 非阻塞设置
	{
		flag = fcntl(listenfd, F_GETFL, 0);       //获取文件的flags值。
		fcntl(listenfd, F_SETFL, flag | O_NONBLOCK);     //设置成非阻塞模式；
	}

	int opt = 1;
	// sockfd为需要端口复用的套接字
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	
	// 设置IP地址
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, Ipaddr, &servaddr.sin_addr) <= 0) 
	{
		cerr << "inet_pton error for " << Ipaddr << endl;
		exit(EXIT_FAILURE);
	}
	servaddr.sin_port = htons(port);

	//==========bind==========
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        cerr << "bind socket error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}

Sock::~Sock()
{
	// if (fdList)
	// {	
	// 	// 关闭所有连接
	// 	for (int i = 0; i < fdNum; i++)
	// 	{
	// 		close(fdList[i]);
	// 	}

	// 	delete fdList;
	// }

	close(listenfd);
}

void Sock::fork_sock()
{
	int ret;
	struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    fd_set sockfds, readfds, writefds;

	FD_ZERO(&sockfds);
	FD_SET(listenfd, &sockfds);

	listen(listenfd, 1010);

	signal(SIGCHLD, sig_child);
		 //子进程结束发送信号给父进程，调用sig_child函数
	while (1)
	{
		readfds = sockfds;
		writefds = sockfds;
		ret = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		if(ret < 0)
		{
			if(errno == EINTR)
				continue;
			cerr << "Select error: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}
		if (FD_ISSET(listenfd, &readfds))
		{ // 监听句柄活动则建立新的连接,并fork子进程
			pid_t pid;
			int clientfd, flag;
			int i = 0;
			while (i < MaxConnNum)
			{
				if (fdList[i] < 0) // 找到一个空句柄位置
					break;
				else
					i++;
			}
			if (i >= MaxConnNum) // 超过最大连接数
				break;
			if((clientfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) < 0)
			{
				cerr << "Accept error: " << strerror(errno) << endl;
				exit(EXIT_FAILURE);
			}
			
			//解析客户端地址
			char ipbuff[INET_ADDRSTRLEN + 1] = {0};
			inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuff, INET_ADDRSTRLEN);
			uint16_t cli_port = ntohs(cliaddr.sin_port);
			cout << "Connection from " << ipbuff << ", port " << cli_port << endl;
			
			// 重置对应连接状态
			clearState(i);
			fdList[i] = clientfd;	// 加入句柄
			
			if(!blockFlag)
			{
				flag = fcntl(clientfd, F_GETFL, 0);              //获取文件的flags值。
				fcntl(clientfd, F_SETFL, flag | O_NONBLOCK);     //设置成非阻塞模式；
			}

			pid = fork();
			if (pid < 0)
			{
				cerr << "Fork error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
				exit(EXIT_FAILURE);
			}
			else if (pid == 0)
			{ // child process
				srand(getpid()); // 重新生成种子
				close(listenfd);	// 关闭监听句柄
				if (blockFlag)
				{   // block
					// 申请收发缓存
					tcpbuffer[blockModeDefaultStep] = new (nothrow) char[MaxBuffSize];
					if (tcpbuffer[blockModeDefaultStep] == NULL)
					{
						cerr << "New memory error" << endl;
						exit(EXIT_FAILURE);
					}
					bufferp[blockModeDefaultStep] = 0;
					block_exchange(clientfd);
					delete tcpbuffer[blockModeDefaultStep];
					exit(EXIT_SUCCESS);
				}
				else
				{ // nonblock
					fd_set testfds;
					FD_ZERO(&sockfds);
					FD_SET(clientfd, &sockfds);
					ret = 0;
					
					// 申请内存
					tcpbuffer[i] = new (nothrow) char[MaxBuffSize];
					if (tcpbuffer[i] == NULL)
					{
						cerr << "pid: " << getpid() <<" : New memory error" << endl;
						exit(EXIT_FAILURE);
					}
					clearState(i);
					
					while(ret != 1)
					{
						//cout << "Server [" << getpid() << "] start, client = " << clientfd << endl;
						testfds = sockfds;
						if(writeEnable[i] == true)
							ret = select(FD_SETSIZE, NULL, &testfds, NULL, NULL);
						else
							ret = select(FD_SETSIZE, &testfds, NULL, NULL, NULL);
						
						if(ret < 0 && errno == ECHILD)
							continue;

						//if(curstep[i] == Close && !writeEnable[i] && FD_ISSET(clientfd, &testfds))
						//{
						//	cout << "client close" << endl;
						//}
						ret = nonblock_exchange(clientfd, i);
						//cout << "Return: " << ret << endl;
						if(ret == 1)
						{
							// fdList[i] = -1;
							cout << "Server [" << getpid() << "] save success!" << endl;
							break;
						}
						else if(ret == -1)
						{
							// fdList[i] = -1;
							break;
						}
						else if(ret == -2)
						{
							break;
						}
					}
				}
				if(tcpbuffer[i])
				{
					delete tcpbuffer[i];
					tcpbuffer[i] = NULL;
				}
				cout << "Server [" << getpid() << "] return " << ret << endl;
				exit(EXIT_SUCCESS);
			}
			else 
			{
				fdList[i] = -1;
				close(clientfd);
				clearState(i);
			}
		}
		else
		{ // 其他句柄活动, 理论上执行不到这里
			cerr << "Something error!" << endl;
			exit(EXIT_FAILURE);
		}
	
	}

	// wait all child (服务器端还需要吗？)
    while(wait(NULL) > 0)
        ;
}

void Sock::nofork_sock()
{
    int ret;
	struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    fd_set sockfds, readfds, writefds;

	FD_ZERO(&sockfds);
	FD_SET(listenfd, &sockfds);

	listen(listenfd, 1000);

	while (1)
	{
		readfds = sockfds;
		writefds = sockfds;
		// FD_CLR(listenfd, writefds);
		// cout << "nofork select start" << endl;
		ret = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
		if (ret <= 0)
		{
			cerr << "Select error: " << strerror(errno) << endl;
			exit(EXIT_FAILURE);
		}
		for (int fd = 0; fd < FD_SETSIZE; fd++)
		{
			int i;
			for (i = 0; i < MaxConnNum; i++) // 将fd对应到下标i
				if (fdList[i] == fd)
					break;
			// 如果活跃的是监听socket，接入连接
			if (fd == listenfd && FD_ISSET(fd, &readfds))
			{
				int clientfd = 0, flag;
				i = 0;	// 从头开始找空位
				while(clientfd >= 0)
				{
					int ii;
					for (ii = i; i < MaxConnNum; i++)	// 每次寻找从前一次寻找结束的位置开始
					{
						if (fdList[ii] < 0) // 找到一个空句柄位置
							break;
						else
							ii++;
					}
					if (ii >= MaxConnNum) // 无空连接
						break;
					else
						i == ii;

					if ((clientfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
					{
						cerr << "Accept error: " << strerror(errno) << endl;
						exit(EXIT_FAILURE);
					}
				
					//解析客户端地址
					char ipbuff[INET_ADDRSTRLEN + 1] = {0};
					inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuff, INET_ADDRSTRLEN);
					uint16_t cli_port = ntohs(cliaddr.sin_port);
					cout << "connection from " << ipbuff << ", port " << cli_port << endl;

					// 申请收发缓存
					if(tcpbuffer[i] == NULL)
						tcpbuffer[i] = new (nothrow) char[MaxBuffSize];
					if (tcpbuffer[i] == NULL)
					{
						cerr << "New memory error" << endl;
						fdList[i] = -1;
						continue;
					}
					// 重置对应连接状态
					clearState(i);
					fdList[i] = clientfd;	// 加入句柄

					flag = fcntl(clientfd, F_GETFL, 0);		  //获取文件的flags值。
					fcntl(clientfd, F_SETFL, flag | O_NONBLOCK); //设置成非阻塞模式；

					/* 加入句柄集合 */
					FD_SET(clientfd, &sockfds);
				}
			}
			else if(fdList[i] != -1 && curstep[i] == Close && !writeEnable[i] && FD_ISSET(fd, &readfds))
			{	// 在close阶段读取，执行传输结束工作
				// cout << "Start save!" << endl;
				// cout << "curstep[i] == " << curstep[i] << endl;
				ret = nonblock_exchange(fd, i);
				// cout << "return: " << ret << endl;
				if(ret == 0)
				{
					cout << "server still open" << endl;
					continue;
				}
				else if(ret == 1)
				{
					FD_CLR(fdList[i], &sockfds);
					fdList[i] = -1;
					delete tcpbuffer[i];
					tcpbuffer[i] = NULL;
					clearState(i);
					cout << "client[" << pid[i] << "] close save success!" << endl;
					continue;
				}
				else if(ret == -1)
				{
					FD_CLR(fdList[i], &sockfds);
					fdList[i] = -1;
					delete tcpbuffer[i];
					tcpbuffer[i] = NULL;
					clearState(i);
					continue;
				}
				else if(ret == -2)
				{
					continue;
				}
				else
				{
					cerr << "Close error!" << endl;
					delete tcpbuffer[i];
					tcpbuffer[i] = NULL;
					exit(EXIT_FAILURE);
				}
			}
			else if (fdList[i] != -1 && (FD_ISSET(fd, &writefds) || FD_ISSET(fd, &readfds)))
			{ // 其他句柄活动, 则执行读写 nonblock

				// cout << "nonblock write start" << endl;
				//cout << "curstep[i] == " << curstep[i] << " writeenable = " << writeEnable[i] << endl;
				if(curstep[i] == Close && !writeEnable[i] && !FD_ISSET(fd, &readfds))
					continue;
				ret = nonblock_exchange(fd, i);
				// cout << "return: " << ret << endl;
				if(ret == -1)
				{
					FD_CLR(fdList[i], &sockfds);
					fdList[i] = -1;
					delete tcpbuffer[i];
					tcpbuffer[i] = NULL;
					clearState(i);
					continue;
				}
			}
		}
	}
}

void Sock::block_exchange(const int client_fd)
	//client_fd————对端socket
{
	string buffer_snd;
	char buffer_rec_c[Recv_Max_buffer];
	int buffer_rec_int;
	int buffer_snd_len, buffer_rec_len;

	ssize_t _rs, _ws;
	
	for(int i = 0; i < totStep; ++i)
	{
		//==========write==========
		if(i == String)
			randstrlen[blockModeDefaultStep] = rand() % lim_Max + lim_Min;
		bufferlen[blockModeDefaultStep] = std_string_snd[i].length() + 
										  (i == String ? strlen(to_string(randstrlen[blockModeDefaultStep]).c_str()) : 0) +
										  (i == Time || i == String);
	
		if(i == String)
			sprintf(tcpbuffer[blockModeDefaultStep], "%s%d", std_string_snd[i].c_str(), randstrlen[blockModeDefaultStep]);
		else
			strcpy(tcpbuffer[blockModeDefaultStep], std_string_snd[i].c_str());
		
		while(bufferp[blockModeDefaultStep] < bufferlen[blockModeDefaultStep])
		{
			_ws = write(client_fd, tcpbuffer[blockModeDefaultStep] + bufferp[blockModeDefaultStep], bufferlen[blockModeDefaultStep] - bufferp[blockModeDefaultStep]);
			if(_ws <= 0)
			{
				cerr << "write error" << endl;
				close(client_fd);
				return;
			}
			bufferp[blockModeDefaultStep] += _ws;
		}	
		bufferp[blockModeDefaultStep] = 0;
		writeEnable[blockModeDefaultStep] ^= 1;
		//==========read==========
		if(i == Close)
		{
			while(TaihouDaisuki)
			{
				_rs = read(client_fd, buffer_rec_c, 1);
				if(_rs == 0)
					break;
			}

			string filename = to_string(sid[blockModeDefaultStep]);
			filename.append(".").append(to_string(pid[blockModeDefaultStep])).append(".pid.txt");
			int wfd;
			if((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
			{
				cerr << "open error" << endl;
				close(client_fd);
				return;
			}
			write(wfd, &sid[blockModeDefaultStep], sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, &pid[blockModeDefaultStep], sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, timestamp[blockModeDefaultStep], TimeStamplen);
			write(wfd, "\n", 1);
			for(int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
				buffer_rec_c[j] = strbuff[blockModeDefaultStep][j];
			write(wfd, buffer_rec_c, randstrlen[blockModeDefaultStep]);
			write(wfd, "\n", 1);
			close(client_fd);
			close(wfd);
			return;
		}
		
		buffer_rec_len = ((i == Sid || i == Pid) ? sizeof(int) : ((i == Time) ? TimeStamplen : randstrlen[blockModeDefaultStep]));
		while(bufferp[blockModeDefaultStep] < buffer_rec_len)
		{
			_rs = read(client_fd, buffer_rec_c, buffer_rec_len - bufferp[blockModeDefaultStep]);
			if (_rs <= 0)
			{
				cerr << "read error" << endl;
				close(client_fd);
				return;
			}
			memcpy(tcpbuffer[blockModeDefaultStep] + bufferp[blockModeDefaultStep], buffer_rec_c, _rs);
			bufferp[blockModeDefaultStep] += _rs;
		}

		switch (i)
		{
			case Sid:
			{
				memcpy(&sid[blockModeDefaultStep], tcpbuffer[blockModeDefaultStep], sizeof(int));  
				sid[blockModeDefaultStep] = ntohl(sid[blockModeDefaultStep]);
				break;													
			}
			case Pid:
			{
				memcpy(&pid[blockModeDefaultStep], tcpbuffer[blockModeDefaultStep], sizeof(int));
				pid[blockModeDefaultStep] = ntohl(pid[blockModeDefaultStep]);
				break;
			}
			case Time:
			{
				memcpy(timestamp[blockModeDefaultStep], tcpbuffer[blockModeDefaultStep], TimeStamplen);
				break;
			}
			case String:
			{
				strbuff[blockModeDefaultStep].erase();
				for(int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
					strbuff[blockModeDefaultStep].push_back(tcpbuffer[blockModeDefaultStep][j]);
				break;
			}
			case Close: //It can't be run if it is correct
			{
				cerr << "There is something error in your program" << endl;
				break;
			}
			default:
				;	
		}
		bufferp[blockModeDefaultStep] = 0;
	}	
	close(client_fd);
}

int Sock::nonblock_exchange(const int client_fd, const int sockid)
{
	string buffer_snd;
	char buffer_rec_c[Recv_Max_buffer];
	int buffer_rec_int;
	int buffer_snd_len, buffer_rec_len;

	ssize_t _rs, _ws;
	int &i = curstep[sockid];

	if(writeEnable[sockid] == 1) // write
	{
		if(bufferp[sockid] == 0) //first time in, reset the buffer
		{
			if(i == String)
				randstrlen[sockid] = rand() % lim_Max + lim_Min;
			bufferlen[sockid] = std_string_snd[i].length() + 
								(i == String ? strlen(to_string(randstrlen[sockid]).c_str()) : 0) + 
								(i == Time || i == String);
			
			if(i == String)
				sprintf(tcpbuffer[sockid], "%s%d", std_string_snd[i].c_str(), randstrlen[sockid]);
			else
				strcpy(tcpbuffer[sockid], std_string_snd[i].c_str());
		}
		
		do
		{
			errno = 0;
			_ws = write(client_fd, tcpbuffer[sockid] + bufferp[sockid], bufferlen[sockid] - bufferp[sockid]);
		}while(_ws < 0 && errno == EINTR); //被信号打断， 则重新write

		if(_ws <= 0)
		{
			if (_ws < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
				return 0;

			cerr << "write error: " << strerror(errno) << endl;
			cerr << "error sockid: " << sockid << endl;
			cerr << "bufferp[sockid]: " << bufferp[sockid] << endl;
			close(client_fd);
			return -1;
		}
		bufferp[sockid] += _ws;
		if(bufferp[sockid] == bufferlen[sockid])
		{
			bufferp[sockid] = 0;
			writeEnable[sockid] ^= 1;
		}
	}
	else // read
	{
		if(i == Close)
		{
			do
			{
				errno = 0;
				_rs = read(client_fd, buffer_rec_c, 1);
			}while(_rs < 0 && errno == EINTR); //被信号打断， 则重新read

			if(_rs > 0)
			{
				cout << "what do you fuxking read??? read = " << _rs << endl;
				close(client_fd);
			 	return -1;
			}

			string filename = to_string(sid[sockid]);
			filename.append(".").append(to_string(pid[sockid])).append(".pid.txt");
			int wfd;
			if((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
			{
				cerr << "open error" << endl;
				return -2;
			}
			write(wfd, &sid[sockid], sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, &pid[sockid], sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, timestamp[sockid], TimeStamplen);
			write(wfd, "\n", 1);
			for(int j = 0; j < randstrlen[sockid]; ++j)
				buffer_rec_c[j] = strbuff[sockid][j];
			write(wfd, buffer_rec_c, randstrlen[sockid]);
			write(wfd, "\n", 1);
			close(client_fd);
			close(wfd);
			return 1;
		}
		
		buffer_rec_len = ((i == Sid || i == Pid) ? sizeof(int) : ((i == Time) ? TimeStamplen : randstrlen[sockid]));

		do
		{
			errno = 0;
			_rs = read(client_fd, buffer_rec_c, buffer_rec_len - bufferp[sockid]);
		} while (_rs < 0 && errno == EINTR); //被信号打断， 则重新read

		if(_rs <= 0) 
		{
			if (_rs < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
				return 0;
			cerr << "read error" << endl;
			close(client_fd);
			return -1;
		}
		memcpy(tcpbuffer[sockid] + bufferp[sockid], buffer_rec_c, _rs);
		bufferp[sockid] += _rs;
		if(bufferp[sockid] < buffer_rec_len)
			return 0;
			
		switch (i)
		{
			case Sid:
			{
				memcpy(&sid[sockid], tcpbuffer[sockid], sizeof(int));
				//cout << "read Sid from scokid " << sockid << endl;  
				sid[sockid] = ntohl(sid[sockid]);
				break;													
			}
			case Pid:
			{
				memcpy(&pid[sockid], tcpbuffer[sockid], sizeof(int));
				//cout << "read Pid from scokid " << sockid << endl;  
				pid[sockid] = ntohl(pid[sockid]);
				break;
			}
			case Time:
			{
				memcpy(timestamp[sockid], tcpbuffer[sockid], TimeStamplen);
				//cout << "read Time from scokid " << sockid << endl;  
				//cout << "server [" << getpid() << "] get timestampe len = " << bufferp[sockid] << endl;
				//cout << "server [" << getpid() << "] get timestamp = " << timestamp[sockid] << endl;
				break;
			}
			case String:
			{
				//cout << "read String from scokid " << sockid << endl; 
				strbuff[sockid].erase();
				for(int j = 0; j < randstrlen[sockid]; ++j)
					strbuff[sockid].push_back(tcpbuffer[sockid][j]);
				//cout << sockid << ": string backup complete" << endl;
				break;
			}
			case Close: //It can't be run if it is correct
			{
				cerr << "There is something error in your program" << endl;
				break;
			}
			default:
				;	
		}
		bufferp[sockid] = 0;
		writeEnable[sockid] ^= 1;
		++i;
	}
	return 0;
}

bool Sock::is_fork()
{
    return forkFlag;
}

static void sig_child(int signal)
{
	pid_t pid;
	int stat, i;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		//printf("child %d terminated.\n", pid);
		;
}

void Sock::clearState(int index)
{
	// 重置对应连接状态
	curstep[index] = Sid;		//accpet时对应位置Sid
	writeEnable[index] = 1;		//accept时对应位置1
	bufferp[index] = 0;
}
