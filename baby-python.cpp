// Compile with:
//
//     c++ -std=c++11 -O3 baby-python.cpp -o baby-python

//// includes //////////////////////////////////////////////////////////////

#include "linenoise.hpp"
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <iostream>


//// types /////////////////////////////////////////////////////////////////


const int MAX_REPR = 80;
const int MAX_RECURSION = 20;

class Object;
class ASTNode;
class ASTDefineFun;


//// Scope: which variables exist right now?

class Scope {
public:
  Scope(std::shared_ptr<Scope> parent): parent_(parent), objects_() { }

  void assign(
    const std::string& name,
    std::shared_ptr<Object> object,
    std::vector<std::shared_ptr<ASTNode>>& stack
  );
  std::shared_ptr<Object> del(
    const std::string& name,
    std::vector<std::shared_ptr<ASTNode>>& stack
  );
  std::shared_ptr<Object> get(
    const std::string& name,
    std::vector<std::shared_ptr<ASTNode>>& stack
  );

private:
  std::shared_ptr<Scope> parent_;
  std::unordered_map<std::string, std::shared_ptr<Object>> objects_;
};


//// Objects: data within the language

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


class ObjectList: public Object {
public:
  ObjectList(const std::vector<std::shared_ptr<Object>>& values): values_(values), Object() { }

  const std::vector<std::shared_ptr<Object>>& values() const { return values_; }

  std::string repr(int& remaining) const override;

private:
  const std::vector<std::shared_ptr<Object>> values_;
};


class ObjectFunction: public Object {
public:
  ObjectFunction(): Object() { }

  virtual std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) = 0;

private:
};


class ObjectFunctionAdd: public ObjectFunction {
public:
  ObjectFunctionAdd(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectFunctionMul: public ObjectFunction {
public:
  ObjectFunctionMul(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectFunctionGet: public ObjectFunction {
public:
  ObjectFunctionGet(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectFunctionLen: public ObjectFunction {
public:
  ObjectFunctionLen(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectFunctionMap: public ObjectFunction {
public:
  ObjectFunctionMap(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectFunctionReduce: public ObjectFunction {
public:
  ObjectFunctionReduce(): ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
};


class ObjectUserFunction: public ObjectFunction {
public:
  ObjectUserFunction(std::shared_ptr<ASTDefineFun> fun): fun_(fun), ObjectFunction() { }

  std::string repr(int& remaining) const override;

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack,
    std::vector<std::shared_ptr<Object>> args
  ) override;

private:
  std::shared_ptr<ASTDefineFun> fun_;
};


//// ASTNodes: sequence of instructions (as a tree) to run


class ASTNode {
public:
  ASTNode(int pos, const std::string& line): pos_(pos), line_(line) { }
  virtual ~ASTNode() = default;

  int pos() const { return pos_; }
  const std::string& line() const { return line_; }

  virtual std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) = 0;

private:
  int pos_;
  const std::string line_;
};

class ASTLiteralInt: public ASTNode {
public:
  ASTLiteralInt(int pos, const std::string& line, int value)
    : value_(value), ASTNode(pos, line) { }

  int value() const { return value_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  int value_;
};


class ASTLiteralList: public ASTNode {
public:
  ASTLiteralList(
    int pos,
    const std::string& line,
    std::vector<std::shared_ptr<ASTNode>>& values
  )
    : values_(values)
    , ASTNode(pos, line) { }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  std::vector<std::shared_ptr<ASTNode>> values_;
};


class ASTDefineFun: public ASTNode, public std::enable_shared_from_this<ASTDefineFun> {
public:
  ASTDefineFun(
   int pos,
   const std::string& line,
   const std::vector<std::string>& params,
   std::vector<std::shared_ptr<ASTNode>>& body
  )
    : params_(params)
    , body_(body)
    , ASTNode(pos, line) { }

  const std::vector<std::string>& params() { return params_; }
  std::vector<std::shared_ptr<ASTNode>>& body() { return body_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  const std::vector<std::string> params_;
  std::vector<std::shared_ptr<ASTNode>> body_;
};


class ASTCallNamed: public ASTNode, public std::enable_shared_from_this<ASTCallNamed> {
public:
  ASTCallNamed(
    int pos,
    const std::string& line,
    const std::string& name,
    std::vector<std::shared_ptr<ASTNode>> args
  )
    : name_(name)
    , args_(args)
    , ASTNode(pos, line) { }

  const std::string& name() const { return name_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  const std::string name_;
  std::vector<std::shared_ptr<ASTNode>> args_;
};


class ASTAssignment: public ASTNode {
public:
  ASTAssignment(
    int pos,
    const std::string& line,
    const std::string& name,
    std::shared_ptr<ASTNode> value
  )
    : name_(name)
    , value_(value)
    , ASTNode(pos, line) { }

  const std::string& name() const { return name_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  const std::string name_;
  std::shared_ptr<ASTNode> value_;
};


class ASTDelete: public ASTNode {
public:
  ASTDelete(int pos, const std::string& line, const std::string& name)
    : name_(name), ASTNode(pos, line) { }

  const std::string& name() const { return name_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  const std::string name_;
};


class ASTIdentifier: public ASTNode {
public:
  ASTIdentifier(int pos, const std::string& line, const std::string& name)
    : name_(name), ASTNode(pos, line) { }

  const std::string& name() const { return name_; }

  std::shared_ptr<Object> run(
    std::shared_ptr<Scope> scope,
    std::vector<std::shared_ptr<ASTNode>>& stack
  ) override;

private:
  const std::string name_;
};


//// error handling (in parsing and while running code)


std::string error_arrow(int position);
std::runtime_error error(int position, const std::string& message);
std::runtime_error error(
  std::vector<std::shared_ptr<ASTNode>>& stack,
  const std::string& message
);


//// parsing: turning the source code into ASTNodes


typedef std::pair<int, const std::string> PosToken;


std::vector<PosToken> tokenize(const std::string& line);

std::shared_ptr<ASTNode>
  parse(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_int(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_list(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_fun(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_call(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_assign(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_delete(int& i, const std::vector<PosToken>& tokens, const std::string& line);
std::shared_ptr<ASTNode>
  parse_id(int& i, const std::vector<PosToken>& tokens, const std::string& line);


//// error handling ////////////////////////////////////////////////////////


std::string error_arrow(int position) {
  return std::string("---") + std::string(position, '-') + std::string("^\n");
}


std::runtime_error error(int position, const std::string& message) {
  return std::runtime_error(error_arrow(position) + message);
}


std::runtime_error error(
  std::vector<std::shared_ptr<ASTNode>>& stack,
  const std::string& message
) {
  std::string stack_trace;
  for (int i = 0;  i < stack.size();  i++ ) {
    if (i != 0) {
      stack_trace += "   " + stack[i]->line() + "\n";
    }
    stack_trace += error_arrow(stack[i]->pos());
  }
  return std::runtime_error(stack_trace + message);
}


//// parsing ///////////////////////////////////////////////////////////////


std::vector<PosToken> tokenize(const std::string& line) {
  std::regex whitespaces("\\s*");
  std::regex token_regex("(-?[0-9]+|[A-Za-z_][A-Za-z_0-9]*|\\(|\\)|\\[|\\]|,|;|\\{|\\}|=)");
  auto tokens_begin = std::sregex_iterator(line.begin(), line.end(), token_regex);
  auto tokens_end = std::sregex_iterator();

  int previous = 0;
  std::vector<PosToken> out;

  for (auto token_iter = tokens_begin;  token_iter != tokens_end;  ++token_iter) {
    // text between tokens must be only whitespace
    std::string between = line.substr(previous, token_iter->position() - previous);
    if (!std::regex_match(between, whitespaces)) {
      throw error(previous, "unexpected characters");
    }
    previous = token_iter->position() + token_iter->length();

    // collect tokens
    out.push_back(PosToken(token_iter->position(), token_iter->str()));
  }
  return out;
}


std::shared_ptr<ASTNode>
  parse(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  if (i >= tokens.size()) {
    throw error(0, "line ends without complete expression");
  }

  std::regex is_number("-?[0-9]+");
  std::regex is_name("[A-Za-z_][A-Za-z_0-9]*");

  if (tokens[i].second == "[") {
    return parse_list(i, tokens, line);
  }

  else if (tokens[i].second == "def") {
    return parse_fun(i, tokens, line);
  }

  else if (tokens[i].second == "del") {
    return parse_delete(i, tokens, line);
  }

  else if (std::regex_match(tokens[i].second, is_number)) {
    return parse_int(i, tokens, line);
  }

  else if (std::regex_match(tokens[i].second, is_name)) {
    if (i + 1 < tokens.size()  &&  tokens[i + 1].second == "=") {
      return parse_assign(i, tokens, line);
    }

    else if (i + 1 < tokens.size()  &&  tokens[i + 1].second == "(") {
      return parse_call(i, tokens, line);
    }

    else {
      return parse_id(i, tokens, line);
    }
  }

  else {
    throw error(tokens[i].first, "unrecognized syntax");
  }
}


std::shared_ptr<ASTNode>
  parse_int(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  int pos = tokens[i].first;
  int value = std::stoi(tokens[i].second);

  i++;  // get past int

  return std::make_shared<ASTLiteralInt>(pos, line, value);
}


std::shared_ptr<ASTNode>
  parse_list(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  int pos = tokens[i].first;

  i++;   // get past "["

  std::vector<std::shared_ptr<ASTNode>> values;

  bool first = true;
  while (tokens[i].second != "]") {
    if (!first) {
      if (tokens[i].second != ",") {
        throw error(tokens[i].first, "commas are required between list items");
      }
      i++;  // get past ","
    }
    first = false;

    values.push_back(parse(i, tokens, line));
  }

  i++;   // get past "]"

  return std::make_shared<ASTLiteralList>(pos, line, values);
}


std::shared_ptr<ASTNode>
  parse_fun(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  int pos = tokens[i].first;

  i++;  // get past "def"

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

      body.push_back(parse(i, tokens, line));
    }

    i++;   // get past "}"
  }
  else {
    // no curly brackets; only one statement
    body.push_back(parse(i, tokens, line));
  }

  return std::make_shared<ASTDefineFun>(pos, line, params, body);
}


std::shared_ptr<ASTNode>
  parse_call(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
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

    args.push_back(parse(i, tokens, line));
  }

  i++;   // get past ")"

  return std::make_shared<ASTCallNamed>(pos, line, name, args);
}


std::shared_ptr<ASTNode>
  parse_assign(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name
  i++;  // get past "="

  std::shared_ptr<ASTNode> value = parse(i, tokens, line);

  return std::make_shared<ASTAssignment>(pos, line, name, value);
}


std::shared_ptr<ASTNode>
  parse_delete(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
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

  return std::make_shared<ASTDelete>(pos, line, name);
}


std::shared_ptr<ASTNode>
  parse_id(int& i, const std::vector<PosToken>& tokens, const std::string& line) {
  int pos = tokens[i].first;
  const std::string name = tokens[i].second;

  i++;  // get past name

  return std::make_shared<ASTIdentifier>(pos, line, name);
}


//// Scope /////////////////////////////////////////////////////////////////


void Scope::assign(
  const std::string& name,
  std::shared_ptr<Object> object,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  objects_[name] = object;
}


std::shared_ptr<Object> Scope::del(
  const std::string& name,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  if (objects_.count(name)) {
    std::shared_ptr<Object> result = objects_[name];
    objects_.erase(name);
    return result;
  }
  else if (parent_) {
    return parent_->del(name, stack);
  }
  else {
    throw error(stack, "there is no variable named '" + name + "'");
  }
}


std::shared_ptr<Object> Scope::get(
  const std::string& name,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  if (objects_.count(name)) {
    return objects_[name];
  }
  else if (parent_) {
    return parent_->get(name, stack);
  }
  else {
    throw error(stack, "there is no variable named '" + name + "'");
  }
}


//// Objects ///////////////////////////////////////////////////////////////


std::string ObjectInt::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  std::string out = std::to_string(value_);

  remaining -= out.size();

  return out;
}


std::string ObjectList::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining--;

  std::string out = "[";
  for (int i = 0;  i < values_.size();  i++) {
    if (i != 0) {
      out += ", ";
      remaining -= 2;
    }
    out += values_[i]->repr(remaining);

    if (remaining < 0) {
      break;
    }
  }

  if (remaining >= 0) {
    out += "]";
  }

  remaining--;

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
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 2) {
    throw error(stack, "'add' function takes exactly 2 arguments");
  }

  std::shared_ptr<ObjectInt> arg0_int = std::dynamic_pointer_cast<ObjectInt>(args[0]);
  std::shared_ptr<ObjectInt> arg1_int = std::dynamic_pointer_cast<ObjectInt>(args[1]);

  std::shared_ptr<ObjectList> arg0_list = std::dynamic_pointer_cast<ObjectList>(args[0]);
  std::shared_ptr<ObjectList> arg1_list = std::dynamic_pointer_cast<ObjectList>(args[1]);

  if (arg0_int  &&  arg1_int) {
    return std::make_shared<ObjectInt>(arg0_int->value() + arg1_int->value());
  }

  else if (arg0_list  &&  arg1_list) {
    std::vector<std::shared_ptr<Object>> values;
    for (int i = 0;  i < arg0_list->values().size();  i++) {
      values.push_back(arg0_list->values()[i]);
    }
    for (int i = 0;  i < arg1_list->values().size();  i++) {
      values.push_back(arg1_list->values()[i]);
    }
    return std::make_shared<ObjectList>(values);
  }

  else {
    throw error(stack, "'add' function's arguments must both be integers or both be lists");
  }
}


std::string ObjectFunctionMul::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 24;

  return "<builtin function 'mul'>";
}


std::shared_ptr<Object> ObjectFunctionMul::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 2) {
    throw error(stack, "'mul' function takes exactly 2 arguments");
  }

  std::shared_ptr<ObjectInt> arg0_int = std::dynamic_pointer_cast<ObjectInt>(args[0]);
  std::shared_ptr<ObjectInt> arg1_int = std::dynamic_pointer_cast<ObjectInt>(args[1]);

  if (arg0_int  &&  arg1_int) {
    return std::make_shared<ObjectInt>(arg0_int->value() * arg1_int->value());
  }

  else {
    throw error(stack, "'mul' function's arguments must both be integers");
  }
}


std::string ObjectFunctionGet::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 24;

  return "<builtin function 'get'>";
}


std::shared_ptr<Object> ObjectFunctionGet::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 2) {
    throw error(stack, "'get' function takes exactly 2 arguments");
  }

  std::shared_ptr<ObjectList> arg0_list = std::dynamic_pointer_cast<ObjectList>(args[0]);
  std::shared_ptr<ObjectInt> arg1_int = std::dynamic_pointer_cast<ObjectInt>(args[1]);

  if (arg0_list  &&  arg1_int) {
    return arg0_list->values()[arg1_int->value()];
  }

  else {
    throw error(stack, "'get' function's arguments must be a list (first) and an integer (second)");
  }
}


std::string ObjectFunctionLen::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 24;

  return "<builtin function 'len'>";
}


std::shared_ptr<Object> ObjectFunctionLen::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 1) {
    throw error(stack, "'len' function takes exactly 1 argument");
  }

  std::shared_ptr<ObjectList> arg0_list = std::dynamic_pointer_cast<ObjectList>(args[0]);

  if (arg0_list) {
    return std::make_shared<ObjectInt>(arg0_list->values().size());
  }

  else {
    throw error(stack, "'len' function's argument must be a list");
  }
}


std::string ObjectFunctionMap::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 24;

  return "<builtin function 'map'>";
}


std::shared_ptr<Object> ObjectFunctionMap::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != 2) {
    throw error(stack, "'map' function takes exactly 2 arguments");
  }

  std::shared_ptr<ObjectFunction> arg0_function = std::dynamic_pointer_cast<ObjectFunction>(args[0]);
  std::shared_ptr<ObjectList> arg1_list = std::dynamic_pointer_cast<ObjectList>(args[1]);

  if (arg0_function  &&  arg1_list) {
    std::vector<std::shared_ptr<Object>> values;
    for (int i = 0;  i < arg1_list->values().size();  i++) {
      std::vector<std::shared_ptr<Object>> farg;
      farg.push_back(arg1_list->values()[i]);

      std::shared_ptr<Object> result = arg0_function->run(scope, stack, farg);

      values.push_back(result);
    }

    return std::make_shared<ObjectList>(values);
  }

  else {
    throw error(stack, "'map' function's arguments must be a function (first) and a list (second)");
  }
}


std::string ObjectFunctionReduce::repr(int& remaining) const {
  if (remaining < 0) {
    return "";
  }

  remaining -= 21;

  return "<builtin function 'reduce'>";
}


std::shared_ptr<Object> ObjectFunctionReduce::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() == 2) {
    std::shared_ptr<ObjectFunction> arg0_function = std::dynamic_pointer_cast<ObjectFunction>(args[0]);
    std::shared_ptr<ObjectList> arg1_list = std::dynamic_pointer_cast<ObjectList>(args[1]);

    if (arg1_list->values().size() == 0) {
      throw error(stack, "'reduce' function's list argument can only be empty if a third argument (the initial value) is provided");
    }

    std::shared_ptr<Object> result = arg1_list->values()[0];

    for (int i = 1;  i < arg1_list->values().size();  i++) {
      std::vector<std::shared_ptr<Object>> fargs;
      fargs.push_back(result);
      fargs.push_back(arg1_list->values()[i]);

      result = arg0_function->run(scope, stack, fargs);
    }

    return result;
  }

  else if (args.size() == 3) {
    std::shared_ptr<ObjectFunction> arg0_function = std::dynamic_pointer_cast<ObjectFunction>(args[0]);
    std::shared_ptr<ObjectList> arg1_list = std::dynamic_pointer_cast<ObjectList>(args[1]);
    std::shared_ptr<Object> result = args[2];

    for (int i = 0;  i < arg1_list->values().size();  i++) {
      std::vector<std::shared_ptr<Object>> fargs;
      fargs.push_back(result);
      fargs.push_back(arg1_list->values()[i]);

      result = arg0_function->run(scope, stack, fargs);
    }

    return result;
  }

  else {
    throw error(stack, "'reduce' function takes either 2 or 3 arguments");
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
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack,
  std::vector<std::shared_ptr<Object>> args
) {
  if (args.size() != fun_->params().size()) {
    throw error(stack, "wrong number of arguments for user-defined function");
  }

  std::shared_ptr<Scope> nested_scope(scope);

  for (int i = 0;  i < args.size();  i++) {
    nested_scope->assign(fun_->params()[i], args[i], stack);
  }

  std::shared_ptr<Object> out;
  for (int i = 0;  i < fun_->body().size();  i++) {
    out = fun_->body()[i]->run(nested_scope, stack);
  }
  return out;
}


//// ASTNodes //////////////////////////////////////////////////////////////


std::shared_ptr<Object> ASTLiteralInt::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  return std::make_shared<ObjectInt>(value_);
}


std::shared_ptr<Object> ASTLiteralList::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  std::vector<std::shared_ptr<Object>> values;

  for (int i = 0;  i < values_.size();  i++) {
    values.push_back(values_[i]->run(scope, stack));
  }

  return std::make_shared<ObjectList>(values);
}


std::shared_ptr<Object> ASTDefineFun::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  return std::make_shared<ObjectUserFunction>(shared_from_this());
}


std::shared_ptr<Object> ASTCallNamed::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  if (stack.size() == MAX_RECURSION) {
    throw error(stack, "recursion is too deep (probably an infinite loop)");
  }

  std::shared_ptr<Object> maybe_fun = scope->get(name_, stack);

  std::shared_ptr<ObjectFunction> fun = std::dynamic_pointer_cast<ObjectFunction>(maybe_fun);

  if (!fun) {
    throw error(stack, "attempting to call an object that is not a function");
  }

  std::vector<std::shared_ptr<Object>> args;
  for (int i = 0;  i < args_.size();  i++) {
    args.push_back(args_[i]->run(scope, stack));
  }

  std::shared_ptr<Scope> nested_scope = std::make_shared<Scope>(scope);

  stack.push_back(shared_from_this());
  std::shared_ptr<Object> result = fun->run(nested_scope, stack, args);
  stack.pop_back();

  return result;
}


std::shared_ptr<Object> ASTAssignment::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  std::shared_ptr<Object> result = value_->run(scope, stack);

  scope->assign(name_, result, stack);

  return result;
}


std::shared_ptr<Object> ASTDelete::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  return scope->del(name_, stack);
}


std::shared_ptr<Object> ASTIdentifier::run(
  std::shared_ptr<Scope> scope,
  std::vector<std::shared_ptr<ASTNode>>& stack
) {
  return scope->get(name_, stack);
}


//// main function /////////////////////////////////////////////////////////


int main(int argc, char** argv) {
  // create a variable Scope
  std::shared_ptr<Scope> scope = std::make_shared<Scope>(nullptr);

  // and put some built-ins in it
  std::vector<std::shared_ptr<ASTNode>> stack;
  scope->assign("add", std::make_shared<ObjectFunctionAdd>(), stack);
  scope->assign("mul", std::make_shared<ObjectFunctionMul>(), stack);
  scope->assign("get", std::make_shared<ObjectFunctionGet>(), stack);
  scope->assign("len", std::make_shared<ObjectFunctionLen>(), stack);
  scope->assign("map", std::make_shared<ObjectFunctionMap>(), stack);
  scope->assign("reduce", std::make_shared<ObjectFunctionReduce>(), stack);

  // use the command-line arguments to add some data from files
  for (int argi = 1;  argi < argc;  argi++) {
    std::string arg = argv[argi];

    std::string::size_type pos = arg.find('=');
    if (arg.find('=') == std::string::npos) {
      std::cout << "arguments must be separated by '=', as in: data=/path/to/data.int32" << std::endl;
      return -1;
    }

    std::string var_name = arg.substr(0, pos);
    std::string file_name = arg.substr(pos + 1, -1);

    std::ifstream file(file_name, std::ios::binary);
    if (!file) {
      std::cout << "could not open file: " << file_name << std::endl;
      return -1;
    }

    std::vector<std::shared_ptr<Object>> values;
    int32_t raw;
    while (file.read(reinterpret_cast<char*>(&raw), sizeof(raw))) {
      values.push_back(std::make_shared<ObjectInt>(raw));
    }

    file.close();

    scope->assign(var_name, std::make_shared<ObjectList>(values), stack);
  }

  // baby-python startup screen!
  std::cout << "                     num = -123        add(x, x)   get(lst, i)   map(f, lst)" << std::endl;
  std::cout << "               oo    lst = [1, 2, 3]   mul(x, x)   len(lst)      reduce(f, lst)" << std::endl;
  std::cout << ". . . __/\\_/\\_/`'    f = def(x) single-expr   f = def(x, y) { ... ; last-expr }" << std::endl;
  std::cout << std::endl;

  linenoise::LoadHistory(".baby-python-history");

  // start the REPL (Read Evaluate Print Loop)
  while (true) {
    std::string line;
    bool quit = linenoise::Readline(">> ", line);
    if (quit) {
      linenoise::SaveHistory(".baby-python-history");
      break;
    }
    linenoise::AddHistory(line.c_str());

    // parse the line in two steps
    std::vector<PosToken> tokens;
    int i = 0;
    std::shared_ptr<ASTNode> ast;
    try {
      // (1) break the whole string into a list of tokens
      tokens = tokenize(line);
      // (2) build an AST tree from the tokens
      ast = parse(i, tokens, line);
    }
    catch (std::runtime_error const& exception) {
      // syntax error while tokenizing or building AST
      std::cout << exception.what() << std::endl;
    }

    if (tokens.size() == 1  &&  tokens[0].second == "exit") {
      linenoise::SaveHistory(".baby-python-history");
      break;
    }

    if (ast) {
      if (i < tokens.size()) {
        // unused tokens after building a whole AST is an error
        std::cout << error_arrow(tokens[i].first);
        std::cout << "complete expression, but line doesn't end" << std::endl;
      }

      else {
        // create a new stack and attempt to run the AST
        std::vector<std::shared_ptr<ASTNode>> stack;
        std::shared_ptr<Object> result(nullptr);
        try {
          result = ast->run(scope, stack);
        }
        catch (std::runtime_error const& exception) {
          std::cout << exception.what() << std::endl;
        }

        if (result) {
          // execution was successful! print the result!
          int remaining = MAX_REPR;
          std::string repr = result->repr(remaining);
          if (repr.size() > MAX_REPR) {
            repr = repr.substr(0, MAX_REPR - 3) + "...";
          }
          std::cout << repr << std::endl;
        }

      }
    }
  }

  return 0;
}


//// The end! (About half as much code as the line-editor.)
