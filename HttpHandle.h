/*
 * httphandler.h
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */

#ifndef HTTPHANDLER_H_
#define HTTPHANDLER_H_


#include "MysqlHelper.h"
#include <string>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

using namespace mysqlhelper;
//http请求处理类
class HttpHandle
{
public:
	HttpHandle(const char* recv, std::shared_ptr<MysqlHelper> sqlhelper, int* cli_st);
	~HttpHandle();

	//获取回复内容
	const char* getReplyContent() const;
	const std::string& getMethod() const;

	void handleRequest();
	//int sendContent();


private:
	void setMethod(const char*);
	void fillReply(size_t contentlen);
	void handleGET();
	void handlePOST();

    size_t getFileContent(FILE *fd, struct stat*);
    size_t findRequestFile(std::string filename);
	//动态设置http请求内容,query为条件，buf为动态内容
	size_t getSqlReply(std::string query);



private:
	char* reply_content;	//需要回复给浏览器的http内容
	char* contentbuf;   //用于保存临时数据

	std::string _method;		//http method
	std::string _method_content; //method内容
	std::shared_ptr<MysqlHelper> _sqlhelper;

	int * _cli_st;    //客户端连接socket 文件描述符
	size_t message_len;

	size_t post_content_length;
	std::string post_var;
public:
	size_t getMessage_len() const;

public:
	static const char *getFileType(std::string &filename);//根据扩展名返回文件类型描述
	static int hex2dec(const char hex);  //将16进制的字符转化为十进制的整数
	static unsigned char hexstr2dec(const char *hex);  //将16进制的字符串转化为十进制的unsigned char
	//将HTTP GET请求中的转义符号转化为标准字符,注意，空格被转义为'+'号
	static void httpStr2Stdstr(const char *httpstr, size_t pos ,char * stdstr);
};




#endif /* HTTPHANDLER_H_ */
