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
// fun(args, definition)
// get(list, index)
// add(x, y)
// mul(x, y)
// map(function, list)
// reduce(function, list, identity)
// x = ...

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


class ASTAssignment: public ASTNode {
public:
  ASTAssignment(int pos, const std::string& name, std::unique_ptr<ASTNode> value)
    : name_(name)
    , value_(std::move(value))
    , ASTNode(pos) { }

  const std::string name() const { return name_; }

  void debug() override {
    std::cout << "ASTAssignment(" << pos() << ", " << name_ << ", ";
    value_->debug();
    std::cout << ")";
  }

private:
  const std::string name_;
  std::unique_ptr<ASTNode> value_;
};


class ASTIdentifier: public ASTNode {
public:
  ASTIdentifier(int pos, const std::string& name): name_(name), ASTNode(pos) { }

  const std::string name() const { return name_; }

  void debug() override {
    std::cout << "ASTIdentifier(" << pos() << ", " << name_ << ")";
  }

private:
  const std::string name_;
};


typedef std::pair<int, std::string> PosToken;


std::string error_arrow(int position);
std::runtime_error error(int position, const std::string& message);
std::vector<PosToken> tokenize(const std::string& line);
std::unique_ptr<ASTNode> parse(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_list(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_assign(int& i, const std::vector<PosToken>& tokens);
std::unique_ptr<ASTNode> parse_id(int& i, const std::vector<PosToken>& tokens);


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

  std::regex is_number("-?[0-9]+");
  std::regex is_name("[A-Za-z_][A-Za-z_0-9]*");

  if (tokens[i].second == "[") {
    return parse_list(i, tokens);
  }

  else if (tokens[i].second == "fun") {
    throw error(tokens[i].first, "'fun' not implemented");
  }

  else if (tokens[i].second == "get") {
    throw error(tokens[i].first, "'get' not implemented");
  }

  else if (tokens[i].second == "add") {
    throw error(tokens[i].first, "'add' not implemented");
  }

  else if (tokens[i].second == "mul") {
    throw error(tokens[i].first, "'mul' not implemented");
  }

  else if (tokens[i].second == "map") {
    throw error(tokens[i].first, "'map' not implemented");
  }

  else if (tokens[i].second == "reduce") {
    throw error(tokens[i].first, "'reduce' not implemented");
  }

  else if (std::regex_match(tokens[i].second, is_number)) {
    return parse_int(i, tokens);
  }

  else if (std::regex_match(tokens[i].second, is_name)) {
    if (i + 1 < tokens.size()  &&  tokens[i + 1].second == "=") {
      return parse_assign(i, tokens);
    }
    else {
      return parse_id(i, tokens);
    }
  }

  else {
    throw error(tokens[i].first, "unrecognized syntax");
  }
}


std::unique_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  int value = std::stoi(tokens[i].second);

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


std::unique_ptr<ASTNode> parse_assign(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name
  i++;  // get past "="

  std::unique_ptr<ASTNode> value = parse(i, tokens);

  return std::make_unique<ASTAssignment>(pos, name, std::move(value));
}


std::unique_ptr<ASTNode> parse_id(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;

  return std::make_unique<ASTIdentifier>(pos, name);
}
