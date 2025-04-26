const path=require('path')
const grpc=require('@grpc/grpc-js')
const protoLoader=require('@grpc/proto-loader')

const PROTO_PATH=path.join(__dirname,'GateServer.Varify.proto')
//同步加载proto文件并定义解析方案 生成packageDefinition这个类
const packageDefinition=protoLoader.loadSync(PROTO_PATH,{keepCase:true,longs:String,
    enums:String,defaults:true,oneofs:true
})
//grpc解析生成的类生成对象
const protoDescriptor=grpc.loadPackageDefinition(packageDefinition)

const message_proto1=protoDescriptor.GateServer.Varify

//message_proto导出供其他js文件使用该类
module.exports=message_proto1

