/*
 * MySoket.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */

#include "MySoket.h"
#include "HttpServer.h"
#include "log/Logger.h"


MySoket::MySoket(int uport, std::shared_ptr<MysqlHelper>& sqlhelper): port(uport),server_st(0),client_st(0),
sqlHelper(sqlhelper)
{
	memset(&client_addr, 0, sizeof(client_addr));
	memset(&server_addr, 0, sizeof(server_addr));
}

int MySoket::socketCreate()
{
	server_st = socket(AF_INET, SOCK_STREAM, 0);//建立TCP的socket描述符
	if (server_st == -1)
	{
		Logger::LogDebug("socket create failed" + std::string(strerror(errno)));
		return -1;
	}
	//设置socket可重用
	int on = 1;
	if (setsockopt(server_st, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	{
		Logger::LogDebug("setsockopt failed " + std::string(strerror(errno)));
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(server_st, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
	{
		Logger::LogDebug("bind failed " + std::string(strerror(errno)));
		return -1;
	}
	if (listen(server_st, 100) == -1)
	{
		Logger::LogDebug("listen failed " + std::string(strerror(errno)));
		return -1;
	}
	printf("listen %d success\n", port);
	return 0;
}

int MySoket::socketAccept()
{
	pthread_t thr_d;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//设置线程为可分离状态

	socklen_t len = sizeof(this->client_addr);
	while(1)//循环执行accept
	{
		//accept函数阻塞，直到有client端连接到达，或者accept错误返回
		this->client_st = accept(this->server_st, (struct sockaddr*)&client_addr, &len);
		if (this->client_st < 0)
		{
			Logger::LogDebug("accept error: " + std::string(strerror(errno)) + " in " + std::string(__FUNCTION__));
			return -1;
		}
		else
		{
			Logger::LogDebug("accept by " + std::string(this->getClientHost()));

			//一个包含客户端st和打开的数据库连接实例的结构体,用来给线程处理函数传参数
			stSql st_and_sql;
			st_and_sql._st = client_st;
			st_and_sql._sqlhelper = this->sqlHelper;
			{
				//将来自client端的socket做为参数，启动一个可分离线程
				pthread_create(&thr_d, &attr,  HttpServer::socketContr, (void *)&st_and_sql);
			}

		}



	}
}

int MySoket::socketClose()
{
	close(server_st);
	return 0;
}

MySoket::~MySoket()
{

}

const char *MySoket::getServerHost() const {
	return inet_ntoa(server_addr.sin_addr); //返回服务器地址
}

int MySoket::getPort() const{
		return ntohl(server_addr.sin_port);
}

const char *MySoket::getClientHost() const {
	return inet_ntoa(client_addr.sin_addr);  //返回客户端地址
}

