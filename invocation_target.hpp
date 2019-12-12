#pragma once
#include "named_member_fun.hpp"
#include <memory>
#include <tuple>
#include <type_traits>

#define TOF(arg) std::decay_t<decltype(arg)>

template <typename Deserializer, typename Serializer, typename Bytes> struct RPCFramework {
  template <typename Class> struct GenericSingleRPCReceiver {
    virtual Bytes invoke(Class &, const Deserializer &, const Serializer &, Bytes &) = 0;
    virtual ~GenericSingleRPCReceiver() {}
  };

  template <typename> struct SingleRPCReceiver;

  template <int id, typename Class, typename Ret, typename... Args>
  struct SingleRPCReceiver<named_member_fun<id, Ret (Class::*)(Args...)>> : public GenericSingleRPCReceiver<Class> {
    Ret (Class::*method)(Args...);
    SingleRPCReceiver(Ret (Class::*method)(Args...)) : method(method) {}
    Ret invoke_step1(Class &_this, const Deserializer &d, Bytes &b) const {
      return std::apply([&](Args *... a) { return (_this.*method)(*a...); },
                        d.template deserialize<Args...>(std::move(b)));
    }

    Bytes invoke(Class &_this, const Deserializer &d, const Serializer &s, Bytes &b) override {
      if constexpr (!std::is_void<Ret>::value)
        return s.serialize(invoke_step1(_this, d, b));
      else {
        invoke_step1(_this, d, b);
        return Bytes{};
      }
    }
  };

  template <typename Class, typename... NamedMethods> class RPCReceiver {
    Class *_this;

    std::array<std::unique_ptr<GenericSingleRPCReceiver<Class>>, sizeof...(NamedMethods)> invocation_targets;

  public:
    RPCReceiver(Class *c, typename NamedMethods::fun_t... f)
        : _this(c), invocation_targets{
                        std::unique_ptr<GenericSingleRPCReceiver<Class>>{new SingleRPCReceiver<NamedMethods>(f)}...} {}

    Bytes invoke(const Deserializer &deserializer, const Serializer &serializer, Bytes &serialized, int target) {
      return invocation_targets[target]->invoke(*_this, deserializer, serializer, serialized);
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
