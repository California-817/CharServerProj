const grpc=require('@grpc/grpc-js')
const message_proto1 = require('./proto')
const const_module=require('./const')
const redis_module=require('./redis')
const {v4:uuidv4}=require('uuid');
const emailModule=require('./email');
async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        //先去查redis看是否已经有未过期的验证码
        let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);
        console.log("query_res is ", query_res)
        let uniqueId = query_res;
        if(query_res ==null){ //没有未过期的验证码
            uniqueId = uuidv4(); //生成验证码
            if (uniqueId.length > 4) {
                uniqueId = uniqueId.substring(0, 4); //取4位
            } 
            //设置过期时间1分钟
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,60)
            if(!bres){ //redis设置失败
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
                });
                return;
            }
            console.log("uniqueId is ", uniqueId)
            let text_str =  '您的验证码为'+ uniqueId +'请一分钟内完成注册'
            //发送邮件（只有在redis中没有有效验证码的时候发送） redis设置成功
            let mailOptions = {
            from: '1876439531@qq.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
             };
            let send_res = await emailModule.SendMail(mailOptions);
            console.log("send res is ", send_res)
            callback(null, { email:  call.request.email,
            error:const_module.Errors.Success}); 
                return;
        }
        //redis中还有有效的验证码 不会再次发送 直接返回grpc结果给GateServer
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success});
    }
    catch(error){
        console.log("catch error is ", error)
        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
}
function main(){
    var server=new grpc.Server()
    server.addService(message_proto1.VarifyServer.service,{GetVarifyCode:GetVarifyCode})
    server.bindAsync('0.0.0.0:8081',grpc.ServerCredentials.createInsecure(),()=>{
        console.log('grpc server start')
    })
}
main()