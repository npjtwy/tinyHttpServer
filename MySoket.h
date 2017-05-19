/*
 * MySoket.h
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */

#ifndef MYSOKET_H_
#define MYSOKET_H_
//MySoket类 将常用socket操作封装起来 限TCP
#include "MysqlHelper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory>

using namespace mysqlhelper;

typedef  struct stSql{
	int _st;
	std::shared_ptr<MysqlHelper> _sqlhelper;
} stSql;


class MySoket
{
public:
	MySoket(int uport, std::shared_ptr<MysqlHelper> &sqlhelper);
	~MySoket();
	int socketCreate();
	int socketAccept();
	int socketClose();
	const char* getServerHost() const;
	const char* getClientHost() const;
	int getPort() const;
private:
	int port;
	int server_st;
	int client_st;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	std::shared_ptr<MysqlHelper> sqlHelper;
};

#endif /* MYSOKET_H_ */
