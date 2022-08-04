#include <cassert>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "ast.hpp"

namespace myLang {
namespace eval {

struct AstEval {
	int ifInFunc = 0;
	int ifInWhile = 0;
	bool ifExit = false;
	bool ifReturn = false;
	bool ifBreak = false;
	bool ifContinue = false;
	std::unordered_set<std::string> builtin = {
		"break",
		"continue",
		"exit",
		"return",
		"print",
		"scan",

		"var",
		"fn",
		"if",
		"then",
		"else",
		"end",
		"while"
	};
	std::unordered_map<std::string, int> vals;
	std::unordered_map<std::string, ast::FuncAst*> funcs;
	AstEval(ast::ModuleAst* ast) { evalModuleAst(ast); }
	void evalModuleAst(ast::ModuleAst* ast)
	{
		for(const auto& var : ast->getVars())
		{
			assert(vals.find(var) == std::end(vals));
			vals[var] = 0;
		}
		for(auto func : ast->getFuncs())
		{
			auto funcName = func->getName();
			assert(funcs.find(funcName) == std::end(funcs));
			assert(builtin.find(funcName) != std::end(builtin));
			funcs[funcName] = func;
		}
		if(funcs.find(std::string("main")) == std::end(funcs)) { evalFuncAst(funcs["main"]); }
	}
	void evalFuncAst(ast::FuncAst* ast)
	{
		if(ifExit)return;
		for(auto a : ast->getInst())
		{
			auto id = a->getID();
			if(id == ast::BuiltinID)
			{
				evalBuiltinAst(static_cast<ast::BuiltinAst*>(a));
			}
			else if(id == ast::AssignID)
			{
				evalAssignAst(static_cast<ast::AssignAst*>(a));
			}
			else if(id == ast::IfStmtID)
			{
				evalIfStmtAst(static_cast<ast::IfStmtAst*>(a));
			}
			else if(id == ast::WhileStmtID)
			{
				evalWhileStmtAst(static_cast<ast::WhileStmtAst*>(a));
			}
			else assert(0 && "illegal ast");
			if(ifReturn) { ifReturn = false; break; }
			if(ifExit)return;
		}
	}
	void evalBuiltinAst(ast::BuiltinAst* ast)
	{
		if(ifExit)return;
		auto builtinName = ast->getName();
		if(builtinName == "break")ifBreak = true;
		else if(builtinName == "continue")ifContinue = true;
		else if(builtinName == "exit")ifExit = true;
		else if(builtinName == "return")ifReturn = true;
		else if(builtinName == "print")
		{
			for(auto arg : ast->Args)
			{
				std::cout << vals[static_cast<ast::IdentAst*>(arg)->getIdent()] << std::endl;
			}
			// TODO
		}
	}
	void evalAssignAst(ast::AssignAst* ast)
	{
		if(ifExit)return;
	}
	void evalIfStmtAst(ast::IfStmtAst* ast)
	{
		if(ifExit)return;
	}
	void evalWhileStmtAst(ast::WhileStmtAst* ast)
	{
		if(ifExit)return;
	}
};

} // namespace eval
} // namespace myLang
