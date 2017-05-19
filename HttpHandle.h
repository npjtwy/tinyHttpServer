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

using namespace std;
using namespace mysqlhelper;
//http请求处理类
class HttpHandle
{
public:
	HttpHandle(const char* recv, shared_ptr<MysqlHelper> sqlhelper);
	~HttpHandle();

	//获取回复内容
	const char* getReplyContent() const;
	const string& getGetCommand() const;
	int setReplyContent();


private:
	//根据getCommand 设置回复内容
	int setReplyContent( char** );
	void setGetCommand(const char*);



private:
	char* reply_content;	//需要回复给浏览器的http内容
	string get_command;		//浏览器get 后面的内容
	shared_ptr<MysqlHelper> _sqlhelper;

public:
	static  int getFileContent(string filename, char** buf);
	//动态设置http请求内容,query为条件，buf为动态内容
	static  int getDynamicContent(string query, char** buff, shared_ptr<MysqlHelper> sqlHelper);


	static const char *getFileType(string &filename);//根据扩展名返回文件类型描述
	static int hex2dec(const char hex);  //将16进制的字符转化为十进制的整数
	static unsigned char hexstr2dec(const char *hex);  //将16进制的字符串转化为十进制的unsigned char
	//将HTTP GET请求中的转义符号转化为标准字符,注意，空格被转义为'+'号
	static void httpStr2Stdstr(const char *httpstr, size_t pos ,char * stdstr);
};




#endif /* HTTPHANDLER_H_ */
