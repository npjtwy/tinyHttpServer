#include "MysqlHelper.h"

mysqlhelper::MysqlHelper::MysqlHelper() : _bConnected(false)
{
    _pstMql = mysql_init(NULL);
}

mysqlhelper::MysqlHelper::MysqlHelper(const std::string &sHost, const std::string &sUser,
                                      const std::string &sPasswd, const std::string &sDatabase,
                                      int port, int iFlag)
{
    this->init(sHost, sUser, sPasswd, sDatabase, port, iFlag);
    this->_pstMql = mysql_init(NULL);
    this->_bConnected = false;
}

mysqlhelper::MysqlHelper::MysqlHelper(const mysqlhelper::DBConf &tcDBConf) : _bConnected(false)
{
    this->init(tcDBConf);
    this->_pstMql = mysql_init(NULL);
    this->_dbConf = tcDBConf;
}

mysqlhelper::MysqlHelper::~MysqlHelper()
{
    if (_pstMql != NULL)
    {
        mysql_close(_pstMql);
        _pstMql = NULL;
    }
}

//初始化数据库配置文件
void mysqlhelper::MysqlHelper::init(const std::string &sHost, const std::string &sUser,
                                    const std::string &sPasswd, const std::string &sDatabase,
                                    int port, int iFlag)
{
    _dbConf.host =       sHost;
    _dbConf.user =       sUser;
    _dbConf.passwd =     sPasswd;
    _dbConf.database =   sDatabase;

    _dbConf.flag =       iFlag;

}

void mysqlhelper::MysqlHelper::init(const mysqlhelper::DBConf &dbconf)
{
    _dbConf = dbconf;

}

void mysqlhelper::MysqlHelper::connect()
{
    disconnect();

    if (_pstMql == NULL)
    {
        _pstMql = mysql_init(NULL);
    }

    //连接数据库
    if (mysql_real_connect(_pstMql, _dbConf.host.c_str(), _dbConf.user.c_str(),_dbConf.passwd.c_str(),
                           _dbConf.database.c_str(), _dbConf.port, NULL, _dbConf.flag) == NULL)
    {
        //连接失败抛出异常
        throw MysqlHelper_Exception("[MysqlHelper::connect]: mysql_real_connect: " +
                                    std::string(mysql_error(_pstMql)));
    }

    std::cout << "数据库连接成功" << std::endl;
    _bConnected = true;
}

void mysqlhelper::MysqlHelper::disconnect()
{
    if (_pstMql != NULL)
    {
        mysql_close(_pstMql);
        _pstMql = mysql_init(NULL);
    }

    _bConnected = false;
}


//执行sql语句
void mysqlhelper::MysqlHelper::execute(const std::string &sSql)
{
    /**
    没有连上, 连接数据库
    */
    if(!_bConnected)
    {
        connect();
    }

    _sLastSql = sSql;

    int iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
    if(iRet != 0)
    {
        /**
        自动重新连接
        */
        int iErrno = mysql_errno(_pstMql);
        if (iErrno == 2013 || iErrno == 2006)
        {
            connect();
            iRet = mysql_real_query(_pstMql, sSql.c_str(), sSql.length());
        }
    }

    if (iRet != 0)
    {
       const char *err_str = mysql_error(_pstMql);
       std::cout << err_str << std::endl;
        throw MysqlHelper_Exception("[MysqlHelper::execute]: mysql_query: [ " + sSql+" ] :" +
                                    std::string(mysql_error(_pstMql))+"\n");
    }
}


size_t mysqlhelper::MysqlHelper::getAffectedRows()
{
    return mysql_affected_rows(_pstMql);
}

void mysqlhelper::MysqlHelper::setDefalutCharacterSet()
{
    //连接后设置字符集
        this->execute("set names utf8");

}

MYSQL *mysqlhelper::MysqlHelper::getMysql()
{
    return _pstMql;
}

//查询结果的保存
mysqlhelper::MysqlData mysqlhelper::MysqlHelper::queryRecord(const std::string &sSql)
{
    MysqlData   data;

    //执行语句
    this->execute(sSql);

    MYSQL_RES *pstRes = mysql_store_result(_pstMql);

    if(pstRes == NULL)
    {
        throw MysqlHelper_Exception("[MysqlHelper::queryRecord]: mysql_store_result: " +
                                    sSql + " : " + std::string(mysql_error(_pstMql)));
    }

    std::vector<std::string> vtFields;//存储字段
    MYSQL_FIELD *field;
    //获取字段
    while((field = mysql_fetch_field(pstRes)))
    {
        vtFields.push_back(field->name);
    }

    std::map<std::string, std::string> mpRow;
    MYSQL_ROW stRow;

    //逐行保存查询结果
    while((stRow = mysql_fetch_row(pstRes)) != (MYSQL_ROW)NULL)
    {
        mpRow.clear();
        //以数组返回上一次用 mysql_fetch_row() 取得的行中每个字段的长度，如果出错返回 FALSE。
        unsigned long * lengths = mysql_fetch_lengths(pstRes);
        for(size_t i = 0; i < vtFields.size(); i++)
        {
            if(stRow[i])
            {
                //将每个字段和其所对应的内容存入map
                mpRow[vtFields[i]] = std::string(stRow[i], lengths[i]);
            }
            else
            {
                mpRow[vtFields[i]] = "";
            }
        }

        //查询结果存入vector
        data.data().push_back(mpRow);
    }

    mysql_free_result(pstRes);

    return data;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
mysqlhelper::MsqlRecord::MsqlRecord(const std::map<std::string, std::string> &record):_record(record){}

const std::string& mysqlhelper::MsqlRecord::operator[](const std::string &s)
{
    std::map<std::string, std::string>::const_iterator it = _record.find(s);
    if(it == _record.end())
    {
        throw MysqlHelper_Exception("field '" + s + "' not exists.");
    }
    return it->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::map<std::string, std::string> >& mysqlhelper::MysqlData::data()
{
    return this->_data;
}

size_t mysqlhelper::MysqlData::size() const
{
    return this->_data.size();
}

mysqlhelper::MsqlRecord mysqlhelper::MysqlData::operator[](size_t i)
{
    return MsqlRecord(_data[i]);
}

mysqlhelper::MysqlData &mysqlhelper::MysqlData::operator=(mysqlhelper::MysqlData &&rhs) noexcept {
    this->_data = std::move(rhs._data);
	return *this;
}

mysqlhelper::MysqlData &mysqlhelper::MysqlData::operator=(const mysqlhelper::MysqlData &rhs)
{
    this->_data = rhs._data;
	return *this;
}

mysqlhelper::MysqlData::MysqlData(const mysqlhelper::MysqlData &rhs) {
    this->_data = rhs._data;

}

mysqlhelper::MysqlData::MysqlData(mysqlhelper::MysqlData &&rhs) noexcept {
	this->_data = std::move(rhs._data);

}


