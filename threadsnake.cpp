#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iostream>

// types:

// int
// list
// function
// type

// builtin functions:

// [x, y]
// fun(args) { stmt ; stmt ; stmt }
// del(name)
// get(list, index)
// add(x, y)  --  x and y can both be int or both be lists
// mul(x, y)
// map(function, list)
// reduce(function, list, identity)
// x = ...


//// types /////////////////////////////////////////////////////////////////


const int MAX_REPR = 80;


class Object;
class ASTNode;
class ASTDefineFun;


class Scope {
public:
  Scope(std::shared_ptr<Scope> parent): parent_(parent), objects_() { }

  void assign(int pos, const std::string& name, std::shared_ptr<Object> object);
  void del(int pos, const std::string& name);
  std::shared_ptr<Object> get(int pos, const std::string& name);

private:
  std::shared_ptr<Scope> parent_;
  std::unordered_map<std::string, std::shared_ptr<Object>> objects_;
};


class Object {
public:
  Object() { }

  virtual std::string repr(int& remaining) const = 0;

private:
};


class ObjectInt: public Object {
public:
  ObjectInt(int value): value_(value), Object() { }

  int value() const { return value_; }

  std::string repr(int& remaining) const override;

private:
  int value_;
};


class ObjectFunction: public Object {
public:
  ObjectFunction(): Object() { }

  virtual std::shared_ptr<Object> run(
    int pos, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Object>> args
  ) = 0;

private:
};


class ObjectFunctionAdd: public ObjectFunction {
public:
  ObjectFunctionAdd(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    int pos, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectUserFunction: public ObjectFunction {
public:
  ObjectUserFunction(std::shared_ptr<ASTDefineFun> fun): fun_(fun), ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    int pos, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Object>> args
  ) override;

private:
  std::shared_ptr<ASTDefineFun> fun_;
};


class ASTNode {
public:
  ASTNode(int pos): pos_(pos) { }
  virtual ~ASTNode() = default;

  int pos() const { return pos_; }

  virtual std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) = 0;

  virtual void debug() = 0;

private:
  int pos_;
};

class ASTLiteralInt: public ASTNode {
public:
  ASTLiteralInt(int pos, int value): value_(value), ASTNode(pos) { }

  int value() const { return value_; }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTLiteralInt(" << pos() << ", " << value_ << ")";
  }

private:
  int value_;
};


class ASTLiteralList: public ASTNode {
public:
  ASTLiteralList(int pos, std::vector<std::shared_ptr<ASTNode>> content)
    : content_(content)
    , ASTNode(pos) { }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

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
  std::vector<std::shared_ptr<ASTNode>> content_;
};


class ASTDefineFun: public ASTNode {
public:
  ASTDefineFun(int pos, const std::vector<std::string>& params, std::vector<std::shared_ptr<ASTNode>> body)
    : params_(params)
    , body_(body)
    , ASTNode(pos) { }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTDefineFun(" << pos() << ", {";
    for (int i = 0;  i < params_.size();  i++) {
      std::cout << "\"" << params_[i] << "\"";
      if (i + 1 != params_.size()) {
        std::cout << ", ";
      }
    }
    std::cout << "}, {";
    for (int i = 0;  i < body_.size();  i++) {
      body_[i]->debug();
      if (i + 1 != body_.size()) {
        std::cout << ", ";
      }
    }
    std::cout << "})";
  }

private:
  const std::vector<std::string> params_;
  std::vector<std::shared_ptr<ASTNode>> body_;
};


class ASTCallNamed: public ASTNode {
public:
  ASTCallNamed(int pos, const std::string& name, std::vector<std::shared_ptr<ASTNode>> args)
    : name_(name)
    , args_(args)
    , ASTNode(pos) { }

  const std::string name() const { return name_; }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTCallNamed(" << pos() << ", \"" << name_ << "\", {";
    for (int i = 0;  i < args_.size();  i++) {
      args_[i]->debug();
      if (i + 1 != args_.size()) {
        std::cout << ", ";
      }
    }
    std::cout << "})";
  }

private:
  const std::string name_;
  std::vector<std::shared_ptr<ASTNode>> args_;
};


class ASTAssignment: public ASTNode {
public:
  ASTAssignment(int pos, const std::string& name, std::shared_ptr<ASTNode> value)
    : name_(name)
    , value_(value)
    , ASTNode(pos) { }

  const std::string name() const { return name_; }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTAssignment(" << pos() << ", \"" << name_ << "\", ";
    value_->debug();
    std::cout << ")";
  }

private:
  const std::string name_;
  std::shared_ptr<ASTNode> value_;
};


class ASTDelete: public ASTNode {
public:
  ASTDelete(int pos, const std::string& name): name_(name), ASTNode(pos) { }

  const std::string name() const { return name_; }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTDelete(" << pos() << ", \"" << name_ << "\")";
  }

private:
  const std::string name_;
};


class ASTIdentifier: public ASTNode {
public:
  ASTIdentifier(int pos, const std::string& name): name_(name), ASTNode(pos) { }

  const std::string name() const { return name_; }

  std::shared_ptr<Object> run(std::shared_ptr<Scope> scope) override;

  void debug() override {
    std::cout << "ASTIdentifier(" << pos() << ", \"" << name_ << "\")";
  }

private:
  const std::string name_;
};


typedef std::pair<int, std::string> PosToken;


std::string error_arrow(int position);
std::runtime_error error(int position, const std::string& message);
std::vector<PosToken> tokenize(const std::string& line);
std::shared_ptr<ASTNode> parse(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_list(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_fun(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_call(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_assign(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_delete(int& i, const std::vector<PosToken>& tokens);
std::shared_ptr<ASTNode> parse_id(int& i, const std::vector<PosToken>& tokens);


//// main function /////////////////////////////////////////////////////////


int main() {
  using_history();
  rl_bind_key('\t', rl_insert);

  std::shared_ptr<Scope> scope = std::make_shared<Scope>(nullptr);

  scope->assign(0, "add", std::make_shared<ObjectFunctionAdd>());

  char* line;
  while ((line = readline(">> ")) != nullptr) {
    if (strlen(line) > 0) {
      add_history(line);
    }

    std::vector<PosToken> tokens;
    int i = 0;
    std::shared_ptr<ASTNode> ast;
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
        std::shared_ptr<Object> result(nullptr);
        try {
          result = ast->run(scope);
        }
        catch (std::runtime_error const& exception) {
          // failure: could not execute the code for some reason
          std::cout << exception.what() << std::endl;
        }

        if (result) {
          // success: print the result's repr

          int remaining = MAX_REPR;
          std::string repr = result->repr(remaining);
          if (repr.size() > MAX_REPR) {
            repr = repr.substr(0, MAX_REPR - 3) + "...";
          }
          std::cout << repr << std::endl;
        }

      }
    }

    free(line);
  }

  return 0;
}


//// parsing ///////////////////////////////////////////////////////////////


std::string error_arrow(int position) {
  return std::string("---") + std::string(position, '-') + std::string("^\n");
}


std::runtime_error error(int position, const std::string& message) {
  return std::runtime_error(error_arrow(position) + message);
}


std::vector<PosToken> tokenize(const std::string& line) {
  std::regex whitespace("\\s*");
  std::regex token_regex("(-?[0-9]+|[A-Za-z_][A-Za-z_0-9]*|\\(|\\)|\\[|\\]|,|;|\\{|\\}|=)");
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


std::shared_ptr<ASTNode> parse(int& i, const std::vector<PosToken>& tokens) {
  if (i >= tokens.size()) {
    throw error(0, "line ends without complete expression");
  }

  std::regex is_number("-?[0-9]+");
  std::regex is_name("[A-Za-z_][A-Za-z_0-9]*");

  if (tokens[i].second == "[") {
    return parse_list(i, tokens);
  }

  else if (tokens[i].second == "fun") {
    return parse_fun(i, tokens);
  }

  else if (tokens[i].second == "del") {
    return parse_delete(i, tokens);
  }

  else if (std::regex_match(tokens[i].second, is_number)) {
    return parse_int(i, tokens);
  }

  else if (std::regex_match(tokens[i].second, is_name)) {
    if (i + 1 < tokens.size()  &&  tokens[i + 1].second == "=") {
      return parse_assign(i, tokens);
    }

    else if (i + 1 < tokens.size()  &&  tokens[i + 1].second == "(") {
      return parse_call(i, tokens);
    }

    else {
      return parse_id(i, tokens);
    }
  }

  else {
    throw error(tokens[i].first, "unrecognized syntax");
  }
}


std::shared_ptr<ASTNode> parse_int(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  int value = std::stoi(tokens[i].second);

  i++;  // get past int

  return std::make_shared<ASTLiteralInt>(pos, value);
}


std::shared_ptr<ASTNode> parse_list(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;

  i++;   // get past "["

  std::vector<std::shared_ptr<ASTNode>> content;

  bool first = true;
  while (tokens[i].second != "]") {
    if (!first) {
      if (tokens[i].second != ",") {
        throw error(tokens[i].first, "commas are required between list items");
      }
      i++;  // get past ","
    }
    first = false;

    content.push_back(parse(i, tokens));
  }

  i++;   // get past "]"

  return std::make_shared<ASTLiteralList>(pos, content);
}


std::shared_ptr<ASTNode> parse_fun(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;

  i++;  // get past "fun"

  if (tokens[i].second != "(") {
    throw error(tokens[i].first, "'fun' must be followed by a list of function parameters");
  }

  i++;  // get past "("

  std::regex is_name("[A-Za-z_][A-Za-z_0-9]*");
  std::vector<std::string> params;

  bool first = true;
  while (tokens[i].second != ")") {
    if (!first) {
      if (tokens[i].second != ",") {
        throw error(tokens[i].first, "commas are required between function parameter names");
      }
      i++;  // get past ","
    }
    first = false;

    if (std::regex_match(tokens[i].second, is_name)) {
      params.push_back(tokens[i].second);
    }
    else {
      throw error(tokens[i].first, "function parameters must be identifiers");
    }
    i++;  // get past parameter name
  }

  i++;  // get past ")"

  std::vector<std::shared_ptr<ASTNode>> body;

  if (tokens[i].second == "{") {
    // curly brackets; accept statements separated by semicolons

    i++;  // get past "{"

    bool first = true;
    while (tokens[i].second != "}") {
      if (!first) {
        if (tokens[i].second != ";") {
          throw error(tokens[i].first, "semicolons are required between statements");
        }
        i++;  // get past ";"
      }
      first = false;

      body.push_back(parse(i, tokens));
    }

    i++;   // get past "}"
  }
  else {
    // no curly brackets; only one statement
    body.push_back(parse(i, tokens));
  }

  return std::make_shared<ASTDefineFun>(pos, params, body);
}


std::shared_ptr<ASTNode> parse_call(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name
  i++;  // get past "("

  std::vector<std::shared_ptr<ASTNode>> args;

  bool first = true;
  while (tokens[i].second != ")") {
    if (!first) {
      if (tokens[i].second != ",") {
        throw error(tokens[i].first, "commas are required between list items");
      }
      i++;  // get past ","
    }
    first = false;

    args.push_back(parse(i, tokens));
  }

  i++;   // get past ")"

  return std::make_shared<ASTCallNamed>(pos, name, args);
}


std::shared_ptr<ASTNode> parse_assign(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name
  i++;  // get past "="

  std::shared_ptr<ASTNode> value = parse(i, tokens);

  return std::make_shared<ASTAssignment>(pos, name, value);
}


std::shared_ptr<ASTNode> parse_delete(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;

  i++;  // get past "del"

  if (i >= tokens.size()  ||  tokens[i].second != "(") {
    throw error(pos, "'del' must be followed by a name in parentheses");
  }

  i++;  // get past "("

  std::regex is_name("[A-Za-z_][A-Za-z_0-9]*");

  std::string name;
  if (i < tokens.size()  &&  std::regex_match(tokens[i].second, is_name)) {
    name = tokens[i].second;
  }
  else {
    throw error(pos, "name of variable to delete must be provided");
  }

  i++;  // get past name

  if (i >= tokens.size()  ||  tokens[i].second != ")") {
    throw error(pos, "parentheses must be closed after name of variable to delete");
  }

  i++;  // get past ")"

  return std::make_shared<ASTDelete>(pos, name);
}


std::shared_ptr<ASTNode> parse_id(int& i, const std::vector<PosToken>& tokens) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name

  return std::make_shared<ASTIdentifier>(pos, name);
}

//// data //////////////////////////////////////////////////////////////////


void Scope::assign(int pos, const std::string& name, std::shared_ptr<Object> object) {
  objects_[name] = object;
}


void Scope::del(int pos, const std::string& name) {
  objects_.erase(name);
}


std::shared_ptr<Object> Scope::get(int pos, const std::string& name) {
  if (objects_.count(name)) {
    return objects_[name];
  }
  else {
    throw error(pos, "there is no variable named '" + name + "'");
  }
}


std::string ObjectInt::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  std::string out = std::to_string(value_);

  remaining -= out.size();

  return out;
}


std::string ObjectFunctionAdd::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 24;

  return "<builtin function 'add'>";
}


std::shared_ptr<Object> ObjectFunctionAdd::run(
  int pos, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 2) {
    throw error(pos, "'add' function takes exactly 2 arguments");
  }

  std::shared_ptr<ObjectInt> arg0_int = std::dynamic_pointer_cast<ObjectInt>(args[0]);
  std::shared_ptr<ObjectInt> arg1_int = std::dynamic_pointer_cast<ObjectInt>(args[1]);

  if (arg0_int  &&  arg1_int) {
    return std::make_shared<ObjectInt>(arg0_int->value() + arg1_int->value());
  }

  else {
    throw error(pos, "'add' function's arguments must both be integers");
  }
}


std::string ObjectUserFunction::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 23;

  return "<user-defined function>";
}


std::shared_ptr<Object> ObjectUserFunction::run(
  int pos, std::shared_ptr<Scope> scope, std::vector<std::shared_ptr<Object>> args
) {

}


std::shared_ptr<Object> ASTLiteralInt::run(std::shared_ptr<Scope> scope) {
  return std::make_shared<ObjectInt>(value_);
}


std::shared_ptr<Object> ASTLiteralList::run(std::shared_ptr<Scope> scope) {
  return std::make_shared<ObjectInt>(123);
}


std::shared_ptr<Object> ASTDefineFun::run(std::shared_ptr<Scope> scope) {
  return std::make_shared<ObjectInt>(123);
}


std::shared_ptr<Object> ASTCallNamed::run(std::shared_ptr<Scope> scope) {
  std::shared_ptr<Object> maybe_fun = scope->get(pos(), name_);

  std::shared_ptr<ObjectFunction> fun = std::dynamic_pointer_cast<ObjectFunction>(maybe_fun);

  if (!fun) {
    throw error(pos(), "attempting to call an argument that is not a function");
  }

  std::vector<std::shared_ptr<Object>> args;
  for (int i = 0;  i < args_.size();  i++) {
    args.push_back(args_[i]->run(scope));
  }

  return fun->run(pos(), scope, args);
}


std::shared_ptr<Object> ASTAssignment::run(std::shared_ptr<Scope> scope) {
  std::shared_ptr<Object> result = value_->run(scope);

  scope->assign(pos(), name_, result);

  return result;
}


std::shared_ptr<Object> ASTDelete::run(std::shared_ptr<Scope> scope) {
  return std::make_shared<ObjectInt>(123);
}


std::shared_ptr<Object> ASTIdentifier::run(std::shared_ptr<Scope> scope) {
  return scope->get(pos(), name_);
}
