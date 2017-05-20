/*
 * httpserver.cpp
 *
 *  Created on: 2017年5月16日
 *      Author: yang
 */

#include "HttpServer.h"
#include "log/Logger.h"


HttpServer::HttpServer(const char* dbusr, const char* dbpw, const char* dbname, int iport) :
	_sqlhelper(new MysqlHelper("localhost",dbusr, dbpw, dbname)), _mysocket(new MySoket(iport, _sqlhelper))
{}

HttpServer::~HttpServer()
{
	//断开socket 和 数据库连接
	_mysocket->socketClose();
	_sqlhelper->disconnect();
	std::cout << "socket closed" << std::endl;
	std::cout << "database disconnected" << std::endl;
}

int HttpServer::init()
{
	try {

		_sqlhelper->connect(); //连接到数据库

	}catch (...)
	{
		Logger::LogDebug("connect to Mysql err in " + std::string(__FUNCTION__) );
	}
	_sqlhelper->setDefalutCharacterSet();//设置默认字符集 utf8

	if (_mysocket->socketCreate() != 0)
	{
		Logger::LogDebug("socketCreate error " + std::string(strerror(errno))+ " " + std::string(__FUNCTION__));
		//perror("socketCreate err");
		return errno;
	} else
	{
		Logger::LogDebug("created socket. host: " + std::string(_mysocket->getServerHost()) );
		return 0;
	}
}

void HttpServer::run(bool isDaemo)
{
	Logger::LogDebug("httpServer starting");
	if (isDaemo)
		setDaemon(); //设置进程为daemon状态

	signal1(SIGINT, catchSignal);
	signal1(SIGQUIT, catchSignal);

	_mysocket->socketAccept();

	Logger::LogDebug("httpServer is closed");
}

void HttpServer::setDaemon() {
	pid_t pid, sid;
	pid = fork();
	if (pid < 0)
	{
		Logger::LogDebug("fork failed : "+ std::string(strerror(errno))+ " in " + std::string(__FUNCTION__));
		exit (EXIT_FAILURE);
		;
	}
	if (pid > 0)
	{
		exit (EXIT_SUCCESS);
	}

	if ((sid = setsid()) < 0)
	{

		Logger::LogDebug("setsid failed failed :"+ std::string(strerror(errno)) + " in " + std::string(__FUNCTION__));
		exit (EXIT_FAILURE);
	}


}

void HttpServer::catchSignal(int sig,siginfo_t *siginfo,void *myact)  {

	/*
	printf("signal number:%d\n",n);//打印出信号值
	printf("siginfo signo:%d\n",siginfo->si_signo); // siginfo结构里保存的信号值
	printf("siginfo err:%s\n",strerror(siginfo->si_errno)); // 打印出错误代码
	printf("siginfo code:%d\n",siginfo->si_code);   //　打印出出错原因
	*/
	switch (sig){
		case SIGINT:
			Logger::LogDebug("SIGINT errcode: " + std::to_string(siginfo->si_signo));
			break;
		case SIGSEGV:
			Logger::LogDebug("SIGSEGV errcode: " + std::to_string(siginfo->si_signo));
			break;
		case SIGABRT:
			Logger::LogDebug("SIGABRT errcode: " + std::to_string(siginfo->si_signo));
			break;
		case SIGQUIT:
			Logger::LogDebug("SIGABRT errcode: " + std::to_string(siginfo->si_signo));
			break;
		}
}


//设置信号处理函数
/*
struct sigaction {
	//信号处理函数
   void     (*sa_handler)(int);
   // sa_flags设置别的值 此函数制定别的信号处理函数替代sa_handler
   void     (*sa_sigaction)(int, siginfo_t *, void *);
   sigset_t   sa_mask;  信号屏蔽集 通过sigemptyset/sigaddset 清空和设置屏蔽集
   int        sa_flags; 取0 表示默认行为
   void     (*sa_restorer)(void);
};
 */
int HttpServer::signal1(int signal, void (*func)(int n,siginfo_t *siginfo,void *myact)) {
	/*
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	//sigaction给信号signal设置新的信号处理函数act， 同时保留该信号原有的信号处理函数oldact
	return sigaction(signal, &act, &oact);
	*/
	struct sigaction act;
	sigemptyset(&act.sa_mask);   /** 清空阻塞信号 **/
	act.sa_flags = SA_SIGINFO;     /** 设置SA_SIGINFO 表示传递附加信息到触发函数 **/
	act.sa_sigaction = func;
	sigaction(signal, &act, &act);
}


//程序主要处理函数
void *HttpServer::socketContr(void *st) {
	Logger::LogDebug("thread is be created");
	stSql * st_and_sql = ( stSql*)st;

	//得到来自客户端的socket
	int client_st =  st_and_sql->_st;

	char buf_from_cli[BUFSIZ];
	memset(buf_from_cli, 0, sizeof(buf_from_cli));

	//接收来自client端socket的消息
	int rc = (int) recv(client_st, buf_from_cli, sizeof(buf_from_cli), 0);
	if (rc <= 0)
	{

		Logger::LogDebug("recv failed " + std::string(strerror(errno)) + " in " + std::string(__FUNCTION__));
	}
	else
	{
		//新建一个HttpHandle类  处理http消息
		auto httphandle = std::make_shared<HttpHandle>(buf_from_cli, st_and_sql->_sqlhelper, &client_st);

		//生成要回复的内容
		httphandle->handleRequest();

		size_t ilen = httphandle->getMessage_len();
		if (ilen > 0) {
			send(client_st, httphandle->getReplyContent(), ilen, 0);
		}
	}

	close(client_st);//关闭client端socket

	Logger::LogDebug("thread is end");

	return NULL;
}

