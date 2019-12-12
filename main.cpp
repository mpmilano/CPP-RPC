#include "invocation_target.hpp"
#include <cstring>
#include <functional>
#include <iostream>
#include <type_traits>
#include <vector>

template <int i, typename other_tuple, typename T1, typename... T2>
void exec_all(std::tuple<T1, T2...> &out, const other_tuple &in) {
  std::get<i>(out) = std::get<i>(in)();
  if constexpr (sizeof...(T2) > i)
    exec_all<i + 1>(out, in);
}

template <int> void exec_all(const std::tuple<> &, const std::tuple<> &) {}

template <typename tuple1, typename tuple2> void exec_all(tuple1 &out, const tuple2 &in) { exec_all<0>(out, in); }

using Bytes = std::unique_ptr<std::vector<char>>;
struct deserializer_t {
  template <typename... T>
  static std::enable_if_t<(std::is_pod<T>::value && ... && true), std::tuple<T *...>> deserialize(Bytes b) {
    auto buf_ptr = b->data();
    std::tuple<T *...> out;
    exec_all<std::tuple<T *...>>(out, std::make_tuple([&]() -> T * {
                                   auto ret = ((T *)buf_ptr);
                                   buf_ptr += sizeof(T);
                                   return ret;
                                 }...));
    return out;
  }
};
struct serializer_t {
  template <typename... T>
  static std::enable_if_t<(std::is_pod<T>::value && ... && true), Bytes> serialize(const T &... t) {
    std::vector<char> *v = new std::vector<char>{};
    v->resize((sizeof(t) + ... + 0));
    auto insert_ptr = v->data();

    std::vector<std::function<void()>> funcs{[&] {
      std::memcpy(insert_ptr, &t, sizeof(t));
      insert_ptr += sizeof(t);
    }...};
    for (auto f : funcs)
      f();
    return Bytes{v};
  }
};
using simpleRPC = RPCFramework<deserializer_t, serializer_t, Bytes>;
template <typename T> using RPCReceiverBuilder = typename simpleRPC::RPCReceiverBuilder<T>;

struct Test {

  void foo() {}
  void bar(int a, double b) { std::cout << "bar called with " << a << " and " << b << std::endl; }

  int baz() { return 0; }

  int bop(int i, int d) { return i + d; }

  auto register_rpc_targets(RPCReceiverBuilder<Test> &r) {
    return r.build_receiver(this, name_fun<0>(&Test::foo), name_fun<1>(&Test::bar), name_fun<2>(&Test::baz),
                            name_fun<3>(&Test::bop));
  }

  void register_rpc_invocations() {}
};

int main() {
  Test t;
  RPCReceiverBuilder<Test> r;
  auto rpc = t.register_rpc_targets(r);
  auto vec = serializer_t{}.serialize((int)12, (double)23.5);
  rpc.invoke(deserializer_t{}, serializer_t{}, vec, 1);
}