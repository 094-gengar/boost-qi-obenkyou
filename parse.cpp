#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/fusion/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "ast.hpp"
#include "eval.hpp"

namespace qi = boost::spirit::qi;
namespace ph = boost::phoenix;
using qi::_1, qi::_val;

namespace myLang {
namespace parser {
template<class Iterator, class Skipper>
struct Calc : qi::grammar<Iterator, ast::ModuleAst*(), Skipper> {
	qi::rule<Iterator, std::string(), Skipper> Ident;
	qi::rule<Iterator, std::vector<std::string>(), Skipper> Vars;
	qi::rule<Iterator, ast::FuncAst*(), Skipper> Func;
	qi::rule<Iterator, ast::ModuleAst*(), Skipper> Module;
	qi::rule<Iterator, ast::BaseAst*(), Skipper> Stmt;
	qi::rule<Iterator, ast::BuiltinAst*(), Skipper> Builtin;
	qi::rule<Iterator, ast::AssignAst*(), Skipper> Assign;
	qi::rule<Iterator, ast::IfStmtAst*(), Skipper> IfStmt;
	qi::rule<Iterator, ast::WhileStmtAst*(), Skipper> WhileStmt;
	qi::rule<Iterator, std::vector<ast::BaseAst*>(), Skipper> Stmts;
	qi::rule<Iterator, ast::BaseAst*(), Skipper> Factor;
	qi::rule<Iterator, ast::BaseAst*(), Skipper> Expr, E1, E2, E3, E4;

	Calc() : Calc::base_type(Module)
	{
		Module = Vars[_val = ph::new_<ast::ModuleAst>(), ph::at_c<0>(*_val) = _1] >> *(Func[ph::push_back(ph::at_c<1>(*_val), _1)]);

		Ident = qi::lexeme[(qi::alpha | qi::char_('_'))[_val = _1] >> *((qi::alnum | qi::char_('_'))[_val += _1])];

		Vars = "var" >> Ident[ph::push_back(_val, _1)] >> *(',' >> Ident[ph::push_back(_val, _1)]) >> ';';

		Func = "fn" >> Ident[_val = ph::new_<ast::FuncAst>(_1)] >> '{' >> *Stmt[ph::push_back(ph::at_c<1>(*_val), _1)] >> '}';

		Stmt = Builtin | IfStmt | WhileStmt | Assign;

		Assign = Ident[_val = ph::new_<ast::AssignAst>(_1)] >> '=' >> Expr[ph::at_c<1>(*_val) = _1] >> ';';

		Builtin =
			 (("break" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("break")])
			| ("continue" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("continue")])
			| ("exit" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("exit")])
			| ("return" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("return")])
			| ("print" >> Ident[_val = ph::new_<ast::BuiltinAst>("print"), ph::push_back(ph::at_c<1>(*_val), ph::new_<ast::IdentAst>(_1))])
			| ("scan" >> Ident[_val = ph::new_<ast::BuiltinAst>("scan"), ph::push_back(ph::at_c<1>(*_val), ph::new_<ast::IdentAst>(_1))])) >> ';';

		IfStmt = "if" >> Expr[_val = ph::new_<ast::IfStmtAst>(), ph::at_c<0>(*_val) = _1]
			>> '{' >> *Stmt[ph::push_back(ph::at_c<1>(*_val), _1)]
			>> -(qi::char_('}') >> "else" >> '{' >> *Stmt[ph::push_back(ph::at_c<2>(*_val), _1)]) >> '}';

		WhileStmt = "while" >> Expr[_val = ph::new_<ast::WhileStmtAst>(), ph::at_c<0>(*_val) = _1] >> "{"
			>> *Stmt[ph::push_back(ph::at_c<1>(*_val), _1)] >> '}';

		Factor = qi::int_[_val = ph::new_<ast::NumberAst>(_1)]
			| Ident[_val = ph::new_<ast::IdentAst>(_1)]
			| '(' >> Expr[_val = _1] >> ')';
		E1 = Factor[_val = _1] >>
			*(('*' >> Factor[_val = ph::new_<ast::BinaryExpAst>("*", _val, _1)])
			| ('/' >> Factor[_val = ph::new_<ast::BinaryExpAst>("/", _val, _1)])
			| ('%' >> Factor[_val = ph::new_<ast::BinaryExpAst>("%", _val, _1)]));
		E2 = E1[_val = _1] >>
			*(('+' >> E1[_val = ph::new_<ast::BinaryExpAst>("+", _val, _1)])
			| ('-' >> E1[_val = ph::new_<ast::BinaryExpAst>("-", _val, _1)]));
		E3 = E2[_val = _1] >>
			*(("==" >> E2[_val = ph::new_<ast::BinaryExpAst>("==", _val, _1)])
			| ("!=" >> E2[_val = ph::new_<ast::BinaryExpAst>("!=", _val, _1)])
			| ('<' >> E2[_val = ph::new_<ast::BinaryExpAst>("<", _val, _1)])
			| ('>' >> E2[_val = ph::new_<ast::BinaryExpAst>(">", _val, _1)])
			| ("<=" >> E2[_val = ph::new_<ast::BinaryExpAst>("<=", _val, _1)])
			| (">=" >> E2[_val = ph::new_<ast::BinaryExpAst>(">=", _val, _1)]));
		E4 = E3[_val = _1] | ('!' >> E3[_val = ph::new_<ast::MonoExpAst>("!", _1)]);
		Expr = E4[_val = _1] >>
			*(("&&" >> E4[_val = ph::new_<ast::BinaryExpAst>("&&", _val, _1)])
			| ("||" >> E4[_val = ph::new_<ast::BinaryExpAst>("||", _val, _1)]));
	}
};

} // namespace parser
} // namespace myLang


int main(int argc, const char* argv[])
{
	std::string input{}, s;
	assert(argc >= 2);
	std::string fName(argv[1]);
	std::vector<std::string> lines;
	std::ifstream fIn(fName);
	if(not fIn.is_open())
	{
		std::cerr << "Could not open the file '" << fName << "'" << std::endl;
		return EXIT_FAILURE;
	}

	while(std::getline(fIn, s)) { input += s; input += '\n'; }

	auto it = std::begin(input);
	myLang::parser::Calc<std::string::iterator, qi::standard_wide::space_type> calc;
	myLang::ast::ModuleAst* res = nullptr;
	bool success = qi::phrase_parse(
		it,
		std::end(input),
		calc,
		qi::standard_wide::space,
		res
	);

	if(success and it == std::end(input))
	{
		std::cout << "OK" << std::endl;
		myLang::eval::AstEval asteval(res);
	}
	else { std::cout << "NG" << std::endl; }

	return EXIT_SUCCESS;
}