#include "invocation_target.hpp"

struct deserializer_t {
  template <typename T> static T *deserialize(char *) { return nullptr; }
};
struct serializer_t {
  static char *serialize(...) { return nullptr; }
};
using bytes = char *;
using simpleRPC = RPCFramework<deserializer_t, serializer_t, bytes>;
template <typename T> using RPCReceiverBuilder = typename simpleRPC::RPCReceiverBuilder<T>;

class Test {

  void foo() {}
  void bar(int, double) {}

  int baz() { return 0; }

  int bop(int i, int d) { return i + d; }

  void register_rpc_targets(RPCReceiverBuilder<Test> &r) {
    r.build_receiver(this, name_fun<0>(&Test::foo), name_fun<1>(&Test::bar), name_fun<2>(&Test::baz),
                     name_fun<3>(&Test::bop));
  }

  void register_rpc_invocations() {}
};

int main() {}