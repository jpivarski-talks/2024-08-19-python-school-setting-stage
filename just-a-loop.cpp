#include <vector>
#include <fstream>
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

  int result = 0;

  for (int x : data) {
    result += x * x;
  }

  std::cout << "result = " << result << std::endl;

  return 0;
}
