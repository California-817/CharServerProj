// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: GateServer.Varify.proto
#ifndef GRPC_GateServer_2eVarify_2eproto__INCLUDED
#define GRPC_GateServer_2eVarify_2eproto__INCLUDED

#include "GateServer.Varify.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/status.h>
#include <grpcpp/support/stub_options.h>
#include <grpcpp/support/sync_stream.h>
#include <grpcpp/ports_def.inc>

namespace GateServer {
namespace Varify {

class VarifyServer final {
 public:
  static constexpr char const* service_full_name() {
    return "GateServer.Varify.VarifyServer";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::GateServer::Varify::GetVarifyRsp* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>> AsyncGetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>>(AsyncGetVarifyCodeRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>> PrepareAsyncGetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>>(PrepareAsyncGetVarifyCodeRaw(context, request, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response, std::function<void(::grpc::Status)>) = 0;
      virtual void GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response, ::grpc::ClientUnaryReactor* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>* AsyncGetVarifyCodeRaw(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::GateServer::Varify::GetVarifyRsp>* PrepareAsyncGetVarifyCodeRaw(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::GateServer::Varify::GetVarifyRsp* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>> AsyncGetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>>(AsyncGetVarifyCodeRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>> PrepareAsyncGetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>>(PrepareAsyncGetVarifyCodeRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response, std::function<void(::grpc::Status)>) override;
      void GetVarifyCode(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>* AsyncGetVarifyCodeRaw(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::GateServer::Varify::GetVarifyRsp>* PrepareAsyncGetVarifyCodeRaw(::grpc::ClientContext* context, const ::GateServer::Varify::GetVarifyReq& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_GetVarifyCode_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status GetVarifyCode(::grpc::ServerContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetVarifyCode(::grpc::ServerContext* context, ::GateServer::Varify::GetVarifyReq* request, ::grpc::ServerAsyncResponseWriter< ::GateServer::Varify::GetVarifyRsp>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_GetVarifyCode<Service > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::GateServer::Varify::GetVarifyReq, ::GateServer::Varify::GetVarifyRsp>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::GateServer::Varify::GetVarifyReq* request, ::GateServer::Varify::GetVarifyRsp* response) { return this->GetVarifyCode(context, request, response); }));}
    void SetMessageAllocatorFor_GetVarifyCode(
        ::grpc::MessageAllocator< ::GateServer::Varify::GetVarifyReq, ::GateServer::Varify::GetVarifyRsp>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::GateServer::Varify::GetVarifyReq, ::GateServer::Varify::GetVarifyRsp>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetVarifyCode(
      ::grpc::CallbackServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_GetVarifyCode<Service > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetVarifyCode(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->GetVarifyCode(context, request, response); }));
    }
    ~WithRawCallbackMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetVarifyCode(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_GetVarifyCode : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_GetVarifyCode() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::GateServer::Varify::GetVarifyReq, ::GateServer::Varify::GetVarifyRsp>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::GateServer::Varify::GetVarifyReq, ::GateServer::Varify::GetVarifyRsp>* streamer) {
                       return this->StreamedGetVarifyCode(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_GetVarifyCode() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status GetVarifyCode(::grpc::ServerContext* /*context*/, const ::GateServer::Varify::GetVarifyReq* /*request*/, ::GateServer::Varify::GetVarifyRsp* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedGetVarifyCode(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::GateServer::Varify::GetVarifyReq,::GateServer::Varify::GetVarifyRsp>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_GetVarifyCode<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_GetVarifyCode<Service > StreamedService;
};
// 获取验证码的方法

}  // namespace Varify
}  // namespace GateServer


#include <grpcpp/ports_undef.inc>
#endif  // GRPC_GateServer_2eVarify_2eproto__INCLUDED
