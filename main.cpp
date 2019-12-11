#include "invocation_target.hpp"

struct deserializer_t {
  template <typename T> static T *deserialize(char *) { return nullptr; }
};
struct serializer_t {
  static char *serialize(...) { return nullptr; }
};
using Bytes = char *;
using simpleRPC = RPCFramework<deserializer_t, serializer_t, Bytes>;
template <typename T> using RPCReceiverBuilder = typename simpleRPC::RPCReceiverBuilder<T>;

struct Test {

  void foo() {}
  void bar(int, double) {}

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
  char bytes[1024] = {0};
  using charp = char*;
  charp ptr1 = bytes;
  charp& ptr2 = ptr1;
  rpc.invoke(deserializer_t{},serializer_t{},ptr2,0);
}