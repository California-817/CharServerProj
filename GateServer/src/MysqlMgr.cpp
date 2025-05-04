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
    auto conn=_pool->GetConnection(); //获取了一个mysql连接
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
        mysql_stmt_close(stmt);
        return -6;
    }
    //结果集存储在conn中 需要手动获取
    //获取结果集
    result=mysql_store_result(conn.get());
    if(result==nullptr)
    {
        std::cerr << "mysql_store_result() failed: " << mysql_error(conn.get()) << std::endl;
        mysql_stmt_close(stmt);
        return -7;
    }
    //遍历结果集 只有1行
    while((row=mysql_fetch_row(result)))
    {
        std::cout<<row[0]<<std::endl;
        auto uid=atoi(row[0]);
        if(result)
        {mysql_free_result(result);}
        mysql_stmt_close(stmt);
        return uid;
    }
    if(result)
    {mysql_free_result(result);}
    mysql_stmt_close(stmt);
    return -8;
}

bool MysqlMgr::CheckEmail(const std::string& name,const std::string& email)
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    std::string select_email;
    std::string query="select email from user where name=?";
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    bind[0].buffer_type=MYSQL_TYPE_STRING;
    bind[0].buffer=(char*)name.c_str();
    bind[0].buffer_length=name.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
     // 执行预处理语句
     if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //获取结果集
    //使用了mysql_stmt_prepare 和mysql_stmt_execut来执行预处理语句，但随后又调用了 mysql_store_result这是不正确的，
    //因为 mysql_store_result 是用于处理普通查询的结果集，而不是预处理语句的结果集
    //对于预处理语句 应该使用 mysql_stmt_store_result 和 mysql_stmt_fetch来处理结果集。
    // res=mysql_store_result(con);
    // if(res==nullptr)
    // {
    //     std::cerr << "mysql_store_result() failed: " << mysql_error(con) << std::endl;
    //     mysql_stmt_close(stmt);
    //     return false;
    // }
    // while((row=mysql_fetch_row(res)))
    // {
    //     std::cout<<row[0]<<std::endl;
    //     select_email=row[0];
    // }
    // 使用 mysql_stmt_store_result 和 mysql_stmt_fetch
    if (mysql_stmt_store_result(stmt)) {
        std::cerr << "mysql_stmt_store_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    MYSQL_BIND result_bind[1];//第一列结果集的存储结构
    memset(result_bind,0,sizeof result_bind);
    result_bind[0].buffer_type=MYSQL_TYPE_STRING;
    result_bind[0].buffer=(char*)malloc(256);//真正存放处
    result_bind[0].buffer_length=256;
    if (mysql_stmt_bind_result(stmt, result_bind)) { 
        std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        free(result_bind[0].buffer);
        mysql_stmt_close(stmt);
        return false;
    }
    while (mysql_stmt_fetch(stmt) == 0) { //一行一行遍历结果集
        select_email = (char*)result_bind[0].buffer; //每次得到的都是第一列的第n行数据
        std::cout << select_email << std::endl;
    }
    free(result_bind[0].buffer); //malloc出来的空间 需要手动释放
    mysql_stmt_close(stmt);
    if(select_email!=email)
    {
        return false;
    }
    return true;
}

bool MysqlMgr::UpdatePwd(const std::string& name,const std::string& password)
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];
    std::string query="update user set password = ? where name = ?";
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    for(int i=0;i<2;i++)
    {
        bind[i].buffer_type=MYSQL_TYPE_STRING;}
    bind[0].buffer=(char*)password.c_str();
    bind[0].buffer_length=password.length();
    bind[1].buffer=(char*)name.c_str();
    bind[1].buffer_length=name.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
          // 执行预处理语句
     if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    return true;
}