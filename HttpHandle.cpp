/*
 * httphandler.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "HttpHandle.h"
#include "MySoket.h"
#include "log/Logger.h"


#define HEAD "HTTP/1.0 200 OK\n\
Content-Type: %s\n\
Transfer-Encoding: chunked\n\
Connection: Keep-Alive\n\
Accept-Ranges:bytes\n\
Content-Length:%d\n\n"

#define TAIL "\n\n"

#define EXEC "s?wd="

using namespace std;


HttpHandle::HttpHandle(const char* recv, shared_ptr<MysqlHelper> sqlhelper) :
		reply_content(NULL), get_command(), _sqlhelper(sqlhelper)
{//构造函数  完成get command 的解析
	this->setGetCommand(recv);
}
HttpHandle::~HttpHandle()
{
	free(this->reply_content);
}


int HttpHandle::setReplyContent()
{
	return this->setReplyContent(&this->reply_content);
}

/*
 * http get 内容
 1.GET /index.jsp HTTP/1.1  如果get为空 :  GET / HTTP...

 2.Accept-Language: zh-cn

 3.Connection: Keep-Alive

 4.Host: 192.168.0.106

 5.Content-Length: 37

 6.userName=new_andy&password=new_andy
 */


void HttpHandle::setGetCommand(const char *recv)
{
	string header(recv);

	auto beg = header.find_first_of(' ', 0);//找到第一个' '位置
	beg += 2;//前进两个位置 判断get请求是否为空
	if (header[beg] == ' ')
		return;
	else
	{
		while(header[beg] != ' ')
		{
			this->get_command.push_back(header[beg++]);
		}
	}
}

int HttpHandle::setReplyContent(char** replyContent)
{
	int iContentLen = 0;
	char *contentbuf = NULL;
	if (get_command.empty()) //GET请求后面为空，得到默认页面内容图
	{
		iContentLen = getFileContent("../www/default.html", &contentbuf);
	} else
	{
		if (strncmp(get_command.c_str(), EXEC, strlen(EXEC)) == 0) //GET请求后面为s?wd=
		{
			char query[1024];
			//得到s?wd=字符串后面的转义字符内容  生成数据库查询语句的一部分
			httpStr2Stdstr(get_command.c_str(),strlen(EXEC), query);

			iContentLen = getDynamicContent(string(query), &contentbuf, this->_sqlhelper);

		} else
		{
			//动态设置http请求内容,query为条件，buf为动态内容
			iContentLen = getFileContent(get_command, &contentbuf);
		}
	}

	//生成回复消息

	if (iContentLen > 0)
	{
//		char headbuf[1024];
//		memset(headbuf, 0, sizeof(headbuf));
//		sprintf(headbuf, HEAD, getFileType(this->get_command), iContentLen); //设置消息头
//
//		replyContent = string(headbuf) + replyContent + string(TAIL);
//		printf("headbuf:\n%s", headbuf);
//		return replyContent.size();//返回消息总长度

		char headbuf[1024];
		memset(headbuf, 0, sizeof(headbuf));
		sprintf(headbuf, HEAD, getFileType(this->get_command), iContentLen); //设置消息头
		size_t iheadlen = strlen(headbuf);//得到消息头长度
		size_t itaillen = strlen(TAIL);//得到消息尾长度
		size_t isumlen = iheadlen + iContentLen + itaillen;//得到消息总长度
		*replyContent = (char*)malloc(isumlen);//根据消息总长度，动态分配内存
		char *tmp = *replyContent;
		memcpy(tmp, headbuf, iheadlen); //安装消息头
		memcpy(&tmp[iheadlen], contentbuf, static_cast<size_t>(iContentLen)); //安装消息体
		memcpy(&tmp[iheadlen + iContentLen], TAIL, itaillen); //安装消息尾

		printf("reply content:\n%s", *replyContent);

		if (contentbuf)
			free(contentbuf);
		return  static_cast<int>(isumlen);//返回消息总长度
	}
	else
		return 0;

}

const char* HttpHandle::getReplyContent() const
{
	return reply_content;
}



const string& HttpHandle::getGetCommand() const
{
	return this->get_command;
}

int HttpHandle::getFileContent(string filename, char **buf) {

	struct stat t;
	memset(&t, 0, sizeof(t));

	filename  = "../www/" + filename;

	//根据文件名获取内容
	FILE *fd = fopen(filename.c_str(), "rb");//从只读方式打开参数filename指定的文件
	if (fd)	//如果打开成功
	{

		stat(filename.data(), &t);
		*buf = (char*)malloc(static_cast<size_t>(t.st_size));//根据文件大小，动态分配内存buf
		fread(*buf, static_cast<size_t>(t.st_size), 1, fd);//将文件读取到buf
		fclose(fd);
		return static_cast<int>(t.st_size);
	} else
	{

		Logger::LogDebug("open file " + filename + " failed in HttpHandle::getFileContent: " + std::string(strerror(errno)));
		return 0;
	}
}

const char *HttpHandle::getFileType(string &filename) {
	////////////得到文件扩展名///////////////////
	auto len = filename.size();

	auto pos = filename.find_first_of('.', 0);

	string sExt;
	pos++;

	while(pos < len)
	{
		sExt.push_back(filename[pos++]);
	}

	////////根据扩展名返回相应描述///////////////////

	if (sExt == "bmp")
		return "image/bmp";

	if (sExt == "gif")
		return "image/gif";

	if (sExt == "ico")
		return "image/x-icon";

	if (sExt == "jpg")
		return "image/jpeg";

	if (sExt ==  "avi")
		return "video/avi";

	if (sExt == "css")
		return "text/css";

	if (sExt ==  "dll")
		return "application/x-msdownload";

	if (sExt == "exe")
		return "application/x-msdownload";

	if (sExt ==  "dtd")
		return "text/xml";

	if (sExt == "mp3")
		return "audio/mp3";

	if (sExt ==  "mpg")
		return "video/mpg";

	if (sExt == "png")
		return "image/png";

	if (sExt ==  "ppt")
		return "application/vnd.ms-powerpoint";

	if (sExt == "xls")
		return "application/vnd.ms-excel";

	if (sExt == "doc")
		return "application/msword";

	if (sExt ==  "mp4")
		return "video/mpeg4";

	if (sExt == "ppt")
		return "application/x-ppt";

	if (sExt ==  "wma")
		return "audio/x-ms-wma";

	if (sExt == "wmv")
		return "video/x-ms-wmv";

	return "text/html";
}

int HttpHandle::hex2dec(const char hex) //将16进制的字符转化为十进制的整数
{
	switch (hex)
	{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'a':
			return 10;
		case 'A':
			return 10;
		case 'b':
			return 11;
		case 'B':
			return 11;
		case 'c':
			return 12;
		case 'C':
			return 12;
		case 'd':
			return 13;
		case 'D':
			return 13;
		case 'e':
			return 14;
		case 'E':
			return 14;
		case 'f':
			return 15;
		case 'F':
			return 15;
		default:
			return -1;
	}
}
unsigned char HttpHandle::hexstr2dec(const char *hex) //将16进制的字符串转化为十进制的unsigned char
{
	return static_cast<unsigned  char>(hex2dec(hex[0]) * 16 + hex2dec(hex[1]));

}

//将HTTP GET请求中的转义符号转化为标准字符,注意，空格被转义为'+'号
void HttpHandle::httpStr2Stdstr(const char *httpstr, size_t pos ,char * stdstr)
{
	//定位到要转换的字符串首位置
	httpstr += pos;
	int index = 0;
	size_t i;
	//httpstr = %E4%BC%A0%E6%99%BA
	for (i = 0; i < strlen(httpstr); i++)
	{
		if (httpstr[i] == '%')
		{
			stdstr[index] = hexstr2dec(&httpstr[i + 1]);
			i += 2;
		} else
		{
			stdstr[index] = httpstr[i];
		}
		index++;
	}
}

int HttpHandle::getDynamicContent(string query, char **buff, shared_ptr<MysqlHelper>sqlHelper) {
	char* templetcontent;

	//获取模板页面内容

	//int getFileContent(string filename, string& buf)
	if (getFileContent("templet.html", &templetcontent) == 0)
		return 0;
//
	*buff = (char*)malloc(BUFSIZ);
//	char *body = NULL;
	string body;
	//生成sql查询语句
	string sqlquery = "select * from baidu where name like \"%"+query+"%\"";

	MysqlData sql_data;
	//执行sql查询
	try {
		 sql_data = sqlHelper->queryRecord(sqlquery);
	}catch (...)
	{
		Logger::LogDebug("sql query in " + std::string(__FUNCTION__));
	}

	if (!sql_data.size())
	{
		body =  "抱歉，没有查询结果";
	}
	else
	{
		//<a href="http://192.168.1.254">新 闻</a>
		//生成链接：
		char *url = (char*)malloc(256);
		bzero(url, strlen(url));
		const char *href = "<a href=\"%s\">%s</a>";
		sprintf(url, href, sql_data[0]["url"].c_str(), sql_data[0]["name"].c_str());

		//body为查询得到的description的内容
		body = sql_data[0]["description"] + " " + std::string(url);

		FILE *fp = fopen("log","a+");
		if (!fp)
			perror("fopen in getDynamicContent");
		fwrite(body.c_str(),1,body.size(), fp);
		fclose(fp);

		free(url);
	}

	//将查询结果写入buff 其中 query和body将填充templet.html中的两个%s
	sprintf(*buff, templetcontent, query.c_str(), body.c_str());
	if(templetcontent)
		free(templetcontent);
	return static_cast<int>(strlen(*buff));
}


