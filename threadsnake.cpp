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
    std::cout << "ASTLiteralInt(" << pos() << ", " << value_ << ")";
  }

private:
  int value_;
};


class ASTLiteralList: public ASTNode {
public:
  ASTLiteralList(int pos, std::vector<std::unique_ptr<ASTNode>> content)
    : content_(std::move(content))
    , ASTNode(pos) { }

  void debug() override {
    std::cout << "ASTLiteralList(" << pos() << ", {";
    for (int i = 0;  i < content_.size();  i++) {
      content_[i]->debug();
      if (i + 1 != content_.size()) {
        std::cout << ", ";
      }
    }
    std::cout << "})";
  }

private:
  std::vector<std::unique_ptr<ASTNode>> content_;
};


typedef std::pair<int, std::string> PosToken;


std::string error_arrow(int position);
std::runtime_error error(int position, const std::string& message);
std::vector<PosToken> tokenize(const std::string& line);
std::unique_ptr<ASTNode> parse(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_list(int& i, const std::vector<PosToken>& tokens);


int main() {
  using_history();
  rl_bind_key('\t', rl_insert);

  char* line;
  while ((line = readline(">> ")) != nullptr) {
    if (strlen(line) > 0) {
      add_history(line);
    }

    std::vector<PosToken> tokens;
    int i = 0;
    std::unique_ptr<ASTNode> ast;
    try {
      tokens = tokenize(line);
      ast = parse(i, tokens);
    }
    catch (std::runtime_error const& exception) {
      // failure: syntax error while tokenizing or building AST
      std::cout << exception.what() << std::endl;
    }

    if (ast) {
      if (i < tokens.size()) {
        // failure: more input after complete AST
        std::cout << error_arrow(tokens[i].first);
        std::cout << "complete expression, but line doesn't end" << std::endl;
      }
      else {
        // success!

        ast->debug();
        std::cout << std::endl;
      }
    }

    free(line);
  }

  return 0;
}


std::string error_arrow(int position) {
  return std::string("---") + std::string(position, '-') + std::string("^\n");
}


std::runtime_error error(int position, const std::string& message) {
  return std::runtime_error(error_arrow(position) + message);
}


std::vector<PosToken> tokenize(const std::string& line) {
  std::regex whitespace("\\s*");
  std::regex token_regex("(-?[0-9]+|[A-Za-z_][A-Za-z_0-9]*|\\(|\\)|\\[|\\]|,|=)");
  auto tokens_begin = std::sregex_iterator(line.begin(), line.end(), token_regex);
  auto tokens_end = std::sregex_iterator();

  int previous = 0;
  std::vector<PosToken> out;

  for (auto token_iter = tokens_begin;  token_iter != tokens_end;  ++token_iter) {
    // text between tokens must be only whitespace
    std::string between = line.substr(previous, token_iter->position() - previous);
    if (!std::regex_match(between, whitespace)) {
      throw error(previous, "unexpected characters");
    }
    previous = token_iter->position() + token_iter->length();

    // collect tokens
    out.push_back(PosToken(token_iter->position(), token_iter->str()));
  }
  return out;
}


std::unique_ptr<ASTNode> parse(int& i, const std::vector<PosToken>& tokens) {
  if (i >= tokens.size()) {
    throw error(0, "line ends without complete expression");
  }

  std::unique_ptr<ASTNode> as_integer = parse_int(i, tokens);

  if (as_integer) {
    return as_integer;
  }

  else if (tokens[i].second == "[") {
    return parse_list(i, tokens);
  }

  else {
    throw error(tokens[i].first, "unrecognized syntax");
  }
}


std::unique_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;

  int value;
  try {
    value = std::stoi(tokens[i].second);
  }
  catch (std::invalid_argument const& exception) {
    return nullptr;
  }

  i++;
  return std::make_unique<ASTLiteralInt>(pos, value);
}


std::unique_ptr<ASTNode> parse_list(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;

  std::vector<std::unique_ptr<ASTNode>> content;

  i++;   // get past "["

  bool first = true;
  while (tokens[i].second != "]") {
    if (!first) {
      if (tokens[i].second != ",") {
        throw error(tokens[i].first, "commas are required between list items");
      }
      i++;
    }
    first = false;

    content.push_back(parse(i, tokens));
  }

  i++;   // get past "]"

  return std::make_unique<ASTLiteralList>(pos, std::move(content));
}
