#include <application.hpp>
#include <result.hpp>

int main(int argc, char **argv) {
  fb::Application *app{nullptr};

  if (auto r = fb::Initialize(app, argc, argv); r == fb::Result::Success) {
    if (r = fb::Run(app); r != fb::Result::Success)
      fb::LogErr("Execution failed with code: ", r);
    fb::Destroy(app);
  } else {
    fb::LogErr("Initialization failed with code: ", r);
    return r;
  }

  return fb::Result::Success;
}
