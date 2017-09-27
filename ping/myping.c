#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define ICMP_SIZE (sizeof(struct icmp))
#define ICMP_ECHO 0 
#define ICMP_ECHOREPLY 0
#define BUF_SIZE 1024
#define NUM 5 // 发送报文次数

#define UCHAR unsigned char 
#define USHORT unsigned short
#define UINT unsigned int

struct icmp{
	UCHAR	type;		//类型
	UCHAR	code;		//代码
	USHORT	checksum;	//检验和
	USHORT	id;			//标识符
	USHORT	sequence;	//序号
	struct timeval timestamp; //时间戳
};

struct ip{
	//主机字节序判断
	#if __BYTE_ORDER == __LITTLE_ENDIAN
	UCHAR hlen:4;
	UCHAR version:4;
	#endif //_LITTLE_ENDIAN

	#if __BYTE_ORDER == __BIG_ENDIAN
	UCHAR version:4;
	UCHAR hlen:4;
	#endif //_BIG_ENDIAN

	UCHAR tos;			//服务类型
	USHORT len;			//总长度
	USHORT id ;			//标识符
	USHORT offset;		//标志和片偏移
	UCHAR ttl;			//生存时间
	UCHAR protocol;		//协议
	USHORT checksum;	//检验和
	struct in_addr ipsrc;
	struct in_addr ipdst;
};

char buf[BUF_SIZE] = {0};


unsigned short check_sum(unsigned short *addr, int len)
{
	unsigned int sum = 0;
	while(len > 1){
		sum += *addr++;
		len -=2;
	}

	if(len == 1){
		sum += *(unsigned char *)addr;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
}

float time_diff(struct timeval* begin,
		struct timeval* end)
{
	int n;
	n = (end->tv_sec - begin->tv_sec) * 1000000 + \
		(end->tv_usec - begin->tv_usec);
	
	//转化为毫秒返回
	return (float)(n/1000);
}

//封装一个icmp报文
void pack(struct icmp *icmp, int sequence)
{
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->id = getpid();
	icmp->sequence = sequence;
	gettimeofday(&icmp->timestamp, 0);
	icmp->checksum = check_sum((USHORT *)icmp, ICMP_SIZE);
}
//对接收到的icmp报文进行解包
int unpack(char* buf,int len, char* addr)
{
	int i, ipheadlen;
	struct ip* ip;
	struct icmp* icmp;
	float rtt;			//往返时间
	struct timeval end; //记录接收报文的时间戳

	ip = (struct ip*)buf;

	//计算ip首部长度，ip首部的长度标识 * 4 
	ipheadlen = (ip->hlen)<<2;
	
	//取得icmp的首部报文
	icmp =(struct icmp*)(buf + ipheadlen);

	//计算icmp报文的总长度
	len -= ipheadlen; 

	if(icmp->type != ICMP_ECHOREPLY || \
			icmp->id != getpid()){
		printf("icmp packages are not send by us. \n");
		return -1;
	}

	//计算往返时间
	rtt = time_diff(&icmp->timestamp, &end);
	printf("%d bytes from %s : icmp_seq=%u ttl=%d rtt=%fms \n",\
			len, addr, icmp->sequence, ip->ttl, rtt);

	return 0;
}

int main(int argc, char* argv[])
{
	struct hostent* host;
	struct icmp sendicmp;
	struct sockaddr_in from;
	struct sockaddr_in to;
	int fromlen = 0;
	int sockfd;
	int nsend = 0;
	int nreceived = 0;
	int i, n;
	in_addr_t inaddr;

	memset(&from, 0, sizeof(struct sockaddr_in));
	memset(&to, 0,sizeof(struct sockaddr_in));

	if(argc < 2){
		printf("useage : %s hostname/IP address \n",argv[0]);
		exit(1);
	}
	
	//生成原生套接字
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))==-1){
		printf("socker error..\n");
		exit(1);
	}
	to.sin_family = AF_INET;
	//判断是域名还是IP地址
	if(inaddr = inet_addr(argv[1]) == INADDR_NONE){
		//域名
		if((host = gethostbyname(argv[1]))== NULL){
			printf("gethostbyname error \n");
			exit(1);
		}
		to.sin_addr = *(struct in_addr *)host->h_addr_list[0];
	}
	else{
		// ip 地址
		to.sin_addr.s_addr = inaddr;
	}

	printf("ping %s(%s) : %d bytes of data.\n",\
			argv[1],inet_ntoa(to.sin_addr),(int)ICMP_SIZE);

	for(i = 0; i<NUM; i++){
		nsend++;
		memset(&sendicmp, 0, ICMP_SIZE);
		pack(&sendicmp, nsend);

		//发送icmp报文
		if(sendto(sockfd, &sendicmp, ICMP_SIZE, 0,(struct \
						sockaddr*)&to, sizeof(to)) == -1){
			printf("sendto error \n");
			continue;
		}

		//接收报文
		if((n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct\
							sockaddr*)&from, &fromlen)) < 0){
			printf(" recvfrom error \n");
			continue;
		}
		nreceived ++;
		if(unpack(buf, n,inet_ntoa(from.sin_addr)) == -1){
			printf(" unapck error \n");
		}
		sleep(1);
	}

	printf(" --- %s ping statistics ---\n",argv[1]);
	printf("%d packages transmitted, %d received, %%%d package\
			loss \n", nsend, nreceived, (nsend - nreceived)/nsend*100);
	return 0;
}

//int main(int argc, char* argv[])
//{
//	struct hostent *host; 
//
//	if(argc < 2){
//		printf("use : %s <hostname> \n",argv[0]);
//		exit(1);
//	}
//
//	host = gethostbyname(argv[1]);
//
//	int i = 0;
//	for(i=0;host->h_addr_list[i]; i++){
//		printf("IP addr %d: %s \n", i+1,\
//				inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
//	}
//	return 0;
//}


//int main()
//{
//
//	return 0;
//}
/*
int main(void)
{
	char* addr1 = "1.2.3.4";
	char* addr2 = "192.168.1.1";

	in_addr_t data = inet_addr(addr1); 
	printf("%s -> %#lx \n",addr1, (long)data);

	data = inet_addr(addr2);
	printf("%s -> %#lx \n",addr2, (long)data);

	return 0;
}
*/
/*
int main(void){
	struct timeval begin, end;

	gettimeofday(&begin, 0);
	printf("do something here ...\n");
	sleep(1);

	gettimeofday(&end, 0);
	printf("done... time:%fms\n",TimeDiff(&begin,&end));
	return 0;
}
*/
/*
int main()
{
	unsigned short hosts = 0x1234;
	unsigned short nets;
	unsigned long hostl = 0x12345678;
	unsigned long netl;

	nets = htons(hosts);
	netl = htonl(hostl);

	printf("主机字节序：%#x\n",hosts);
	printf("网络字节序：%#x\n",nets);

	printf("主机字节序：%#lx\n",hostl);
	printf("网络字节序：%#lx\n",netl);
	return 0;
}*/
