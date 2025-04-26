//调用发送验证码服务的逻辑
const nodemailer=require('nodemailer');
const config_module=require('./config') //config.js模块读取了json配置文件

//创建发送邮件的代理
let transport=nodemailer.createTransport({
    host: 'smtp.qq.com' ,//链接哪个服务器
    port: 465, //端口
    secure :true,
    auth: {
        user: config_module.email_user,//发送邮件的发送方邮箱地址
        pass: config_module.email_pass //邮箱授权码
    }
});

//发送邮件的函数 使用nodemailer的库函数
function SendMail(mailOptions_){
    return new Promise(function(resolve,reject){ //promise类似future 是为了将异步化为同步
        transport.sendMail(mailOptions_,function(error,info){ 
            //sendMail是异步发送函数 通过这个回调函数通知发送完成并告知发送结果
            if(error){ //发送失败
                console.log(error);
                reject(error);
            }else{ //发送成功
                console.log('邮件已成功发送：'+info.response);
                resolve(info.response)
            }
        })
    })
}
//导出函数共Server.js使用
module.exports.SendMail=SendMail