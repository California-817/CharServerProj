//读取config.json的配置
const fs=require('fs');

let config=JSON.parse(fs.readFileSync('config.json','utf-8')) //以utf8方式同步读取配置文件中的json字符串并解析
let email_user=config.email.user;
let email_pass=config.email.pass;

let mysql_host=config.mysql.host;
let mysql_port=config.mysql.port;
let mysql_password=config.mysql.password;

let redis_host=config.redis.host;
let redis_port=config.redis.port;
let redis_password=config.redis.password;

let code_prefix="code_";

module.exports={email_pass,email_user,mysql_host,mysql_password,mysql_port,redis_host,redis_password,redis_port,code_prefix}

