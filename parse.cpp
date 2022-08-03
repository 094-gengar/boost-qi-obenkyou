#include <iostream>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
using qi::_1, qi::_val;

template<class Iterator>
struct Calc : qi::grammar<Iterator, int(void), qi::space_type> {
	qi::rule<Iterator, int(void), qi::space_type> Factor;
	qi::rule<Iterator, int(void), qi::space_type> Expr, E1, E2, E3, E4, E5;
	//qi::rule<Iterator, std::vector<char>(void), qi::space_type> Ident;

	Calc() : Calc::base_type(Expr)
	{
		//Ident %= (qi::alpha | qi::char_('_')) >> *(qi::alnum | qi::char_('_'));
		Expr %= E5;

		Factor %= (qi::int_ | '(' >> Expr >> ')');
		E5 %= (E4[_val = _1] >>
				*(("&&" >> E4[_val = _val && _1])
				| ("||" >> E4[_val = _val || _1])));
		E4 %= E3[_val = _1] | ('!' >> E3[_val = !_1]);
		E3 %= (E2[_val = _1] >>
				*(("==" >> E2[_val = _val == _1])
				| ("!=" >> E2[_val = _val != _1])
				| ('<' >> E2[_val = _val < _1])
				| ('>' >> E2[_val = _val > _1])
				| ("<=" >> E2[_val = _val <= _1])
				| (">=" >> E2[_val = _val >= _1])));
		E2 %= (E1[_val = _1] >>
				*(('+' >> E1[_val += _1])
				| ('-' >> E1[_val -= _1])));
		E1 %= (Factor[_val = _1] >>
				*(('*' >> Factor[_val *= _1])
				| ('/' >> Factor[_val /= _1])
				| ('%' >> Factor[_val %= _1])));
	}
};


int main()
{

	while(true)
	{
		std::string s{}; getline(std::cin, s);
		if(s == "exit") { return !puts("exit"); }

		int res;
		auto it = std::begin(s);
		Calc<std::string::iterator> calc;
		bool success = qi::phrase_parse(
			it,
			std::end(s),
			calc,
			qi::space,
			res
		);

		if(success and it == std::end(s))
		{
			std::cout << "OK result: " << res << std::endl;
		}
		else std::cout << "NG" << std::endl;
	}

	return 0;
}