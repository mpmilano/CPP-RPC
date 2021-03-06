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

  template <typename Namespace, Namespace id, typename Class, typename Ret, typename... Args>
  struct SingleRPCReceiver<named_member_fun<Namespace, id, Ret (Class::*)(Args...)>>
      : public GenericSingleRPCReceiver<Class> {
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

    Bytes invoke(const Deserializer &deserializer, const Serializer &serializer, Bytes &serialized,
                 typename Class::rpc target) {
      return invocation_targets[(int)target]->invoke(*_this, deserializer, serializer, serialized);
    }
  };

  template <typename> struct SingleInvoker;
  template <typename Namespace, Namespace id, typename Class, typename Ret, typename... Args>
  struct SingleInvoker<named_member_fun<Namespace, id, Ret (Class::*)(Args...)>> {
    std::pair<Namespace, Bytes> build_invocation(const Serializer &s, const Args &... a) const {
      return std::pair<Namespace, Bytes>{id, s.serialize(a...)};
    }

    const SingleInvoker &get_invoker(std::integral_constant<Namespace, id>) const { return *this; }
  };

  template <typename Class, typename... NamedMethods> class RPCInvoker : public SingleInvoker<NamedMethods>... {

  public:
    using invocation_pair = std::pair<typename Class::rpc, Bytes>;
    using SingleInvoker<NamedMethods>::build_invocation...;
    using SingleInvoker<NamedMethods>::get_invoker...;

    template <typename Class::rpc i, typename... T>
    invocation_pair build_invocation(const Serializer &s, T &&... t) const {
      return this->template get_invoker(std::integral_constant<typename Class::rpc, i>{})
          .build_invocation(s, std::forward<T>(t)...);
    }
  };

  template <typename Class, typename... Methods>
  static RPCReceiver<Class, Methods...> make_rpc_receiver(Class *c, Methods... m) {
    return RPCReceiver<Class, Methods...>{c, m.f...};
  }

  template <typename T> class Builder {

  public:
    template <typename... functions>
    static auto build_receiver(T *t, const functions &... f) -> RPCReceiver<T, TOF(ensure_named_member_fun(f))...> {
      return make_rpc_receiver(t, f...);
    }

    template <typename... functions>
    static auto build_invoker(T *, const functions &... f) -> RPCInvoker<T, TOF(ensure_named_member_fun(f))...> {
      return RPCInvoker<T, TOF(ensure_named_member_fun(f))...>{};
    }
  };
};
