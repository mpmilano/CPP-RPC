#pragma once
#include "named_member_fun.hpp"
#include <tuple>
#include <type_traits>

#define TOF(arg) std::decay_t<decltype(arg)>

template <typename> struct SingleRPCReceiver;
template <int id, typename MemberFun> struct SingleRPCReceiver<named_member_fun<id, MemberFun>> {
  SingleRPCReceiver(MemberFun) {}
};

template <typename Class, typename... NamedMethods> class RPCReceiver : public SingleRPCReceiver<NamedMethods>... {
public:
  RPCReceiver(typename NamedMethods::fun_t... f) : SingleRPCReceiver<NamedMethods>(f)... {}
};

template <typename Class, typename... Methods> RPCReceiver<Class, Methods...> make_rpc_receiver(Methods... m) {
  return RPCReceiver<Class, Methods...>{m.f...};
}

template <typename T> class RPCReceiverBuilder {

public:
  template <typename... functions>
  auto build_receiver(const functions &... f) -> RPCReceiver<T, TOF(ensure_named_member_fun(f))...> {
    return make_rpc_receiver<T>(f...);
  }
};
