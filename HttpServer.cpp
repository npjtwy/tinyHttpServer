/*
 * httpserver.cpp
 *
 *  Created on: 2017年5月16日
 *      Author: yang
 */

#include "HttpServer.h"
#include "log/Logger.h"
#include <thread>
#include <cstring>

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

void HttpServer::init()
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
		exit(EXIT_FAILURE);
	} else
	{
		Logger::LogDebug("created socket. host: " + std::string(_mysocket->getServerHost()) );
	}
}

void HttpServer::run(bool isDaemo)
{
	Logger::LogDebug("httpServer starting");
	if (isDaemo)
		setDaemon(); //设置进程为daemon状态

	signal1(SIGINT, catchSignal);	//捕捉SIGINT信号
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

void HttpServer::catchSignal(int Sign) {
	switch (Sign)
	{
		case SIGINT:
			Logger::LogDebug("signal SIGINT");
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
int HttpServer::signal1(int signal, void (*func)(int)) {
	struct sigaction act, oact;
	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	//sigaction给信号signal设置新的信号处理函数act， 同时保留该信号原有的信号处理函数oldact
	return sigaction(signal, &act, &oact);
}


//程序主要处理函数
void *HttpServer::socketContr(void *st) {
	//std::this_thread::get_id()
	Logger::LogDebug("thread begin");
	stSql * st_and_sql = ( stSql*)st;

	//得到来自客户端的socket
	int client_st =  st_and_sql->_st;

	char buf_from_cli[BUFSIZ];
	memset(buf_from_cli, 0, sizeof(buf_from_cli));
	//接收来自client端socket的消息
	int rc = recv(client_st, buf_from_cli,  sizeof(buf_from_cli), 0);

	if (rc <= 0)
	{

		Logger::LogDebug("recv failed " + std::string(strerror(errno)) + " in " + std::string(__FUNCTION__));
	}
	else
	{
		printf("recv:\n%s", buf_from_cli);

		//新建一个HttpHandle类  处理http消息
		auto httphandle = std::make_shared<HttpHandle>(buf_from_cli, st_and_sql->_sqlhelper);

		//生成要回复的内容
		int ilen = httphandle->setReplyContent();

		if (ilen > 0)
		{
			//将回复的内容发送给client端socket
			send(client_st, httphandle->getReplyContent(), (unsigned  int)ilen, 0);
		}
	}

	close(client_st);//关闭client端socket

	Logger::LogDebug("thread_is end");

	return NULL;
}

