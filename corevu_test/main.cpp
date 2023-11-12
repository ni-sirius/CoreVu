#include "app.hpp"

int main(int, char**)
{
  corevutest::TestApp app{};

  try
  {
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "Hello, from CoreVu!\n";
  return EXIT_SUCCESS;
}