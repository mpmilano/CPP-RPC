#pragma once

template <typename Ret, typename T, typename... Args> using member_fun_t = Ret (T ::*)(Args...);

template <int id, typename> struct named_member_fun;

template <int id, typename Ret, typename T, typename... Args> struct named_member_fun<id, Ret (T::*)(Args...)> {
  member_fun_t<Ret, T, Args...> f;
  using fun_t = member_fun_t<Ret, T, Args...>;
};

template <int id, typename Ret, typename T, typename... Args>
named_member_fun<id, member_fun_t<Ret, T, Args...>> name_fun(member_fun_t<Ret, T, Args...> f) {
  return named_member_fun<id, member_fun_t<Ret, T, Args...>>{f};
}

template <typename T, int id, typename Ret, typename... Args>
static named_member_fun<id, member_fun_t<Ret, T, Args...>>
ensure_named_member_fun(named_member_fun<id, member_fun_t<Ret, T, Args...>> f) {
  return f;
}
