#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <vector>
#include <string>
#include <iostream>

// types:

// int
// list
// function
// type

// builtin functions:

// fun(args, definition)
// get(list, index)
// add(x, y)  -- x may be an int or a list
// mul(x, y)
// map(function, list)
// reduce(function, list, identity)

std::vector<std::string> tokenize(const std::string& line);

int main() {
  using_history();
  rl_bind_key('\t', rl_insert);

  char* line;
  while ((line = readline(">> ")) != nullptr) {
    if (strlen(line) > 0) {
      add_history(line);
    }

    tokenize(line);

    free(line);
  }

  return 0;
}


std::vector<std::string> tokenize(const std::string& line) {
  std::regex word_regex("([\\w0-9]+|\\(|\\)|,)");
  auto words_begin = std::sregex_iterator(line.begin(), line.end(), word_regex);
  auto words_end = std::sregex_iterator();

  std::vector<std::string> out;
  for (auto word_iter = words_begin;  word_iter != words_end;  ++word_iter) {
    out.push_back(word_iter->str());
  }
  return out;
}
