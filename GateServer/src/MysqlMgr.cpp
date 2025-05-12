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
int MysqlMgr::RegUser(const std::string& name,const std::string& email,const std::string& password,
                        const std::string& nick,const std::string& icon,int sex)  //注册用户的sql逻辑 --防止sql注入的逻辑
{       // (new_id,new_name,new_email,new_password,new_nick,new_icon,new_sex);
    auto conn=_pool->GetConnection(); //获取了一个mysql连接
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[6];
    MYSQL_RES* result;
    MYSQL_ROW row;  
    std::string query="call reg_user(?,?,?,?,?,?,@result)";
    stmt=mysql_stmt_init(conn.get());  //初始化预处理语句
    if(mysql_stmt_prepare(stmt,query.c_str(),query.length()))
    { 
        std::cout<<"mysql_stmt_prepare err: "<<mysql_stmt_error(stmt)<<std::endl;
        mysql_stmt_close(stmt);
        return -3;
    }
    //绑定参数
    memset(bind,0,sizeof bind);
    for(int i=0;i<5;i++){
    bind[i].buffer_type=MYSQL_TYPE_STRING;}
    bind[0].buffer=(char*)name.c_str();
    bind[0].buffer_length=name.length();
    bind[1].buffer=(char*)email.c_str();
    bind[1].buffer_length=email.length();  
    bind[2].buffer=(char*)password.c_str();
    bind[2].buffer_length=password.length();
    bind[3].buffer=(char*)nick.c_str();
    bind[3].buffer_length=nick.length();    
    bind[4].buffer=(char*)icon.c_str();
    bind[4].buffer_length=icon.length();    
    bind[5].buffer=(char*)&sex;
    bind[5].buffer_type=MYSQL_TYPE_LONG;
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
    //它的主要作用是 [将查询结果从服务器一次性传输到客户端，并存储在客户端的内存中。这种方式适用于结果集较小的场景]
    //• 优势：• 快速访问：结果集存储在客户端内存中，访问速度非常快。• 简单易用：适合结果集较小的场景，使用起来非常方便
    //• 限制：• 内存占用：客户端需要为整个结果集分配足够的内存，如果结果集非常大，可能会导致内存不足。• 不适合大数据集：
    //如果结果集非常大（例如，包含数百万行数据），一次性加载到内存中可能会导致性能问题或内存不足。

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
    //mysql_stmt_store_result优势:
    //减少内存占用：• 客户端不需要一次性为整个结果集分配内存，只需为当前行分配内存。• 特别适用于处理大型结果集，可以有效避免内存不足的问题。
    //提高性能：• 通过逐步加载数据，客户端可以按需从服务器获取数据，减少不必要的数据传输。• 适用于需要逐行处理结果集的场景，例如数据处理或分析。

    //1.调用   mysql_stmt_store_result   时，MySQL 会将结果集的 [元数据] 传输到客户端，但不会立即传输所有数据行。
    //•这个函数的主要作用是初始化结果集的存储，为后续的逐步加载做准备。
    if (mysql_stmt_store_result(stmt)) {
        std::cerr << "mysql_stmt_store_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    MYSQL_BIND result_bind[1];//第一列结果集的存储结构
    memset(result_bind,0,sizeof result_bind);
    result_bind[0].buffer_type=MYSQL_TYPE_STRING;
    result_bind[0].buffer=(char*)malloc(256);//真正存放处
    result_bind[0].buffer_length=256;
    //2.通过  mysql_stmt_bind_result  ，客户端为 [每一列分配一个缓冲区] ，并将这些缓冲区的地址绑定到预处理语句的列上。
    //•这些缓冲区用于存储从服务器逐步获取的数据行。
    if (mysql_stmt_bind_result(stmt, result_bind)) { 
        std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        free(result_bind[0].buffer);
        mysql_stmt_close(stmt);
        return false;
    }
    //3.每次调用 mysql_stmt_fetch   时，MySQL 会从服务器获取一行数据，并将其存储到绑定的缓冲区中。
    //•[客户端只需要为当前正在处理的行分配内存，而不需要为整个结果集分配内存]。• 这种按需加载的方式大大减少了内存占用
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

bool MysqlMgr::CheckPwd(const std::string& email,const std::string& password,UserInfo& userinfo)
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    std::string query="select * from user where email= ?";
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    bind[0].buffer_type=MYSQL_TYPE_STRING;
    bind[0].buffer=(char*)email.c_str();
    bind[0].buffer_length=email.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    // 执行预处理语句
     if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //1.获取结果集的元数据
    if (mysql_stmt_store_result(stmt)) {
        std::cerr << "mysql_stmt_store_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    int p_id;
    int p_uid;
    char p_name[256];
    char p_email[256];
    char p_password[256];
    char p_nick[256];
    char p_desc[256];
    char p_icon[256];
    int p_sex;
    MYSQL_BIND result_bind[9];
    memset(result_bind,0,sizeof result_bind);
    result_bind[0].buffer_type=MYSQL_TYPE_LONG;
    result_bind[0].buffer=(char*)&p_id;
    result_bind[1].buffer_type=MYSQL_TYPE_LONG;
    result_bind[1].buffer=(char*)&p_uid;
    for(int i=2;i<8;i++)
    {
        result_bind[i].buffer_type=MYSQL_TYPE_STRING;
        result_bind[i].buffer_length=256;}
    result_bind[2].buffer=p_name;
    result_bind[3].buffer=p_email;
    result_bind[4].buffer=p_password;
    result_bind[5].buffer=p_nick;
    result_bind[6].buffer=p_desc;
    result_bind[7].buffer=p_icon;
    result_bind[8].buffer_type=MYSQL_TYPE_LONG;
    result_bind[8].buffer=(char*)&p_sex;
    //2.绑定结果集存放处
    if (mysql_stmt_bind_result(stmt, result_bind)) { 
        std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //3.真正去获取一行行数据到绑定到的地址
    while (mysql_stmt_fetch(stmt) == 0) { //一行一行遍历结果集
       std::cout<<p_id<<" | "<<p_uid<<" | "<<p_name<<" | "<<p_email<<" | "<<p_password<<std::endl;
    }
    if(password!=p_password)
    {
        mysql_stmt_close(stmt);
        return false;}
    //密码与邮箱匹配
    userinfo.uid=p_uid;
    userinfo.name=p_name;
    userinfo.email=p_email;
    userinfo.password=p_password;
    mysql_stmt_close(stmt);
    return true;
}