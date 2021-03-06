/*
 * httpserver.h
 *
 *  Created on: 2017年5月16日
 *      Author: yang
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include "HttpHandle.h"
#include "MySoket.h"
#include "MysqlHelper.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <bits/siginfo.h>

#include <memory>
#include <iostream>
#include <thread>
#include <cstring>

class HttpServer
{
public:
	explicit HttpServer(const char* dbusr, const char* dbpw, const char* dbname, int port = 8800);
	int init();
	void run(bool isDaemo);	//服务器启动函数
	~HttpServer();

public:
	static void* socketContr(void* st_sql);
private:
	static void setDaemon();
	static void catchSignal(int n,siginfo_t *siginfo,void *myact);
	static int signal1(int signal, void (*func)(int n,siginfo_t *siginfo,void *myact));


private:
	std::shared_ptr<MysqlHelper> 	_sqlhelper;
	std::shared_ptr<MySoket>  	    _mysocket;
};

#endif /* HTTPSERVER_H_ */
