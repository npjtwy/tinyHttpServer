/*
 * server.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */
#include <stdio.h>
#include <stdlib.h>
#include "HttpServer.h"
#include "log/Logger.h"

using namespace mysqlhelper;

int main(int arg, char **args)
{
	if (arg < 2) //如果没有参数，main函数返回
	{
		printf("usage:myserver port\n");
		return EXIT_FAILURE;
	}


	Logger::init();

	int iport = atoi(args[1]); //将第一个参数转化为整数
	if (iport == 0)
	{
		Logger::LogDebug("port is invalid");
		return EXIT_FAILURE;
	}



	std::shared_ptr<HttpServer> httpserver(new HttpServer("root", "wang", "httpserver", iport));
	if (httpserver->init() != 0){
		Logger::LogDebug("httpserver init failed");
		return EXIT_SUCCESS;

	}
	//1 代表进入daemon 模式, 0 不进入
	httpserver->run(0);

	return EXIT_SUCCESS;
}

