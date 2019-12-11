#include "invocation_target.hpp"

class Test {

  void foo() {}
  void bar(int, double) {}

  int baz() { return 0; }

  int bop(int i, int d) { return i + d; }

  void register_rpc_targets(RPCReceiverBuilder<Test> &r) {
    r.build_receiver(name_fun<0>(&Test::foo), name_fun<1>(&Test::bar), name_fun<2>(&Test::baz),
                     name_fun<3>(&Test::bop));
  }

  void register_rpc_invocations() {}
};

int main() {}