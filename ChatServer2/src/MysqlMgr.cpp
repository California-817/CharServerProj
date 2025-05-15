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

bool MysqlMgr::GetUserInfo(int uid,UserInfo& userinfo) //根据uid查找用户信息
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    std::string query="select * from user where uid= ?";
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=(char*)&uid;
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
    //判断是否有数据
    if(!mysql_stmt_num_rows(stmt))
    {//没有数据
        std::cout << "没有该用户信息 "<< std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
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
    //填入用户信息
    userinfo=UserInfo(p_uid,p_name,p_email,p_password,p_nick,p_desc,p_icon,p_sex);
    mysql_stmt_close(stmt);
    return true;
}

bool MysqlMgr::GetUserInfo(std::string name,UserInfo& userinfo) //根据名字查找用户信息
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    std::string query="select * from user where name= ?";
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
    //1.获取结果集的元数据
    if (mysql_stmt_store_result(stmt)) {
        std::cerr << "mysql_stmt_store_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //判断是否有数据
    if(!mysql_stmt_num_rows(stmt))
    {//没有数据
        std::cout << "没有该用户信息 " << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
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
    //填入用户信息
    userinfo=UserInfo(p_uid,p_name,p_email,p_password,p_nick,p_desc,p_icon,p_sex);
    mysql_stmt_close(stmt);
    return true;   
}

bool MysqlMgr::AddFriendApply(int from_uid,int to_uid) //向friend_apply表中添加一条好友申请记录
{
    auto con=_pool->GetConnection().get();
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];
    std::string query="INSERT INTO friend_apply (from_uid,to_uid) VALUES (?,?) ";
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=(char*)&from_uid;
    bind[1].buffer_type=MYSQL_TYPE_LONG;
    bind[1].buffer=(char*)&to_uid;
    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    // 执行预处理语句
     if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //执行增加记录语句成功
    mysql_stmt_close(stmt);
    return true;
}

