#include <array>
#include <memory>
#include <fstream>
#include <numeric>
#include <iostream>
#include <chrono>

int main() {
  const int DATA_SIZE = 10'000'000;
  std::unique_ptr<std::array<int32_t, DATA_SIZE>> data =
    std::make_unique<std::array<int32_t, DATA_SIZE>>();

  std::ifstream file("data.int32", std::ios::binary);
  if (!file) {
    std::cout << "could not open file: data.int32" << std::endl;
    return -1;
  }

  file.read(reinterpret_cast<char*>(data->data()), sizeof(int32_t) * DATA_SIZE);

  file.close();

  auto square = [](int x) { return x * x; };

  auto add = [](int x, int y) { return x + y; };

  for (int repeat = 0;  repeat < 10;  repeat++) {
    auto start = std::chrono::high_resolution_clock::now();

    int result = std::accumulate(
      data->begin(), data->end(), 0,
      [square, add](int out, int x) { return add(out, square(x)); }
    );

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = stop - start;
    std::cout << "result = " << result << " (" << duration.count() << " seconds)" << std::endl;
  }

  return 0;
}
