#include"../include/MysqlMgr.h"
MysqlMgr::MysqlMgr()
{
    mINI::INIFile file("../conf/config.ini");
    mINI::INIStructure ini;
    file.read(ini);
    std::string host=ini["Mysql"]["host"];
    std::string port=ini["Mysql"]["port"];
    std::string user=ini["Mysql"]["user"];
    std::string password=ini["Mysql"]["password"];
    std::string database=ini["Mysql"]["database"];
    std::string size=ini["MysqlPool"]["size"];
    std::string max_wait_time=ini["MysqlPool"]["maxwaittime"];
    _pool=std::make_unique<ConnectionPool>(host,static_cast<uint16_t>(atoi(port.c_str())),user,
                    password,database,atoi(size.c_str()),atoi(max_wait_time.c_str()));
}
int MysqlMgr::RegUser(const std::string& name,const std::string& email,const std::string& password)  //注册用户的sql逻辑 --防止sql注入的逻辑
{
    auto conn=_pool->GetConnecion(); //获取了一个mysql连接
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];
    MYSQL_RES* result;
    MYSQL_ROW row;  
    std::string query="call reg_user(?,?,?,@result)";
    stmt=mysql_stmt_init(conn.get());  //初始化预处理语句
    if(mysql_stmt_prepare(stmt,query.c_str(),query.length()))
    { 
        std::cout<<"mysql_stmt_prepare err: "<<mysql_stmt_error(stmt)<<std::endl;
        mysql_stmt_close(stmt);
        return -3;
    }
    //绑定参数
    memset(bind,0,sizeof bind);
    for(int i=0;i<3;i++){
    bind[i].buffer_type=MYSQL_TYPE_STRING;}
    bind[0].buffer=(char*)name.c_str();
    bind[0].buffer_length=name.length();
    bind[1].buffer=(char*)email.c_str();
    bind[1].buffer_length=email.length();  
    bind[2].buffer=(char*)password.c_str();
    bind[2].buffer_length=password.length();
    if(mysql_stmt_bind_param(stmt,bind))
    {
        std::cout<<"mysql_stmt_bind_param err: "<<mysql_stmt_error(stmt)<<std::endl;
        mysql_stmt_close(stmt);
        return -4;
    }
    //执行预处理语句
    if(mysql_stmt_execute(stmt))
    {
        std::cout<<"mysql_stmt_execute err: "<<mysql_stmt_error(stmt)<<std::endl;
        mysql_stmt_close(stmt);
        return -5;
    }
    //执行完存储过程了 查询用户自定义变量获取存储过程的输出值
    std::string select_query="select @result;";
    if(mysql_query(conn.get(),select_query.c_str()))
    {
        std::cerr << "mysql_query() failed: " << mysql_error(conn.get()) << std::endl;
        return -6;
    }
    //结果集存储在conn中 需要手动获取
    //获取结果集
    result=mysql_store_result(conn.get());
    if(result==nullptr)
    {
        std::cerr << "mysql_store_result() failed: " << mysql_error(conn.get()) << std::endl;
        return -7;
    }
    //遍历结果集 只有1行
    while((row=mysql_fetch_row(result)))
    {
        std::cout<<row[0]<<std::endl;
        return atoi(row[0]);
    }
    return -8;
}