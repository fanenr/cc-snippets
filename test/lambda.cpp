#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unistd.h>

void
fn2 (std::string str)
{
  std::cout << str << '\n';
}

template <typename Fn, typename... Args> class lambda
{

private:
  Fn _fn;
  std::tuple<Args...> _args;

public:
  lambda (Fn fn, Args... args)
      : _fn (std::forward<Fn> (fn)), _args (std::forward<Args> (args)...)
  {
  }

  void
  operator() ()
  {
    std::apply (_fn, _args);
  }

  lambda (lambda &&src)
      : _fn (std::move (src._fn)), _args (std::move (src._args))
  {
  }
};

int
main ()
{
  std::string str ("abc");

  lambda cl (fn2, str);
}

template <typename Fn, typename... Args>
std::function<void (void)>
get_fn (Fn &&fn, Args &&...args)
{
  using ReturnType = std::invoke_result_t<Fn, Args...>;

  auto closure
      = std::bind (std::forward<Fn> (fn), std::forward<Args> (args)...);
  std::packaged_task<ReturnType ()> pack (std::move (closure));

  return lambda (pack);
}
