#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

// types:

// int
// list
// function
// type

// builtin functions:

// [x, y]
// x = ...
// fun(args, definition)
// get(list, index)
// add(x, y)
// mul(x, y)
// map(function, list)
// reduce(function, list, identity)

typedef std::pair<int, std::string> Token;

class ASTNode {
public:
  ASTNode(int pos): pos_(pos) { }
  virtual ~ASTNode() = default;

  int pos() const { return pos_; }

  virtual void debug() = 0;

private:
  int pos_;
};

class ASTLiteralInt: public ASTNode {
public:
  ASTLiteralInt(int pos, int value): value_(value), ASTNode(pos) { }

  int value() const { return value_; }

  void debug() override {
    std::cout << "ASTLiteralInt(" << pos() << ", " << value() << ")";
  }

private:
  int value_;
};

std::vector<Token> tokenize(const std::string& line);

std::string error_arrow(int position);

std::unique_ptr<ASTNode> parse(int i, const std::vector<Token>& tokens);


int main() {
  using_history();
  rl_bind_key('\t', rl_insert);

  char* line;
  while ((line = readline(">> ")) != nullptr) {
    if (strlen(line) > 0) {
      add_history(line);
    }

    std::unique_ptr<ASTNode> ast = parse(0, tokenize(line));

    ast->debug();
    std::cout << std::endl;

    free(line);
  }

  return 0;
}


std::vector<Token> tokenize(const std::string& line) {
  std::regex token_regex("([0-9]+|[A-Za-z_][A-Za-z_0-9]*|\\(|\\)|\\[|\\]|,|=)");
  auto tokens_begin = std::sregex_iterator(line.begin(), line.end(), token_regex);
  auto tokens_end = std::sregex_iterator();

  std::vector<Token> out;
  for (auto token_iter = tokens_begin;  token_iter != tokens_end;  ++token_iter) {
    std::cout << "token: {" << token_iter->str() << "}" << std::endl;
    out.push_back(Token(token_iter->position(), token_iter->str()));
  }
  return out;
}


std::string error_arrow(int position) {
  return std::string(position, '-') + std::string("^\n");
}


std::unique_ptr<ASTNode> parse(int i, const std::vector<Token>& tokens) {
  int as_integer;
  try {
    as_integer = std::stoi(tokens[i].second);
  }
  catch (std::invalid_argument const& ex) {
    return std::make_unique<ASTLiteralInt>(tokens[i].first, as_integer);
  }

  throw std::runtime_error(error_arrow(tokens[i].first) + std::string("invalid syntax"));
}
