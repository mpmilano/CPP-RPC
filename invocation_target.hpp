#pragma once
#include "named_member_fun.hpp"
#include <memory>
#include <tuple>
#include <type_traits>

#define TOF(arg) std::decay_t<decltype(arg)>

template <typename Deserializer, typename Serializer, typename Bytes> struct RPCFramework {
  struct GenericSingleRPCReceiver {
    virtual Bytes invoke(const Deserializer &, const Serializer &, Bytes &) = 0;
    virtual ~GenericSingleRPCReceiver() {}
  };

  template <typename> struct SingleRPCReceiver;
  template <int id, typename MemberFun>
  struct SingleRPCReceiver<named_member_fun<id, MemberFun>> : public GenericSingleRPCReceiver {
    SingleRPCReceiver(MemberFun) {}
    Bytes invoke(const Deserializer &, const Serializer &, Bytes &) override { return nullptr; }
  };

  template <typename Class, typename... NamedMethods> class RPCReceiver {
    std::unique_ptr<Class> _this;

    std::array<std::unique_ptr<GenericSingleRPCReceiver>, sizeof...(NamedMethods)> invocation_targets;

  public:
    RPCReceiver(Class *c, typename NamedMethods::fun_t... f)
        : _this(c), invocation_targets{
                             std::unique_ptr<GenericSingleRPCReceiver>{new SingleRPCReceiver<NamedMethods>(f)} ...} {}

    Bytes invoke(const Deserializer &deserializer, const Serializer &serializer, Bytes &serialized, int target) {
      return invocation_targets[target].invoke(deserializer, serializer, serialized);
    }
  };

  template <typename Class, typename... Methods>
  static RPCReceiver<Class, Methods...> make_rpc_receiver(Class *c, Methods... m) {
    return RPCReceiver<Class, Methods...>{c, m.f...};
  }

  template <typename T> class RPCReceiverBuilder {

  public:
    template <typename... functions>
    auto build_receiver(T *t, const functions &... f) -> RPCReceiver<T, TOF(ensure_named_member_fun(f))...> {
      return make_rpc_receiver(t, f...);
    }
  };
};
