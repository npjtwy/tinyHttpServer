/*
 * httphandler.cpp
 *
 *  Created on: 2017年5月2日
 *      Author: yang
 */


#include "HttpHandle.h"
#include "log/Logger.h"


#define HEAD "HTTP/1.0 200 OK\n\
Content-Type: %s\n\
Transfer-Encoding: chunked\n\
Connection: Keep-Alive\n\
Accept-Ranges:bytes\n\
Content-Length:%d\n\n"

#define TAIL "\n\n"

#define EXEC "s?wd="

#define MALLOCFAILED "malloc failed in "

HttpHandle::HttpHandle(const char* recv, std::shared_ptr<MysqlHelper> sqlhelper, int *cli_st) :
		reply_content(NULL), contentbuf(NULL), _method(),_method_content(),
		_sqlhelper(sqlhelper), _cli_st(cli_st), post_var()
{
//构造函数  完成 http header 的解析
	post_content_length = 0;
	message_len = 0;
	this->setMethod(recv);
}
HttpHandle::~HttpHandle()
{
	if (this->reply_content) {
		free(this->reply_content);
		this->reply_content = NULL;
	}
	if (this->contentbuf) {
		free(contentbuf);
		this->contentbuf = NULL;
	}
}

const std::string& HttpHandle::getMethod() const
{
	return this->_method;
}

const char *HttpHandle::getReplyContent() const {
	return this->reply_content;
}

size_t HttpHandle::getMessage_len() const {
	return message_len;
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

void HttpHandle::setMethod(const char *recv)
{
	std::string header(recv);

	auto beg = header.find_first_of(' ', 0);//找到第一个' '位置

	//获取method
	for(std::string::size_type it = 0; it != beg; it++)
		_method.push_back(header[it]);

	beg += 2;//前进两个位置 判断请求是否为空
	if (header[beg] == ' ')
		;
	else
	{
		while(header[beg] != ' ')
		{
			this->_method_content.push_back(header[beg++]);
		}
	}

	if(_method == "POST") {
		auto found = header.find("Content-Length");
		if(found != std::string::npos)
		{
			char len[10];
			int i = 0;
			found += 16;    //定位到content-length的第一位数字
			for(i, found; header[found] != '\n'; found++ ,i++)
				len[i] = header[found];

			len[i] = '\0';
			this->post_content_length = (size_t)atoi(len);
		}
		auto found_post_var = header.find("color");
		if(found != std::string::npos)
		{
			found_post_var += 6;
			for(found_post_var; header[found_post_var] != '\n'; found_post_var++)
				post_var.push_back(header[found_post_var]);
		}
	}

}


void HttpHandle::handleRequest() {
	if (this->_method == "GET")
		handleGET();
	else if (this->_method == "POST")
		handlePOST();
	else
		return;
}

void HttpHandle::handlePOST() {

	char *templetcontent;
	size_t len;
	//获取post模板页面内容
	if ((len = findRequestFile("../www/postReply.html")) == 0)
		return ;

	if ((templetcontent = (char *) malloc(len+1)) == NULL)
		Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));

	memcpy(templetcontent, contentbuf, len);
	templetcontent[len] = '\0';

	free(contentbuf);
	if ((contentbuf = (char *) malloc(BUFSIZ)) == NULL)
		Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));

	sprintf(this->contentbuf, templetcontent, this->post_var.c_str());

	if (templetcontent)
		free(templetcontent);

	fillReply(strlen(this->contentbuf));
}


void HttpHandle::handleGET() {
	size_t len = 0;
	if (_method_content.empty()) //后面为空，得到默认页面内容
	{
		len = findRequestFile("index.html");
	} else
	{
		if (strncmp(_method_content.c_str(), EXEC, strlen(EXEC)) == 0) //GET请求后面为s?wd= 表示查询
		{
			char query[1024];
			//得到s?wd=字符串后面的转义字符内容  生成数据库查询语句的一部分
			httpStr2Stdstr(_method_content.c_str(),strlen(EXEC), query);
			len = getSqlReply(std::string(query));
		}
		else
		{
			//GET 后面是一个文件
			len = findRequestFile(_method_content);
		}
	}
	//将回复内容填到reply_content
	fillReply(len);

}



//根据sql语句生成回复
size_t HttpHandle::getSqlReply(std::string query) {


	char *templetcontent;
	size_t len;
	//获取模板页面内容
	if ((len = findRequestFile("templet.html")) == 0)
		return 0;

	//将content内容拷贝给 templecontent
	//templatecontent将用作填充字符串的模板
	//contentbuf将接受填充过的值 因此要重新分配内存
	if ((templetcontent = (char *) malloc(len+1)) == NULL)
		Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));
	memcpy(templetcontent, contentbuf, len);

	templetcontent[len] = '\0';

	free(contentbuf);
	if ((contentbuf = (char *) malloc(BUFSIZ)) == NULL)
		Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));

	std::string body;//查询结果
	if (!query.empty()) {

		//生成sql查询语句
		std::string sqlquery = "select * from baidu where name like \"%" + query + "%\"";

		MysqlData sql_data;
		//执行sql查询
		try {
			sql_data = this->_sqlhelper->queryRecord(sqlquery);
		} catch (...) {
			Logger::LogDebug("sql query in " + std::string(__FUNCTION__));
		}

		if (!sql_data.size()) {
			body = "抱歉，没有查询结果";
		} else {
			//生成链接：
			char *url;
			if ((url = (char *) malloc(1024)) == NULL)
				Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));

			memset(url, 0, strlen(url));

			const char *href = "<a href=\"%s\">%s</a>";
			sprintf(url, href, sql_data[0]["url"].c_str(), sql_data[0]["name"].c_str());

			//body为查询得到的内容
			body = sql_data[0]["description"] + " " + std::string(url);
			if (url)
				free(url);
		}
	}
	else {
		body = "抱歉，没有查询结果";
	}

	const  char* bodystr = body.c_str();

	//将查询结果写入buff 其中 query和body将填充templet.html中的两个%s
	sprintf(this->contentbuf, templetcontent, query.c_str(), bodystr);

	if (templetcontent)
		free(templetcontent);

	return strlen(this->contentbuf);
}


//根据请求内容返回相应文件
size_t HttpHandle::getFileContent(FILE *fd,struct stat *t) {

	if ((this->contentbuf = (char *)malloc(static_cast<size_t>(t->st_size))) == NULL)
		Logger::LogDebug(MALLOCFAILED + std::string(__FUNCTION__));

	memset(contentbuf, 0, strlen(contentbuf));
	if (fread(contentbuf, static_cast<size_t>(t->st_size), 1, fd) == 0)//将文件读取到buf
	{
		Logger::LogDebug("fread error in " + std::string(__FUNCTION__));
		return 0;
	}
	return static_cast<size_t >(t->st_size);
}

size_t HttpHandle::findRequestFile(std::string filename) {

	filename = "../www/" + filename;

	struct stat t;
	memset(&t, 0, sizeof(t));
	FILE *fd = fopen(filename.c_str(), "rb");
	size_t ret = 0;

	if (stat(filename.c_str(), &t) < 0) {
		//文件不存在 返回  404
		//只读方式打开参数filename指定的文件)
		Logger::LogDebug(filename + ": file not found return 404");
		filename = "../www/404.html";
		fd = fopen(filename.c_str(), "rb");
		stat(filename.c_str(), &t);
	}
	else if (fd  == NULL)
	{
		//文件打开失败 返回503
		Logger::LogDebug(filename + ": Can't open file , get 503");
		filename = "../www/503.html";
		stat(filename.c_str(), &t);
		fd = fopen(filename.c_str(), "rb");
	}
	else {
		//正常返回所请求的文件
		fd = fopen(filename.c_str(), "rb");

	}
	ret  = getFileContent(fd, &t);

	if(fd)
		fclose(fd);
	return ret;
}

void HttpHandle::fillReply(size_t iContentLen) {

	if ( iContentLen == 0) {
		Logger::LogDebug("contentbuf is empty");
		return; //没有获取到内容
	}

	char headbuf[1024];
	memset(headbuf, 0, sizeof(headbuf));

	sprintf(headbuf, HEAD, getFileType(this->_method_content), static_cast<int>(iContentLen)); //设置消息头

	size_t iheadlen = strlen(headbuf);//得到消息头长度
	size_t itaillen = strlen(TAIL);//得到消息尾长度
	size_t isumlen = iheadlen + iContentLen + itaillen;//得到消息总长度

	if ((this->reply_content = (char*)malloc(isumlen + 1)) == NULL)//根据消息总长度，动态分配内存
		Logger::LogDebug(MALLOCFAILED);
	memset(reply_content, 0, isumlen);

	memcpy(reply_content, headbuf, iheadlen); //安装消息头
	memcpy(&reply_content[iheadlen], contentbuf, static_cast<size_t>(iContentLen)); //安装消息体
	memcpy(&reply_content[iheadlen + iContentLen], TAIL, itaillen); //安装消息尾

	reply_content[isumlen + 1] = '\0';
	this->message_len = isumlen;
}


//////////////////////////////////////////////////////////////////////////////////////////////

const char *HttpHandle::getFileType(std::string &filename) {
	////////////得到文件扩展名///////////////////
	auto len = filename.size();

	auto pos = filename.find_first_of('.', 0);

	std::string sExt;
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
	if (strlen(httpstr) <= 0)
	{
		*stdstr ='\0';
		return;
	}

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
	stdstr[index] = '\0';
}
/////////////////////////////////////////////////////////////////////////////////////////////




