#include <iostream>
#include <vector>
#include <any>
#include <boost/fusion/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast.hpp"

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
	qi::rule<Iterator, ast::StmtsAst*(), Skipper> Stmts;
	qi::rule<Iterator, ast::BaseAst*(), Skipper> Factor;
	qi::rule<Iterator, ast::BaseAst*(), Skipper> Expr, E1, E2, E3, E4, E5;

	Calc() : Calc::base_type(Module)
	{
		Ident = qi::lexeme[(qi::alpha[_val = _1] | qi::char_('_')[_val = _1]) >> *(qi::alnum[_val += _1] | qi::char_('_')[_val += 1])];
		Vars = "let" >> Ident[ph::push_back(_val, _1)] >> *(',' >> Ident[ph::push_back(_val, _1)]);
		Module = Vars[_val = ph::new_<ast::ModuleAst>(), ph::at_c<0>(*_val) = _1] >> *Func[ph::push_back(ph::at_c<1>(*_val), _1)];
		Func = "fn" >> Ident[_val = ph::new_<ast::FuncAst>(_1)] >> *Stmts[ph::push_back(ph::at_c<1>(*_val), _1)];
		Stmt = Builtin | Assign | IfStmt | WhileStmt;
		Assign = Ident[_val = ph::new_<ast::AssignAst>(_1)] >> '=' >> Expr[ph::at_c<1>(*_val) = _1];
		Builtin = ("hoge" >> qi::eps[_val = ph::new_<ast::BuiltinAst>("hoge")]);
		IfStmt = "if" >> Expr[_val = ph::new_<ast::IfStmtAst>(), ph::at_c<0>(*_val) = _1]
				  >> "then" >> Stmts[ph::at_c<1>(*_val) = _1]
				  >> -("else" >> Stmts[ph::at_c<2>(*_val) = _1]);
		WhileStmt = "while" >> Expr[_val = ph::new_<ast::WhileStmtAst>(), ph::at_c<0>(*_val) = _1] >> "do"
					 >> *Stmts[ph::push_back(ph::at_c<1>(*_val), _1)] >> "end";
		Stmts = qi::eps[_val = ph::new_<ast::StmtsAst>()]
				>> Stmt[ph::push_back(ph::at_c<0>(*_val), _1)
				>> *(':' >> Stmt[ph::push_back(ph::at_c<0>(*_val), _1)])];

		Expr %= E5;
		Factor %= qi::skip(qi::space) \
			[qi::int_ | '(' >> Expr >> ')'];
		E1 = qi::skip(qi::space) \
			[Factor[_val = _1] >>
				*(('*' >> Factor[_val = ph::new_<ast::BinaryExpAst>("*", _val, _1)])
				| ('/' >> Factor[_val = ph::new_<ast::BinaryExpAst>("/", _val, _1)])
				| ('%' >> Factor[_val = ph::new_<ast::BinaryExpAst>("%", _val, _1)]))];
		E2 = qi::skip(qi::space) \
			[E1[_val = _1] >>
				*(('+' >> E1[_val = ph::new_<ast::BinaryExpAst>("+", _val, _1)])
				| ('-' >> E1[_val = ph::new_<ast::BinaryExpAst>("-", _val, _1)]))];
		E3 = qi::skip(qi::space) \
			[E2[_val = _1] >>
				*(("==" >> E2[_val = ph::new_<ast::BinaryExpAst>("==", _val, _1)])
				| ("!=" >> E2[_val = ph::new_<ast::BinaryExpAst>("!=", _val, _1)])
				| ('<' >> E2[_val = ph::new_<ast::BinaryExpAst>("<", _val, _1)])
				| ('>' >> E2[_val = ph::new_<ast::BinaryExpAst>(">", _val, _1)])
				| ("<=" >> E2[_val = ph::new_<ast::BinaryExpAst>("<=", _val, _1)])
				| (">=" >> E2[_val = ph::new_<ast::BinaryExpAst>(">=", _val, _1)]))];
		E4 = E3[_val = _1] | qi::skip(qi::space)['!' >> E3[_val = ph::new_<ast::MonoExpAst>("!", _1)]];
		E5 = qi::skip(qi::space) \
			[E4[_val = _1] >>
				*(("&&" >> E4[_val = ph::new_<ast::BinaryExpAst>("&&", _val, _1)])
				| ("||" >> E4[_val = ph::new_<ast::BinaryExpAst>("||", _val, _1)]))];
	}
};

} // namespace parser
} // namespace myLang


int main()
{

	while(true)
	{
		std::string s{}; getline(std::cin, s);
		if(s == "exit") { return !puts("exit"); }

		auto it = std::begin(s);
		myLang::parser::Calc<std::string::iterator, qi::space_type> calc;
		myLang::ast::ModuleAst* res = nullptr;
		bool success = qi::phrase_parse(
			it,
			std::end(s),
			calc,
			qi::space,
			res
		);

		if(success and it == std::end(s))
		{
			std::cout << "OK" << std::endl;
			// std::cout << "OK result: " << res << std::endl;
		}
		else std::cout << "NG" << std::endl;
	}

	return 0;
}