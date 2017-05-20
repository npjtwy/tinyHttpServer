
#pragma once
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <mysql/mysql.h>


namespace mysqlhelper {

//数据库异常类
struct MysqlHelper_Exception
{
    MysqlHelper_Exception(const std::string &sBuffer):
        errorInfo(sBuffer){}
    ~MysqlHelper_Exception() throw(){}

	std::string errorInfo;
};


//数据库配置接口
struct DBConf
{
	std::string host;
	std::string user;
	std::string passwd;
	std::string database;
    int port;
    int flag;//客户端标识

    //读取配置：
    void loadFromMap(const std::map<std::string, std::string> &mapParam)
    {
    	auto mpTmp = mapParam;
        host =      mpTmp["dbhost"];
        user =      mpTmp["dbuser"];
        passwd =    mpTmp["dbpass"];
        database =  mpTmp["dbname"];
        port = atoi(mpTmp["dbport"].c_str());

        flag        = 0;


        if (mpTmp["dbport"] == "")
            port = 3306;
    }
};



//mysql 查询的一条记录
class MsqlRecord
{
public:
    /**
    *  构造函数.
    */
    MsqlRecord(const std::map<std::string, std::string> &record);

    /**
    * @brief 获取数据，s一般是指数据表的某个字段名
    * @param s 要获取的字段
    * @return  符合查询条件的记录的s字段名
    */
    const std::string& operator[](const std::string &s);
protected:
    const std::map<std::string, std::string> &_record;

};

/**
     * @brief 查询出来的mysql数据
     */
class MysqlData
{
public:

	MysqlData() = default;
	~MysqlData() = default;

    /**
         * @brief 所有数据.
         *
         * @return vector<map<string,string>>&
         */
    std::vector<std::map<std::string, std::string> >& data() ;

    /**
         * 数据的记录条数
         * @return size_t
         */
    size_t size() const;

    /**
         * @brief 获取某一条记录.
         * @param i  要获取第几条记录
         * @return   MysqlRecord类型的数据，可以根据字段获取相关信息，
         */
    MsqlRecord operator[](size_t i);

	MysqlData(const MysqlData& rhs) ;
	MysqlData(MysqlData&& rhs) noexcept ;
	MysqlData& operator=(MysqlData&& rhs) noexcept ;
	MysqlData& operator=(const MysqlData& rhs)  ;


protected:
	std::vector<std::map<std::string, std::string> > _data;
};
class MysqlHelper
{
public:
    MysqlHelper();

    /**
       * @brief 构造函数.
       * @param: sHost:主机IP
       * @param sUser        用户
       * @param sPasswd      密码
       * @param sDatebase    数据库
       * @param port         端口
       * @param iUnixSocket  socket
       * @param iFlag        客户端标识
       */
    MysqlHelper(const std::string& sHost, const std::string& sUser = "", const std::string& sPasswd = "",
                const std::string& sDatabase = "", int port = 0, int iFlag = 0);
    //参数为数据库配置的构造函数
    MysqlHelper(const DBConf& tcDBConf);

    //析构函数
    ~MysqlHelper();
    void init(const std::string& sHost, const std::string& sUser  = "",
              const std::string& sPasswd  = "", const std::string& sDatabase = "",
             int port = 0, int iFlag = 0);
    void init(const DBConf & dbconf);

    void connect();

    // 断开数据库连接.
    void disconnect();

    //直接获取数据库指针.
    MYSQL *getMysql();

    //更新或者插入数据.
    void execute(const std::string& sSql);
    MysqlData queryRecord(const std::string& sSql);
    size_t getAffectedRows();

    void setDefalutCharacterSet();

private:

    /**
    * 数据库指针
    */
    MYSQL       *_pstMql;

    /**
    * 数据库配置
    */
    DBConf   _dbConf;

    /**
    * 是否已经连接
    */
    bool        _bConnected;

    /**
     * 最后执行的sql
     */
    std::string      _sLastSql;

};
}
