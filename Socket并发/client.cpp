#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>

#define DEFAULT_CON_NUM 100
#define MaxConnNum 1024
#define blockModeDefaultStep 0
#define totStep 5
#define Recv_Max_buffer 1010
#define Snd_Max_buffer 100010
#define TimeStampLen 20
#define TaihouDaisuki 1
#define MaxBuffSize 100000

using namespace std;

static void sig_child(int);
static void IncRunTime(int);

int run_Time = 0;

class Sock {
private:
	enum ExchangeStep {
		Sid, Pid, Time, String, Close
	};
	enum Status {
		Idle, Connecting, Connected, Completed
	};
	const string stdstr[totStep] = {
		"StuNo", "pid", "TIME", "str", "end"
	};
	const int sid = 1650275;
	pid_t pid[MaxConnNum];
	char timestamp[MaxConnNum][TimeStampLen];
	int randstrlen[MaxConnNum];
	string strbuff[MaxConnNum];
	int curstep[MaxConnNum];
	int writeEnable[MaxConnNum];	
	char *tcpbuffer[MaxConnNum] = { NULL };
	int bufferlen[MaxConnNum];
	int bufferp[MaxConnNum]; 		//acceptʱ��Ӧλ��ֵ0

	int WaitTime[MaxConnNum];		//ÿ��socket��connectǰ����ȴ���ʱ��

	// int seed;
	const int Limit = 256;

	sockaddr_in servaddr;
	int *fdList;
	int fdStatus[MaxConnNum] = { 0 };
	bool forkFlag;
	bool blockFlag;
	int port;
	int fdNum;
    string targetIpAddr;
	void clearState(int index);
public:
	Sock(const char* Ipaddr = NULL, const int Port = -1, const bool BlockFlag = false, const bool ForkFlag = false, const int conNum = DEFAULT_CON_NUM);
	~Sock();
	void get_time_str(char* const timestr);
	void fork_sock();
    void nofork_sock();
    void block_connect(const int fd);
    void nonblock_connect(const int fd);
    void nonblock_connect();

    int block_exchange(const int);
		//return: 0-----failed, wait for reconnect  1----finish  
    int nonblock_exchange(const int, const int);
		//return��0����continue 1����finish -1����error, wait for reconnect -2����file error

    bool is_fork();
};

/* main ����Ϸ��Լ�� */
int main(int argc, const char** argv)
{
	int port = -1;
	int conNum = DEFAULT_CON_NUM;
	bool blockFlag = false, forkFlag = false;
	char ipaddr[20] = { 0 };

	if (argc < 3)
	{
		cerr << "Need more than two parameter" << endl;
		exit(EXIT_FAILURE);
	}

	// �������
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
			else if (param == "num")
			{
				conNum = atoi(argv[++i]);
				if (port == 0)
				{
					cerr << "Connect number error" << endl;
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
				cerr << "�޷�ʶ��Ĳ�����" << param << endl;
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			cerr << "�޷�ʶ��Ĳ�����" << argv[i] << endl;
			exit(EXIT_FAILURE);
		}
	}

    if(port < 0)
    {
        cerr << "Need parameter port number" << endl;
		exit(EXIT_FAILURE);
    }
	if(strlen(ipaddr) == 0)
	{
		cerr << "Need parameter ip address" << endl;
		exit(EXIT_FAILURE);
	}

    // ����֧�������һ���Ƿ�������
    if (!forkFlag)
        blockFlag = false;

	Sock sock(ipaddr, port, blockFlag, forkFlag, conNum);

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

Sock::Sock(const char* Ipaddr, const int Port, const bool BlockFlag, const bool ForkFlag, const int conNum)
{
	srand((unsigned int)(time(NULL)));

	forkFlag = ForkFlag;
	blockFlag = BlockFlag;
	port = Port;
	fdNum = conNum;
    targetIpAddr.assign(Ipaddr);

	fdList = new(nothrow) int[fdNum];
	if (!fdList)
	{
		cerr << "fdList new error" << endl;
		exit(EXIT_FAILURE);
	}

	int flag;
	for (int i = 0; i < fdNum; i++)
	{
		if ((fdList[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			cerr << "Create socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
			exit(EXIT_FAILURE);
		}
		if (!blockFlag)  // ����������
		{
			flag = fcntl(fdList[i], F_GETFL, 0);       //��ȡ�ļ���flagsֵ��
			fcntl(fdList[i], F_SETFL, flag | O_NONBLOCK);     //���óɷ�����ģʽ��
		}
	}

    // ����IP��ַ
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, Ipaddr, &servaddr.sin_addr) <= 0) 
	{
		cerr << "inet_pton error for " << Ipaddr << endl;
		exit(EXIT_FAILURE);
	}
	servaddr.sin_port = htons(port);
}

Sock::~Sock()
{
	if (fdList)
	{	
		// �ر���������
		// for (int i = 0; i < fdNum; i++)
		// {
		// 	close(fdList[i]);
		// }
		delete fdList;
	}

}

void Sock::fork_sock()
{
    pid_t pid;

	signal(SIGCHLD, sig_child);
		 //�ӽ��̽��������źŸ������̣�����sig_child����
	for(int i = 0; i < fdNum; i++)
    {
        pid = fork();
        if(pid < 0)
        {
			cerr << "Fork error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {   // child process
            int sockfd = fdList[i];
            int ret;
			int status = Connected;
			srand(getpid());	// ������������
			
			if(blockFlag)
            {   // block
				while(true)
				{
					if (status == Idle)
					{
						if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
						{
							cerr << "Create socket error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
							exit(EXIT_FAILURE);
						}
					}

					block_connect(sockfd);
					// �����շ�����
					tcpbuffer[blockModeDefaultStep] = new (nothrow) char[MaxBuffSize];
					if (tcpbuffer[blockModeDefaultStep] == NULL)
					{
						cerr << "New memory error" << endl;
						fdList[blockModeDefaultStep] = -1;
						break;
					}
					clearState(blockModeDefaultStep);
					// bufferp[blockModeDefaultStep] = 0;
					ret = block_exchange(sockfd);
					if (ret == 0)	// exchange error
					{
						status = Idle;
						continue;
					}
					else if(ret == 1) // OK
					{	
						break;
					}
				}
			}
            else
            {   // nonblock
			    int flag;
				fd_set sockfds, testfds;
				FD_ZERO(&sockfds);
				FD_SET(sockfd, &sockfds);
    			struct timeval timeout, tmot = {0 , 100};
				// �����շ�����
				tcpbuffer[i] = new (nothrow) char[MaxBuffSize];
				if (tcpbuffer[i] == NULL)
				{
					cerr << "pid " << getpid() << " : New memory error" << endl;
					exit(EXIT_FAILURE);
				}
				// ���ö�Ӧ����״̬
				clearState(i);
				
				while (true)
				{
					timeout = tmot;
					if (status == Idle)
					{
						if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
						{
							cerr << "Create socket error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
							exit(EXIT_FAILURE);
						}
						flag = fcntl(sockfd, F_GETFL, 0);       //��ȡ�ļ���flagsֵ��
						fcntl(sockfd, F_SETFL, flag | O_NONBLOCK);     //���óɷ�����ģʽ��
						FD_SET(sockfd, &sockfds);
					}
					testfds = sockfds;

					const int MaxTime = 15; //����ȴ�ʱ�����ޣ��ݶ�15s
					sleep(rand() % MaxTime); //����ȴ�һ��ʱ���ٷ���connect
					nonblock_connect(sockfd);

					if((ret = select(FD_SETSIZE, NULL, &testfds, NULL, NULL)) >= 0)
					{
						nonblock_connect(sockfd);
						if(errno == EISCONN)
					    	cout << "Pid: " << getpid() <<" connected to " << targetIpAddr << endl;
					}
					else if (ret < 0)
					    cerr << strerror(errno) << endl;

					for (ret = 0; ret != 1;)
					{
						ret = nonblock_exchange(sockfd, i);
						if (ret == 0)
						{
							testfds = sockfds;
							if (writeEnable[i])
								select(FD_SETSIZE, NULL, &testfds, NULL, NULL);
							else
								select(FD_SETSIZE, &testfds, NULL, NULL, NULL);
						}
						else if (ret == -1)
						{	// need to reconnect
							status = Idle;
							FD_CLR(sockfd, &sockfds);
							break;
						}
						else if (ret == -2)
						{	// open file error
							sleep(5);
						}
						else 	// ret == 1 Complete!
						{
							if(tcpbuffer[i])
							{
								delete tcpbuffer[i];
								tcpbuffer[i] = NULL;
							}
							cout << "Client [" << getpid() << "] exit success" << endl;
							exit(EXIT_SUCCESS);
						}
					}
				}
            }
			if(tcpbuffer[i])
				delete tcpbuffer[i];
			exit(EXIT_SUCCESS);
        }
		else
			close(fdList[i]);	// �����̹رվ��
    }

    // wait all child
    while(wait(NULL) > 0)
        sleep(1);
}

void Sock::nofork_sock()
{	// nonblock
	int ret;
	fd_set sockfds, testfds;
    struct timeval timeout, tmot = {0 , 500000};

    // FD_ZERO(&sockfds);
    // for(int i = 0; i < fdNum; i++)
    //     FD_SET(fdList[i], &sockfds);
    // testfds = sockfds;

	int i = 0;
	
	const int MaxTime = 15; //����ȴ�ʱ�����ޣ��ݶ�15s
	for(int i = 0; i < fdNum; ++i)
	{
		WaitTime[i] = rand() % MaxTime;
		fdStatus[i] = Idle;
	}

	
	signal(SIGALRM, IncRunTime); //���ö�ʱ��
	alarm(1);

	while (true)
	{
		int flag;
		FD_ZERO(&sockfds);
		FD_SET(fdList[i], &sockfds);
		timeout = tmot;

		//����ʱ���ӳٵ����ⷢconnect
		nonblock_connect();

		if(fdStatus[i] == Connecting)
		{
			testfds = sockfds;
			do{
				errno == 0;
				ret = select(FD_SETSIZE, NULL, &testfds, NULL, &timeout);
			} while(ret < 0 && errno == EINTR);
			//cout << "return : " << ret << endl;
			nonblock_connect(fdList[i]);
			if(errno == EISCONN)	// ȷ���Ƿ�������
			{
				cout << "Pid: " << getpid() <<" connected to " << targetIpAddr << endl;
				// �����շ�����
				if(tcpbuffer[i] == NULL)
				{
					tcpbuffer[i] = new (nothrow) char[MaxBuffSize];
					if (tcpbuffer[i] == NULL)
					{
						cerr << "New memory error" << endl;
						fdList[i] = -1;
						exit(EXIT_FAILURE);
					}
				}
				// ���ö�Ӧ����״̬
				clearState(i);
				fdStatus[i] = Connected;
			}
			else if (ret < 0)	
			{
				close(fdList[i]);
				if ((fdList[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				{
					cerr << "Create socket error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
					exit(EXIT_FAILURE);
				}
				flag = fcntl(fdList[i], F_GETFL, 0);		   //��ȡ�ļ���flagsֵ��
				fcntl(fdList[i], F_SETFL, flag | O_NONBLOCK); //���óɷ�����ģʽ��

				fdStatus[i] = Idle;
				WaitTime[i] = run_Time + rand() % MaxTime;
			}
		}
		
		if (fdStatus[i] == Connected)
		{
			testfds = sockfds;
			if (writeEnable[i])
				ret = select(FD_SETSIZE, NULL, &testfds, NULL, &timeout);
			else
				ret = select(FD_SETSIZE, &testfds, NULL, NULL, &timeout);
			if (ret > 0)
			{
				//cout << "cur i = " << i << endl;
				ret = nonblock_exchange(fdList[i], i);
				if (ret == 0)
				{
					// do nothing
				}
				else if (ret == -1)
				{ // need to reconnect
					// fdStatus[i] == Idle;
					int flag;
					if ((fdList[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						cerr << "Create socket error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
						exit(EXIT_FAILURE);
					}
					flag = fcntl(fdList[i], F_GETFL, 0);		  //��ȡ�ļ���flagsֵ��
					fcntl(fdList[i], F_SETFL, flag | O_NONBLOCK); //���óɷ�����ģʽ��
					//nonblock_connect(fdList[i]);
					
					// ���ö�Ӧ����״̬
					curstep[i] = Sid;   //connectʱ��Ӧλ��Sid
					writeEnable[i] = 1; //connectʱ��Ӧλ��1
					fdStatus[i] = Idle;
					WaitTime[i] = run_Time + rand() % MaxTime; //�����ط�connect��ʱ
					bufferp[i] = 0;
				}
				else if (ret == -2)
				{ // open file error
					// sleep(5);
					if(tcpbuffer[i])
					{
						delete tcpbuffer[i];
						tcpbuffer[i] = NULL;
					}
					exit(EXIT_FAILURE);
				}
				else // ret == 1 Completed!
				{
					if (tcpbuffer[i])
					{
						delete tcpbuffer[i];
						tcpbuffer[i] = NULL;
					}
					fdStatus[i] = Completed;
					cout << "Client fd[" << fdList[i] << "] save success!" << endl;
				}
			}
		}
		//cout << "i = " << i << endl;
		/* ������+1 */
		int starti = i;
		// cout << "starti: " << i << endl;
		while(fdStatus[(i + 1)%fdNum] == Completed)
		{
			i = (i + 1) % fdNum;
			if (i == starti) //	��������ɵľ�� ����һȦ�����ʾȫ�����
			{
				for(i = 0; i < fdNum; i++)
				{
					close(fdList[i]);	// ������ر����о��
				}
				exit(EXIT_SUCCESS);
			}
		}
		i = (i + 1)%fdNum;
	}
}
/* ���������ӵ������ */
void Sock::nonblock_connect(const int fd)
{
    connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
}
/* �������������fdList�е����о�� */
void Sock::nonblock_connect()
{
    fd_set connfds, testfds;

	for(int i = 0; i < fdNum; ++i)
		if(fdStatus[i] == Idle && WaitTime[i] <= run_Time)
		{
			connect(fdList[i], (struct sockaddr*)&servaddr, sizeof(servaddr));
			fdStatus[i] = Connecting;
		}
			
}

void Sock::block_connect(const int fd)
{
	const int MaxTime = 15; //����ȴ�ʱ�����ޣ��ݶ�15s
	sleep(rand() % MaxTime); //����ȴ�һ��ʱ���ٷ���connect

    int ret;
    ret = connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret == 0)
        cout << "Pid: " << getpid() <<" connected to " << targetIpAddr << endl;
    else
		cerr << strerror(errno) << endl;
}

int Sock::block_exchange(const int fd)
{
	char buffer_rec[Recv_Max_buffer];
	char buffer_snd[Snd_Max_buffer];
	int buffer_rec_len;

	ssize_t _rs, _ws;
	for(int i = 0; i < totStep; ++i)
	{
		//cout << "client [" << getpid() << "] cur step is " << i << endl;
		//==========read==========
		if(bufferp[blockModeDefaultStep] == 0) //first time in, reset the buffer
			bufferlen[blockModeDefaultStep] = stdstr[i].length() + 
			                      	          (i == String ? strlen("99999") : 0) +
			                                  (i == Time || i == String);
		while(TaihouDaisuki)
		{
			_rs = read(fd, buffer_rec, bufferlen[blockModeDefaultStep] - bufferp[blockModeDefaultStep]);
			if (_rs == 0)
			{
				cerr << "network error" << endl;
				close(fd);
				return 1;
			}
			else if (_rs < 0)
			{
				cerr << "read error" << endl;
				close(fd);
				return 0;
			}

			memcpy(tcpbuffer[blockModeDefaultStep] + bufferp[blockModeDefaultStep], buffer_rec, _rs);
			bufferp[blockModeDefaultStep] += _rs;
			if (i == String)
			{
				if (buffer_rec[_rs - 1] != '\0')
					continue;

				randstrlen[blockModeDefaultStep] = 0;
				for (int j = 3; (randstrlen[blockModeDefaultStep] < 10000) && (tcpbuffer[blockModeDefaultStep][j] >= '0' && tcpbuffer[blockModeDefaultStep][j] <= '9'); ++j)
					randstrlen[blockModeDefaultStep] = randstrlen[blockModeDefaultStep] * 10 + tcpbuffer[blockModeDefaultStep][j] - '0';
				tcpbuffer[blockModeDefaultStep][3] = '\0';
				for (int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
					strbuff[blockModeDefaultStep].push_back(rand() % Limit);
			}
			else if (bufferp[blockModeDefaultStep] < bufferlen[blockModeDefaultStep])
				continue;

			tcpbuffer[blockModeDefaultStep][bufferp[blockModeDefaultStep]++] = '\0';
			// cout << "client [" << getpid() << "] std string is " << stdstr[i] << endl;
			// cout << "client [" << getpid() << "] rec string is " << tcpbuffer[blockModeDefaultStep] << endl;
			if (strcmp(tcpbuffer[blockModeDefaultStep], stdstr[i].c_str()))
			{
				cerr << "information exchange error" << endl;
				close(fd);
				return 0;
			}
			bufferp[blockModeDefaultStep] = 0;
			writeEnable[blockModeDefaultStep] ^= 1;
			break;
		}
		//==========write==========
		if(bufferp[blockModeDefaultStep] == 0)
		{
			int tmp;
			switch (i)
			{
				case Sid:
				{
					tmp = htonl(sid);
					bufferlen[blockModeDefaultStep] = sizeof(int);
					memcpy(tcpbuffer[blockModeDefaultStep], &tmp, bufferlen[blockModeDefaultStep]);
					break;
				}
				case Pid:
				{
					pid_t tpid = getpid();
					pid[blockModeDefaultStep] = (forkFlag == false) ? ((tpid << 16) + fd) : tpid;
					tmp = htonl(pid[blockModeDefaultStep]);
					bufferlen[blockModeDefaultStep] = sizeof(int);
					memcpy(tcpbuffer[blockModeDefaultStep], &tmp, bufferlen[blockModeDefaultStep]);
					break;
				}
				case Time:
				{
					get_time_str(timestamp[blockModeDefaultStep]);
					bufferlen[blockModeDefaultStep] = TimeStampLen - 1;
					memcpy(tcpbuffer[blockModeDefaultStep], timestamp[blockModeDefaultStep], bufferlen[blockModeDefaultStep]);
					break;
				}
				case String:
				{
					for(int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
						tcpbuffer[blockModeDefaultStep][j] = strbuff[blockModeDefaultStep][j];
					bufferlen[blockModeDefaultStep] = randstrlen[blockModeDefaultStep];
					break;
				}
				case Close:
				{
					close(fd);

					/*string filename = to_string(sid);
					filename.append(".").append(to_string(pid[blockModeDefaultStep])).append(".pid.txt");
					int wfd;
					if ((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
					{
						cerr << "open error" << endl;
						return 0;
					}
					write(wfd, &sid, sizeof(int));
					write(wfd, "\n", 1);
					write(wfd, &pid[blockModeDefaultStep], sizeof(int));
					write(wfd, "\n", 1);
					write(wfd, timestamp[blockModeDefaultStep], TimeStampLen - 1);
					write(wfd, "\n", 1);
					for (int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
						buffer_snd[j] = strbuff[blockModeDefaultStep][j];
					write(wfd, buffer_snd, randstrlen[blockModeDefaultStep]);
					write(wfd, "\n", 1);
					close(wfd);*/
					
					if (opendir("./txt") == NULL)
					{
						mode_t mode = umask(0);
						mkdir("./txt", 0777);
					}
					string filename = "./txt/";
					filename.append(to_string(sid)).append(".").append(to_string(pid[blockModeDefaultStep])).append(".pid.txt");
					int wfd;
					if ((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
					{
						cerr << "open error" << endl;
						return 0;
					}
					char int_tmp_buff[10];
					sprintf(int_tmp_buff, "%d", sid);
					write(wfd, int_tmp_buff, strlen(int_tmp_buff));
					write(wfd, "\n", 1);
					sprintf(int_tmp_buff, "%d", pid[blockModeDefaultStep]);
					write(wfd, int_tmp_buff, strlen(int_tmp_buff));
					write(wfd, "\n", 1);
					write(wfd, timestamp[blockModeDefaultStep], TimeStampLen - 1);
					write(wfd, "\n", 1);
					for (int j = 0; j < randstrlen[blockModeDefaultStep]; ++j)
						buffer_snd[j] = strbuff[blockModeDefaultStep][j];
					write(wfd, buffer_snd, randstrlen[blockModeDefaultStep]);
					write(wfd, "\n", 1);
					close(wfd);

					return 1;
				}
				default:
					;
			}
		}
		while(bufferp[blockModeDefaultStep] < bufferlen[blockModeDefaultStep])
		{
			_ws = write(fd, tcpbuffer[blockModeDefaultStep] + bufferp[blockModeDefaultStep], bufferlen[blockModeDefaultStep] - bufferp[blockModeDefaultStep]);
			if (_ws == 0)
			{
				cerr << "network error" << endl;
				close(fd);
				return 1;
			}
			else if (_ws < 0)
			{
				cerr << "write error" << endl;
				close(fd);
				return 0;
			}
			bufferp[blockModeDefaultStep] += _ws;
		}
		
		bufferp[blockModeDefaultStep] = 0;
		writeEnable[blockModeDefaultStep] ^= 1;
	}
	return 0;
}

int Sock::nonblock_exchange(const int fd, const int sockid)
{
    char buffer_rec[Recv_Max_buffer];
	char buffer_snd[Snd_Max_buffer];
	int buffer_rec_len;

	ssize_t _rs, _ws;
	int &i = curstep[sockid];
	if(writeEnable[sockid] == 0) // read
	{
		if(bufferp[sockid] == 0) //first time in, reset the buffer
			bufferlen[sockid] = stdstr[i].length() + 
			                    (i == String ? strlen("99999") : 0) +
			                    (i == Time || i == String);

		do
		{
			errno = 0;
			_rs = read(fd, buffer_rec, bufferlen[sockid] - bufferp[sockid]);
		}while(_rs < 0 && errno == EINTR); //���źŴ�ϣ�����read

		if(_rs == 0)
		{
			cerr << "client" << "[" << getpid() <<"] read network error: " << strerror(errno) << endl;
			close(fd);
			return -1;
		}
		else if(_rs < 0)
		{
			if (_rs < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
				return 0;
			cerr << "read error: "<< strerror(errno) << endl;
			close(fd);
			return -1;
		}
		memcpy(tcpbuffer[sockid] + bufferp[sockid], buffer_rec, _rs);
		bufferp[sockid] += _rs;
		if(i == String)
		{
			if(buffer_rec[_rs - 1] != '\0')
				return 0;

			randstrlen[sockid] = 0;
			for (int j = 3; (randstrlen[sockid] < 10000) && (tcpbuffer[sockid][j] >= '0' && tcpbuffer[sockid][j] <= '9'); ++j)
				randstrlen[sockid] = randstrlen[sockid] * 10 + tcpbuffer[sockid][j] - '0';
			tcpbuffer[sockid][3] = '\0';
			for(int j = 0; j < randstrlen[sockid]; ++j)
				strbuff[sockid].push_back(rand() % Limit);
		}
		else if(bufferp[sockid] < bufferlen[sockid])
			return 0;
		tcpbuffer[sockid][bufferp[sockid]++] = '\0';
		if(strcmp(tcpbuffer[sockid], stdstr[i].c_str()))
		{
			cerr << "information exchange error" << endl;
			close(fd);
			return -1;
		}
		bufferp[sockid] = 0;
		writeEnable[sockid] ^= 1;
		if(i == Close)
		{
			if(forkFlag == true)
				close(fd);
			/*string filename = to_string(sid);
			filename.append(".").append(to_string(pid[sockid])).append(".pid.txt");
			int wfd;
			if ((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
			{
				cerr << "open error" << endl;
				return -2;
			}
			write(wfd, &sid, sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, &pid[sockid], sizeof(int));
			write(wfd, "\n", 1);
			write(wfd, timestamp[sockid], TimeStampLen - 1);
			write(wfd, "\n", 1);
			for (int j = 0; j < randstrlen[sockid]; ++j)
				buffer_snd[j] = strbuff[sockid][j];
			write(wfd, buffer_snd, randstrlen[sockid]);
			write(wfd, "\n", 1);
			close(wfd);*/

			if(opendir("./txt") == NULL)
			{
				mode_t mode = umask(0);
				mkdir("./txt", 0777);
			}
			string filename = "./txt/";
			filename.append(to_string(sid)).append(".").append(to_string(pid[sockid])).append(".pid.txt");
			int wfd;
			if ((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
			{
				cerr << "open error" << endl;
				return -2;
			}
			char int_tmp_buff[10];
			sprintf(int_tmp_buff, "%d", sid);
			write(wfd, int_tmp_buff, strlen(int_tmp_buff));
			write(wfd, "\n", 1);
			sprintf(int_tmp_buff, "%d", pid[sockid]);
			write(wfd, int_tmp_buff, strlen(int_tmp_buff));
			write(wfd, "\n", 1);
			write(wfd, timestamp[sockid], TimeStampLen - 1);
			write(wfd, "\n", 1);
			for (int j = 0; j < randstrlen[sockid]; ++j)
				buffer_snd[j] = strbuff[sockid][j];
			write(wfd, buffer_snd, randstrlen[sockid]);
			write(wfd, "\n", 1);
			close(wfd);

			return 1;
		}
	}
	else // write
	{
		if(bufferp[sockid] == 0)
		{
			int tmp;
			switch (i)
			{
				case Sid:
				{
					tmp = htonl(sid);
					bufferlen[sockid] = sizeof(int);
					memcpy(tcpbuffer[sockid], &tmp, bufferlen[sockid]);
					break;
				}
				case Pid:
				{
					pid_t tpid = getpid();
					pid[sockid] = (forkFlag == false) ? ((tpid << 16) + fd) : tpid;
					tmp = htonl(pid[sockid]);
					bufferlen[sockid] = sizeof(int);
					memcpy(tcpbuffer[sockid], &tmp, bufferlen[sockid]);
					break;
				}
				case Time:
				{
					get_time_str(timestamp[sockid]);
					bufferlen[sockid] = TimeStampLen - 1;
					//cout << "client[" << sockid << "] length = " << bufferlen[sockid] << endl;
					memcpy(tcpbuffer[sockid], timestamp[sockid], bufferlen[sockid]);
					
					tcpbuffer[sockid][19] = '\0';
					//cout << "client[" << sockid << "] timestamp = " << tcpbuffer[sockid] << endl;
					
					break;
				}
				case String:
				{
					for(int j = 0; j < randstrlen[sockid]; ++j)
						tcpbuffer[sockid][j] = strbuff[sockid][j];
					bufferlen[sockid] = randstrlen[sockid];
					break;
				}
				case Close:
				{
					//close(fd);

					// string filename = to_string(sid);
					// filename.append(".").append(to_string(pid[sockid])).append(".pid.txt");
					// int wfd;
					// if ((wfd = open(filename.c_str(), O_WRONLY | O_CREAT, 0666)) < 0)
					// {
					// 	cerr << "open error" << endl;
					// 	return -2;
					// }
					// write(wfd, &sid, sizeof(int));
					// write(wfd, "\n", 1);
					// write(wfd, &pid[sockid], sizeof(int));
					// write(wfd, "\n", 1);
					// write(wfd, timestamp[sockid], TimeStampLen - 1);
					// write(wfd, "\n", 1);
					// for (int j = 0; j < randstrlen[sockid]; ++j)
					// 	buffer_snd[j] = strbuff[sockid][j];
					// write(wfd, buffer_snd, randstrlen[sockid]);
					// write(wfd, "\n", 1);
					// close(wfd);
					// return 1;
				}
				default:
					;
			}
		}

		//printf("step[%d] client[%d] address = %x\n", i, sockid, tcpbuffer[sockid]);
		do
		{
			errno = 0;
			_ws = write(fd, tcpbuffer[sockid] + bufferp[sockid], bufferlen[sockid] - bufferp[sockid]);
		}while(_ws < 0 && errno == EINTR); //���źŴ�ϣ�����write

		if(_ws <= 0)
		{
			if(_ws == 0)
			{
				cerr << "write network error: " << strerror(errno) << endl;
				close(fd);
				return -1;
			}
			else if (_ws < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
				return 0;

			cerr << "write error: " << strerror(errno) << endl;
			close(fd);
			return -1;
		}
		bufferp[sockid] += _ws;
		if(bufferp[sockid] < bufferlen[sockid])
			return 0;
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

void Sock::get_time_str(char * const timestr)
{
	time_t rawtime;
	tm *ptm;

	time(&rawtime);
	ptm = gmtime(&rawtime);

	sprintf(timestr, "%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

}

static void sig_child(int signal)
{
	pid_t pid;
	int stat,i;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		//printf("child %d terminated.\n", pid);
		;
}
static void IncRunTime(int signal)
{
	++run_Time; //ȫ�ּ�ʱ��+1
	alarm(1);
	return;
}

void Sock::clearState(int index)
{
	// ���ö�Ӧ����״̬
	curstep[index] = Sid;		//connectʱ��Ӧλ��Sid
	writeEnable[index] = 0;		//connectʱ��Ӧλ��0
	bufferp[index] = 0;
}
