#include "app.hpp"

int main(int, char**)
{
  std::cout << "Hello user, ..  from CoreVu!\n";
  corevutest::SampleApp app{};

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