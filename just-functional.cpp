#include <vector>
#include <fstream>
#include <numeric>
#include <iostream>

int main() {
  std::ifstream file("data.int32", std::ios::binary);
  if (!file) {
    std::cout << "could not open file: data.int32" << std::endl;
    return -1;
  }

  std::vector<int> data;
  int32_t raw;
  while (file.read(reinterpret_cast<char*>(&raw), sizeof(raw))) {
    data.push_back(raw);
  }

  file.close();

  auto square = [](int x) { return x * x; };

  auto add = [](int x, int y) { return x + y; };

  int result = std::accumulate(
    data.begin(), data.end(), 0,
    [square, add](int out, int x) { return add(out, square(x)); }
  );

  std::cout << "result = " << result << std::endl;

  return 0;
}
