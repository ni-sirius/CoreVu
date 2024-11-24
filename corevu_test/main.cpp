#include "app.hpp"
#include "gravity_system_test.hpp"

#include <iostream>

template <typename App>
int run(App& app)
{
  try
  {
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "See you later! .. from CoreVu!\n";
  return EXIT_SUCCESS;
}

int main(int, char**)
{
  std::cout << "Hello user, ..  from CoreVu!\n";

  std::string in_code{0};
  std::cin >> in_code;

  if (in_code.find("ren") != std::string::npos)
  {
    corevutest::SampleApp app{};
    return run(app);
  }
  else if (in_code.find("grav") != std::string::npos)
  {
    // corevutest::GravitySystemTestApp app{};
  }
  else if (in_code.find("mem") != std::string::npos) {}

  std::cout << "No valid command\n";
  return EXIT_FAILURE;
}