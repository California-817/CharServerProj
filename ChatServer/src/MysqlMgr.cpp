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
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
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
    userinfo=UserInfo(p_uid,p_name,p_email,p_password,p_nick,p_desc,p_icon,p_sex,"");
    mysql_stmt_close(stmt);
    return true;
}

bool MysqlMgr::GetUserInfo(std::string name,UserInfo& userinfo) //根据名字查找用户信息
{
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
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
    userinfo=UserInfo(p_uid,p_name,p_email,p_password,p_nick,p_desc,p_icon,p_sex,"");
    mysql_stmt_close(stmt);
    return true;   
}

bool MysqlMgr::AddFriendApply(int from_uid,int to_uid) //向friend_apply表中添加一条好友申请记录
{
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
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
//获取这个user的所有申请人列表
bool MysqlMgr::GetFriendApply(int uid,std::vector<std::shared_ptr<ApplyInfo>>& applylist,int begin,int end) 
{
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
    //联表查询申请人信息
    std::string query="SELECT friend_apply.from_uid,user.name,user.nick,user.user_desc,user.icon,user.sex,friend_apply.apply_status ";
    query+="FROM friend_apply INNER JOIN user ";
    query+="on friend_apply.from_uid=user.uid ";
    query+="WHERE friend_apply.to_uid=? AND friend_apply.id>? ORDER BY friend_apply.id ASC LIMIT ?";
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];
    stmt=mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    memset(bind,0,sizeof bind);
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=(char*)&uid;
    bind[1].buffer_type=MYSQL_TYPE_LONG;
    bind[1].buffer=(char*)&begin;    
    bind[2].buffer_type=MYSQL_TYPE_LONG;
    bind[2].buffer=(char*)&end;
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
        std::cout << "没有申请好友信息 " << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
    // int _uid; //from_uid 对方的uid
	// std::string _name;
    // std::string _nick;
	// std::string _desc;
	// std::string _icon;
	// int _sex;
	// int _status; //是否已经添加
    int p_from_uid;
    char p_name[256];
    char p_nick[256];
    char p_desc[256];
    char p_icon[256];
    int p_sex;
    signed char status;
    MYSQL_BIND result_bind[7];
    memset(result_bind,0,sizeof result_bind);
    result_bind[0].buffer_type=MYSQL_TYPE_LONG;
    result_bind[0].buffer=(char*)&p_from_uid;
    for(int i=1;i<5;i++)
    {
        result_bind[i].buffer_type=MYSQL_TYPE_STRING;
        result_bind[i].buffer_length=256;}
    result_bind[1].buffer=p_name;
    result_bind[2].buffer=p_nick;
    result_bind[3].buffer=p_desc;
    result_bind[4].buffer=p_icon;
    result_bind[5].buffer_type=MYSQL_TYPE_LONG;
    result_bind[5].buffer=(char*)&p_sex;
    result_bind[6].buffer_type=MYSQL_TYPE_TINY;
    result_bind[6].buffer=(char*)&status;
    //2.绑定结果集存放处
    if (mysql_stmt_bind_result(stmt, result_bind)) { 
        std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //3.真正去获取一行行数据到绑定到的地址
    while (mysql_stmt_fetch(stmt) == 0) { //一行一行遍历结果集
      std::shared_ptr<ApplyInfo> _applyinfo=std::make_shared<ApplyInfo>();
      _applyinfo->_desc=p_desc;
      _applyinfo->_icon=p_icon;
      _applyinfo->_name=p_name;
      _applyinfo->_nick=p_nick;
      _applyinfo->_sex=p_sex;
      _applyinfo->_status=status;
      _applyinfo->_uid=p_from_uid;
      applylist.push_back(std::move(_applyinfo));
    }
    mysql_stmt_close(stmt);
    return true;   
}
 //获取这个user的所有好友列表
bool MysqlMgr::GetFriendList(int uid,std::vector<std::shared_ptr<UserInfo>>& friendlist)
{
    std::vector<std::pair<int,std::string>> _friends;
{
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    std::string query="select to_uid,back from friend where self_uid= ?";
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
        std::cout << "没有好友列表信息 " << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }
    int p_self_uid;
    char p_back[256];
    MYSQL_BIND result_bind[2];
    memset(result_bind,0,sizeof result_bind);
    result_bind[0].buffer_type=MYSQL_TYPE_LONG;
    result_bind[0].buffer=(char*)&p_self_uid;
    result_bind[1].buffer_type=MYSQL_TYPE_STRING;
    result_bind[1].buffer=(char*)p_back;
    result_bind[1].buffer_length=256;
    //2.绑定结果集存放处
    if (mysql_stmt_bind_result(stmt, result_bind)) { 
        std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;}
    //3.真正去获取一行行数据到绑定到的地址
    while (mysql_stmt_fetch(stmt) == 0) { //一行一行遍历结果集
        _friends.push_back(std::make_pair(p_self_uid,p_back));
    }
    mysql_stmt_close(stmt);
}
    //得到friend的uid了
    for(auto& frd:_friends)
    {
        std::shared_ptr<UserInfo> userinfo=std::make_shared<UserInfo>();
         MysqlMgr::GetUserInfo(frd.first, *userinfo) ;//根据uid查找用户信息
         userinfo->_back=frd.second;
         friendlist.push_back(std::move(userinfo));
    }
    return true;       
}
//更新申请列表 向好友列表添加好友记录
bool MysqlMgr::AuthAddFriend(int from_uid,int to_uid,std::string back)
{
    auto shared_con=_pool->GetConnection(); //该智能指针对象会一直持续到话括号结束再析构 归还连接
    auto con=shared_con->_con.get(); //获取原始连接
    shared_con->UpdateTimeStamp(); //更新时间戳
    Defer defer([&shared_con,this](){ //出作用域自动归还链接
        _pool->ReturnConnection(shared_con);
    });
    MYSQL_STMT* stmt1;
    MYSQL_STMT* stmt2;
    MYSQL_STMT* stmt3;
    MYSQL_BIND bind1[2];
    MYSQL_BIND bind2[3];
    MYSQL_BIND bind3[3];
    std::string query1="update friend_apply set apply_status = 1 where from_uid=? and to_uid=?";
    std::string query2="insert into friend (self_uid,to_uid,back) values (?,?,?)";
    std::string query3="insert into friend (self_uid,to_uid,back) values (?,?,?)";
    stmt1=mysql_stmt_init(con);
    stmt2=mysql_stmt_init(con);
    stmt3=mysql_stmt_init(con);
    Defer defer([&stmt1,&stmt2,&stmt3](){
        mysql_stmt_close(stmt1);
        mysql_stmt_close(stmt2);
        mysql_stmt_close(stmt3);
    });
    if (mysql_stmt_prepare(stmt1, query1.c_str(), query1.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt1) << std::endl;
        return false;}
    if (mysql_stmt_prepare(stmt2, query2.c_str(), query2.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt2) << std::endl;
        return false;}
    if (mysql_stmt_prepare(stmt3, query3.c_str(), query3.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt3) << std::endl;
        return false;}
    memset(bind1,0,sizeof bind1);
    memset(bind2,0,sizeof bind2);
    memset(bind3,0,sizeof bind3);
    bind1[0].buffer_type=MYSQL_TYPE_LONG;
    bind1[0].buffer=(char*)&to_uid;
    bind1[1].buffer_type=MYSQL_TYPE_LONG;
    bind1[1].buffer=(char*)&from_uid;

    bind2[0].buffer_type=MYSQL_TYPE_LONG;
    bind2[0].buffer=(char*)&from_uid;
    bind2[1].buffer_type=MYSQL_TYPE_LONG;
    bind2[1].buffer=(char*)&to_uid;
    bind2[2].buffer_type=MYSQL_TYPE_STRING;
    bind2[2].buffer=(char*)back.c_str();
    bind2[2].buffer_length=back.length();

    bind3[0].buffer_type=MYSQL_TYPE_LONG;
    bind3[0].buffer=(char*)&to_uid;
    bind3[1].buffer_type=MYSQL_TYPE_LONG;
    bind3[1].buffer=(char*)&from_uid;
    bind3[2].buffer_type=MYSQL_TYPE_STRING;
    std::string form_back=" "; //这个申请人设置的备注在添加好友的时候没有进行保留 这里用空字符串表示 后面进行添加
    bind3[2].buffer=(char*)form_back.c_str();
    bind3[2].buffer_length=form_back.length();
    if (mysql_stmt_bind_param(stmt1, bind1)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt1) << std::endl;
        return false;}
    if (mysql_stmt_bind_param(stmt2, bind2)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt2) << std::endl;
        return false;}
    if (mysql_stmt_bind_param(stmt3, bind3)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt3) << std::endl;
        return false;}
    //开启一个事务
    if(mysql_query(con,"START TRANSACTION"))
    { //开启事务失败
        std::cerr << "Error: " << mysql_error(con) << std::endl;
        return false;
    }
    // 执行预处理语句
    if (mysql_stmt_execute(stmt1)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt1) << std::endl;
        //某一条语句失败则进行回滚事务
        mysql_query(con, "ROLLBACK");
        return false;}
    if (mysql_stmt_execute(stmt2)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt2) << std::endl;
        mysql_query(con, "ROLLBACK");
        return false;}
    if (mysql_stmt_execute(stmt3)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt3) << std::endl;
        mysql_query(con, "ROLLBACK");
        return false;}
    //三条语句都执行成功 进行事务提交
    if(mysql_query(con, "COMMIT"))
    { //提交事务失败
        std::cerr << "Error: " << mysql_error(con) << std::endl;
        return false;        
    }
    //事务执行完成并成功提交
    return true;
}




