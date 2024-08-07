#include <fstream>
#include <iostream>
#include <chrono>

int main() {
  const int DATA_SIZE = 10'000'000;
  int32_t* data = new int32_t[DATA_SIZE];

  std::ifstream file("data.int32", std::ios::binary);
  if (!file) {
    std::cout << "could not open file: data.int32" << std::endl;
    return -1;
  }

  file.read(reinterpret_cast<char*>(data), sizeof(int32_t) * DATA_SIZE);

  file.close();

  for (int repeat = 0;  repeat < 10;  repeat++) {
    auto start = std::chrono::high_resolution_clock::now();

    int result = 0;

    for (int i = 0;  i < DATA_SIZE;  i++) {
      result += data[i] * data[i];
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = stop - start;
    std::cout << "result = " << result << " (" << duration.count() << " seconds)" << std::endl;
  }

  delete [] data;

  return 0;
}
