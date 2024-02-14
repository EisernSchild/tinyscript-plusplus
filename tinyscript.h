// SPDX-License-Identifier: Zlib
/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

 /*
  * TINYEXPR++ - Tiny recursive descent parser and evaluation engine in C++
  *
  * Copyright (c) 2020-2024 Blake Madden
  *
  * C++ version of the TinyExpr library.
  *
  * This software is provided 'as-is', without any express or implied
  * warranty. In no event will the authors be held liable for any damages
  * arising from the use of this software.
  *
  * Permission is granted to anyone to use this software for any purpose,
  * including commercial applications, and to alter it and redistribute it
  * freely, subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  * claim that you wrote the original software. If you use this software
  * in a product, an acknowledgement in the product documentation would be
  * appreciated but is not required.
  * 2. Altered source versions must be plainly marked as such, and must not be
  * misrepresented as being the original software.
  * 3. This notice may not be removed or altered from any source distribution.
  */

  /*
   * TINYSCRIPT++ - Tiny script parser based on TinyExpr in C++
   *
   * Copyright (c) 2024 Denis Reischl
   *
   * This software is provided 'as-is', without any express or implied
   * warranty. In no event will the authors be held liable for any damages
   * arising from the use of this software.
   *
   * Permission is granted to anyone to use this software for any purpose,
   * including commercial applications, and to alter it and redistribute it
   * freely, subject to the following restrictions:
   *
   * 1. The origin of this software must not be misrepresented; you must not
   * claim that you wrote the original software. If you use this software
   * in a product, an acknowledgement in the product documentation would be
   * appreciated but is not required.
   * 2. Altered source versions must be plainly marked as such, and must not be
   * misrepresented as being the original software.
   * 3. This notice may not be removed or altered from any source distribution.
   */

#ifndef __TINYSCRIPT_PLUS_PLUS_H__
#define __TINYSCRIPT_PLUS_PLUS_H__

#include "tinyexpr-plusplus/tinyexpr.h"
#include <sstream>

#define TS_OK 0
#define TS_FAIL -1

/// <summary>copy of te_variable with non constant value pointer</summary>
class ts_variable
{
public:
	/// @private
	using name_type = std::string;

	/// @private
	[[nodiscard]]
	bool
		operator<(const ts_variable& that) const
	{
		return te_string_less{}(m_name, that.m_name);
	}

	/// <summary>The name as it would appear in a formula.</summary>
	name_type m_name;
	/// <summary>The te_type variable address</summary>
	te_type* m_value;
	/// <summary>Constant type TE_DEFAULT</summary>
	const te_variable_flags m_type{ TE_DEFAULT };
};

/// <summary>boolean variable</summary>
class ts_boolean
{
public:
	using name_type = std::string;

	/// <summary>operator needed for std::set</summary>
	/// <param name="that"></param>
	/// <returns></returns>
	[[nodiscard]]
	bool
		operator<(const ts_boolean& that) const
	{
		return te_string_less{}(atName, that.atName);
	}

	/// <summary>The name or identifier.</summary>
	name_type atName;
	/// <summary>The boolean variable address</summary>
	bool* pbValue;
};

/// <summary>
/// compile simple scripts using TinyExpr++
/// </summary>
class ts_parser
{
public:
	/// <param name="atScript">the Script code</param>
	/// <param name="asVars">the script variables</param>
	/// <param name="asBools">the script booleans</param>
	explicit ts_parser(std::string_view atCode, std::set<ts_variable>& asVars, std::set<ts_boolean>& asBools)
	{
		pasVars = std::make_shared<std::set<ts_variable>>(asVars);
		pasBools = std::make_shared<std::set<ts_boolean>>(asBools);

		// remove comments
		atScript = {};
		bool bSingleLine = false, bMultiLine = false;
		for (size_t n = 0; n < atCode.size(); n++)
		{
			std::array<char, 2> at = { atCode[n], ((n + 1) < atCode.size()) ? atCode[n + 1] : ' ' };

			if (bSingleLine == true && at[0] == '\n')
			{
				// single line comment flag ?
				bSingleLine = false;
			}
			else if (bMultiLine == true && at[0] == '*' && at[1] == '/')
			{
				// multiple line comment ? 
				bMultiLine = false;
				n++;
			}
			else if (bSingleLine || bMultiLine)
			{
				// is in comment
				continue;
			}
			else if (at[0] == '/' && at[1] == '/')
			{
				// new comment
				bSingleLine = true;
				n++;
			}
			else if (at[0] == '/' && at[1] == '*')
			{
				// new comment
				bMultiLine = true;
				n++;
			}
			else
				atScript += at[0];
		}

		// cleanup the script
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\r'), atScript.end());
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\n'), atScript.end());
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\t'), atScript.end());
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\v'), atScript.end());
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\f'), atScript.end());
		atScript.erase(std::remove(atScript.begin(), atScript.end(), '\0'), atScript.end());

		// add separator after curly close brace to ensure statement split
		size_t uFind = 0;
		while ((uFind = atScript.find("}", uFind)) != std::string::npos) atScript.insert(++uFind, 1, ';');

		// split the statements
		std::stringstream atS(atScript);
		std::string atStatement;
		std::vector<std::string> aatSme;
		while (std::getline(atS, atStatement, ';'))
			aatSme.push_back(atStatement);

		// split the statements again by curly braces
		std::vector<std::string> aatSmeSplit;
		for (std::string& at : aatSme)
		{
			// remove spaces at start and end
			while ((at.size()) && (at.front() == ' '))
				at.erase(at.begin());
			while ((at.size()) && (at.back() == ' '))
				at.erase(at.end() - 1);

			// skip empty
			if (!at.size()) continue;

			// add curly braces for one line "if" statements to split them later
			if (at.substr(0, 2) == "if")
			{
				std::size_t uLast = at.find_last_of(')');
				if (at.substr(uLast + 1).size())
				{
					if (at.find('{') == std::string::npos)
					{
						at.insert(uLast + 1, 1, '{');
						if (at.find('}') == std::string::npos)
							at.insert(at.size(), 1, '}');
					}
				}
			}

			// search for braces and sort indices
			uFind = 0;
			std::vector<size_t> auFound;
			while ((uFind = at.find("{", uFind)) != std::string::npos) auFound.push_back(uFind++);
			uFind = 0;
			while ((uFind = at.find("}", uFind)) != std::string::npos) auFound.push_back(uFind++);
			sort(auFound.begin(), auFound.end());

			// split by found
			uFind = 0;
			for (size_t uF : auFound)
			{
				// add string to delimiter, delimiter string separately
				if (uFind < uF)
					aatSmeSplit.push_back(at.substr(uFind, uF - uFind));
				aatSmeSplit.push_back(at.substr(uF, 1));
				uFind = uF + 1;
			}
			if (uFind < at.size()) aatSmeSplit.push_back(at.substr(uFind, at.size() - uFind));
		}

		// set level flags vector size to 1
		aeFlags.resize(1);

		// loop through statements and compile them
		for (std::string& at : aatSmeSplit)
		{
			if (at == "{")
				block_level_up();
			else if (at == "}")
			{
				if (uBlockLevel > 0)
					block_level_down();
				else
				{
					nErr = TS_FAIL;
					return;
				}
			}
			else
			{
				// compile statements and add to script vector
				apsStatements_compiled.push_back(std::make_shared<ts_statement>(ts_statement(at, pasVars, pasBools, uBlockLevel)));
				auto nE = apsStatements_compiled.back()->error();
				if (nE)
				{
					nErr = nE;
					apsStatements_compiled.clear();
					return;
				}
			}
		}

		// block level back to zero ?
		if (uBlockLevel != 0) nErr = TS_FAIL;
	}

	/// <summary>evaluate script based on current variable values</summary>
	void evaluate()
	{
		if (!nErr)
		{
			// delete all flags and set level to zero
			for (uBlockLevel = 0; uBlockLevel < (unsigned)aeFlags.size(); uBlockLevel++)
				aeFlags[uBlockLevel] = ts_runtime_flags::none;
			uBlockLevel = 0;

			for (auto& s : apsStatements_compiled)
			{
				// get statement block level
				unsigned uLevelThis = s->level();

				// skip if false flag and level higher
				if ((uLevelThis > uBlockLevel) && ((unsigned)aeFlags[uBlockLevel] & (unsigned)ts_runtime_flags::if_false))
					continue;

				// delete all higher flags if smaller level
				if (uLevelThis < uBlockLevel)
				{
					for (; uBlockLevel > uLevelThis; uBlockLevel--)
						aeFlags[uBlockLevel] = ts_runtime_flags::none;
				}

				// set new block level and process
				uBlockLevel = uLevelThis;
				switch (s->type())
				{
				case ts_parser::ts_types::sm_expr_float:
				case ts_parser::ts_types::sm_expr_bool:
					s->evaluate();
					aeFlags[uBlockLevel] = ts_runtime_flags::none;
					break;
				case ts_parser::ts_types::sm_if:
					s->evaluate();
					aeFlags[uBlockLevel] = s->boolean() ? ts_runtime_flags::if_true : ts_runtime_flags::if_false;
					break;
				case ts_parser::ts_types::sm_if_else:
					break;
				case ts_parser::ts_types::sm_undefined:
				default:
					return;
				}
			}
		}
	}

private:

	/// <summary>possible statement types</summary>
	enum struct ts_types : unsigned
	{
		sm_undefined = 0,
		sm_expr_float,
		sm_expr_bool,
		sm_if,
		sm_if_else
	};

	/// <summary>flags used during evaluation process</summary>
	enum struct ts_runtime_flags : unsigned
	{
		none = 0b00000000,
		if_true = 0b00000001,
		if_false = 0b00000010,
	};

	/// <summary>state in the current statement compilation process</summary>
	struct state
	{
		/// <param name="_atStatement">the statement string</param>
		/// <param name="_pasVars">shared pointer to the script variables</param>
		/// <param name="_pasBools">shared pointer to the script booleans</param>
		explicit state(std::string& _atStatement,
			std::shared_ptr<std::set<ts_variable>> _pasVars,
			std::shared_ptr<std::set<ts_boolean>> _pasBools
		)
			: atStatement(_atStatement)
			, pasVars(_pasVars)
			, pasBools(_pasBools)
		{
		}

		/// <summary>possible comparisation tokens</summary>
		enum struct token_compare_type : unsigned
		{
			TOK_EQUAL = 1,
			TOK_UNEQUAL,
			TOK_GREATER,
			TOK_LESS,
			TOK_GREATER_EQUAL,
			TOK_LESS_EQUAL,
			TOK_AND,
			TOK_OR
		};

		/// <summary>possible token type enumeration</summary>
		enum struct token_type : unsigned
		{
			TOK_NULL = 0,
			TOK_EQUAL = (unsigned)token_compare_type::TOK_EQUAL,
			TOK_UNEQUAL = (unsigned)token_compare_type::TOK_UNEQUAL,
			TOK_GREATER = (unsigned)token_compare_type::TOK_GREATER,
			TOK_LESS = (unsigned)token_compare_type::TOK_LESS,
			TOK_GREATER_EQUAL = (unsigned)token_compare_type::TOK_GREATER_EQUAL,
			TOK_LESS_EQUAL = (unsigned)token_compare_type::TOK_LESS_EQUAL,
			TOK_AND = (unsigned)token_compare_type::TOK_AND,
			TOK_OR = (unsigned)token_compare_type::TOK_OR,
			TOK_ERROR,
			TOK_END,
			TOK_OPEN,
			TOK_CLOSE,
			TOK_OPEN_CURLY,
			TOK_CLOSE_CURLY,
			TOK_NUMBER,
			TOK_IF,
			TOK_ELSE,
			TOK_VAR_FLOAT,
			TOK_VAR_BOOL,
			TOK_ASSIGN,
			TOK_TRUE,
			TOK_FALSE
		};

		/// <summary>get the next token in current statement stream</summary>
		void next_token()
		{
			eType = token_type::TOK_NULL;

			do
			{
				// end of expression
				if (uNext >= atStatement.size())
				{
					eType = token_type::TOK_END;
					return;
				}

				// Try reading a number
				if ((peek() >= '0' && peek() <= '9') || peek() == '.')
				{
					pop_number();
				}
				else
				{
					// Look for a variable or builtin function call.
					if (isalpha(peek()))
					{
						pop_vocable();
					}
					else
					{
						// Look for operator or special.. otherwise move forward or throw error
						pop_operator();
					}
				}
			} while (eType == token_type::TOK_NULL);
		}
		/// <summary>get current token type</summary>
		token_type get_type() { return eType; }
		/// <summary>get remaining statement string</summary>
		std::string remaining() { return atStatement.substr(uNext); }
		/// <summary>get the value unsigned (usually an index)</summary>
		unsigned value_unsigned() { return std::get<unsigned>(sValue); }
		/// <summary>get the value floating</summary>
		te_type value_floating() { return std::get<te_type>(sValue); }

	private:
		/// <summary>peek next character in expression</summary>
		char peek() { if (uNext < atStatement.size()) return atStatement[uNext]; else return 0; }
		/// <summary>pop next character in expression</summary>
		char pop() { if (uNext++ < atStatement.size()) return atStatement[uNext - 1]; else return 0; }
		/// <summary>pop next number in expression</summary>
		void pop_number() { size_t uIx = 0; float fRet = std::stof(atStatement.substr(uNext), &uIx); uNext += uIx; sValue = (te_type)fRet; eType = token_type::TOK_NUMBER; }
		/// <summary>pop next vocable</summary>
		void pop_vocable()
		{
			std::string at = {};
			while (isalpha(peek()) || isdigit(peek()) || (peek() == '_')) at.push_back(pop());

			int nIx = find_vars(at);
			if (nIx < 0)
			{
				nIx = find_bools(at);
				if (nIx < 0)
				{
					if (at == "if")
						eType = token_type::TOK_IF;
					else if (at == "else")
						eType = token_type::TOK_ELSE;
					else if (at == "true")
						eType = token_type::TOK_TRUE;
					else if (at == "false")
						eType = token_type::TOK_FALSE;
					else
					{
						eType = token_type::TOK_ERROR;
						return;
					}
				}
				else
				{
					sValue = (unsigned)nIx;
					eType = token_type::TOK_VAR_BOOL;
				}
			}
			else
			{
				sValue = (unsigned)nIx;
				eType = token_type::TOK_VAR_FLOAT;
			}
		}
		/// <summary>
		/// Look for an operator or special character. 
		/// (or move forward or throw error for unknown character)
		/// </summary>
		void pop_operator()
		{
			std::array<char, 2> at = { pop(),  peek() };
			switch (at[0])
			{
			case '(': eType = token_type::TOK_OPEN; break;
			case ')': eType = token_type::TOK_CLOSE; break;
			case '{': eType = token_type::TOK_OPEN_CURLY; break;
			case '}': eType = token_type::TOK_CLOSE_CURLY; break;
			case '&':
				if (at[1] == '&')
				{
					eType = token_type::TOK_AND;
					pop();
				}
				else
				{
					// integer operations not supported currently
					eType = token_type::TOK_ERROR;
				}
				break;
			case '|':
				if (at[1] == '|')
				{
					eType = token_type::TOK_OR;
					pop();
				}
				else
				{
					// integer operations not supported currently
					eType = token_type::TOK_ERROR;
				}
				break;
			case '=':
			{
				if (at[1] == '=')
				{
					eType = token_type::TOK_EQUAL;
					pop();
				}
				else
					eType = token_type::TOK_ASSIGN;
				break;
			}
			case '!':
			{
				if (at[1] == '=')
				{
					eType = token_type::TOK_UNEQUAL;
					pop();
				}
				else
					eType = token_type::TOK_ERROR;
				break;
			}
			case '>':
			{
				if (at[1] == '=')
				{
					eType = token_type::TOK_GREATER_EQUAL;
					pop();
				}
				else
					eType = token_type::TOK_GREATER;
				break;
			}
			case '<':
			{
				if (at[1] == '=')
				{
					eType = token_type::TOK_LESS_EQUAL;
					pop();
				}
				else
					eType = token_type::TOK_LESS;
				break;
			}


			case ' ': case '\t': case '\n': case '\r': break;
			default: eType = token_type::TOK_ERROR; break;
			}
		}
		/// <summary>find a string in bools list</summary>
		const int find_bools(std::string& atName)
		{
			auto ps = std::find_if(pasBools->begin(), pasBools->end(),
				[atName](const ts_boolean& a) { return a.atName == atName; });
			if (ps == pasBools->end() || ps->atName != atName)
			{
				return -1;
			}
			else
			{
				std::ptrdiff_t nI = std::distance(pasBools->begin(), ps);
				return (int)nI;
			}
		}
		/// <summary>find a string in variables list</summary>
		const int find_vars(std::string& atName)
		{
			auto ps = std::find_if(pasVars->begin(), pasVars->end(),
				[atName](const ts_variable& a) { return a.m_name == atName; });
			if (ps == pasVars->end() || ps->m_name != atName)
			{
				return -1;
			}
			else
			{
				std::ptrdiff_t nI = std::distance(pasVars->begin(), ps);
				return (int)nI;
			}
		}

		/// <summary>the actual statement</summary>
		std::string atStatement;
		/// <summary>all variables used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_variable>> pasVars;
		/// <summary>all booleans used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_boolean>> pasBools;
		/// <summary>the current value</summary>
		std::variant<unsigned, te_type, te_type*> sValue;
		/// <summary>current token type</summary>
		token_type eType;
		/// <summary>current character index in the string</summary>
		size_t uNext = 0;
	};

	/// <summary>TinyExpr statement class</summary>
	class ts_statement_float_expr
	{
	public:
		ts_statement_float_expr() {}

		/// <summary>operator needed to assign</summary>
		ts_statement_float_expr& operator=(const ts_statement_float_expr& s)
		{
			this->pcTEP = s.pcTEP;
			this->atStatement = s.atStatement;
			this->pasVars = s.pasVars;
			this->uDestIx = s.uDestIx;
			this->nErr = s.nErr;
			return *this;
		}

		/// <param name="_atStatement">the statement string</param>
		/// <param name="_pasVars">shared pointer to the script variables</param>
		ts_statement_float_expr(std::string& _atStatement,
			std::shared_ptr<std::set<ts_variable>> _pasVars,
			unsigned _uDestIx
		)
			: atStatement(_atStatement)
			, uDestIx(_uDestIx)
			, pasVars(_pasVars)
		{
			if (_uDestIx >= (unsigned)_pasVars->size())
			{
				nErr = TS_FAIL;
				return;
			}

			// create TinyExpr parser
			pcTEP = std::make_shared<te_parser>();

			// convert and set variables
			std::set<ts_variable> as = *(_pasVars.get());
			std::set<te_variable> asTE;
			for (const ts_variable& s : as)
			{
				// name, value, type, context
				te_variable sTE = { s.m_name, s.m_value, s.m_type, nullptr };
				asTE.insert(sTE);
			}
			pcTEP->set_variables_and_functions(asTE);

			// compile
			if (!pcTEP->compile(_atStatement))
			{
				// error compiling
				nErr = pcTEP->get_last_error_position();
			}
		}
		/// <summary></summary>
		int64_t error() { return nErr; }
		/// <summary>evaluate compiled statement</summary>
		void evaluate()
		{
			if (pasVars->size() && !nErr)
			{
				te_type* pf = std::next(pasVars->begin(), uDestIx)->m_value;
				if (pf) *pf = pcTEP->evaluate();
			}
		}

	private:
		/// <summary>TinyExpr parser</summary>
		std::shared_ptr<te_parser> pcTEP;
		/// <summary>the actual statement</summary>
		std::string atStatement;
		/// <summary>all variables used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_variable>> pasVars;
		/// <summary>destination index in pasVars</summary>
		unsigned uDestIx = 0;
		/// <summary>0 if statement compiled</summary>
		int64_t nErr = TS_OK;
	};

	/// <summary>TinyScript boolean expression statement class</summary>
	class ts_statement_bool_expr
	{
	public:
		ts_statement_bool_expr() {}

		/// <summary>operator needed to assign</summary>
		ts_statement_bool_expr& operator=(const ts_statement_bool_expr& s)
		{
			this->asTermsCompiled = std::move(s.asTermsCompiled);
			this->aasEvaluationValues = std::vector<std::vector<term>>();
			aasEvaluationValues.resize(s.aasEvaluationValues.size());
			this->atStatement = s.atStatement;
			this->pasVars = s.pasVars;
			this->pasBools = s.pasBools;
			this->pbDestBool = s.pbDestBool;
			this->uDestIx = s.uDestIx;
			this->nErr = s.nErr;
			this->psState = s.psState;
			return *this;
		}

		/// <param name="_atStatement">the statement string</param>
		/// <param name="_pasVars">shared pointer to the script variables</param>
		ts_statement_bool_expr(std::string& _atStatement,
			std::shared_ptr<std::set<ts_variable>> _pasVars,
			std::shared_ptr<std::set<ts_boolean>> _pasBools,
			std::shared_ptr<bool> _pbDestBool,
			unsigned _uDestIx
		)
			: atStatement(_atStatement)
			, uDestIx(_uDestIx)
			, pasBools(_pasBools)
			, pasVars(_pasVars)
			, pbDestBool(_pbDestBool)
		{
			if ((_pbDestBool != nullptr) && (_uDestIx >= (unsigned)_pasBools->size()))
			{
				nErr = TS_FAIL;
				return;
			}

			// create state class
			psState = std::make_shared<state>(state(_atStatement, _pasVars, _pasBools));

			// start with level null and one values level
			unsigned uLevel = 0;
			aasEvaluationValues.resize(1);
			do
			{
				psState->next_token();
				switch (psState->get_type())
				{
				case ts_parser::state::token_type::TOK_OPEN:
					uLevel++;
					if (uLevel >= aasEvaluationValues.size())
						aasEvaluationValues.resize(uLevel + 1);
					break;
				case ts_parser::state::token_type::TOK_CLOSE:
					if (uLevel > 0)
						uLevel--;
					else
					{
						nErr = TS_FAIL;
						return;
					}
					break;
				case ts_parser::state::token_type::TOK_NULL:
				case ts_parser::state::token_type::TOK_ERROR:
				case ts_parser::state::token_type::TOK_OPEN_CURLY:
				case ts_parser::state::token_type::TOK_CLOSE_CURLY:
				case ts_parser::state::token_type::TOK_ASSIGN:
				case ts_parser::state::token_type::TOK_IF:
				case ts_parser::state::token_type::TOK_ELSE:
					nErr = TS_FAIL;
					return;

				case ts_parser::state::token_type::TOK_NUMBER:
				{
					// get the actual number
					te_type fValue = psState->value_floating();
					term_level s = { term_level_type::floating_const, uLevel, (te_type)fValue };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_TRUE:
				{
					term_level s = { term_level_type::boolean_const, uLevel, (bool)true };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_FALSE:
				{
					term_level s = { term_level_type::boolean_const, uLevel, (bool)false };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_VAR_FLOAT:
				{
					// get the variable index
					unsigned uIx = psState->value_unsigned();

					// return error if wrong index or type
					if (uIx >= _pasVars->size())
					{
						nErr = TS_FAIL;
						return;
					}

					term_level s = { term_level_type::floating, uLevel, (unsigned)uIx };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_VAR_BOOL:
				{
					// get the variable index
					unsigned uIx = psState->value_unsigned();

					// return error if wrong index or type
					if (uIx >= _pasBools->size())
					{
						nErr = TS_FAIL;
						return;
					}

					term_level s = { term_level_type::boolean, uLevel, (unsigned)uIx };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_OR:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_OR };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_AND:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_AND };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_UNEQUAL:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_UNEQUAL };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_GREATER:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_GREATER };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_LESS:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_LESS };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_GREATER_EQUAL:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_GREATER_EQUAL };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_LESS_EQUAL:
				{
					term_level s = { term_level_type::operative, uLevel, (state::token_compare_type)state::token_compare_type::TOK_LESS_EQUAL };
					asTermsCompiled.push_back(s);
				}
				break;
				case ts_parser::state::token_type::TOK_END:
				default:
					break;
				}
			} while (psState->get_type() != state::token_type::TOK_END);
		}

		/// <summary></summary>
		[[nodiscard]] int64_t error() { return nErr; }
		/// <summary>evaluate compiled statement</summary>
		void evaluate()
		{
			// clear evaluation level vectors
			for (auto& as : aasEvaluationValues)
				as.clear();

			if (pasVars->size() && !nErr)
			{
				// get destination bool
				bool* pb = (pbDestBool == nullptr) ? std::next(pasBools->begin(), uDestIx)->pbValue : pbDestBool.get();
				if (pb)
				{
					unsigned uLevel = 0;

					// evaluate.. loop through terms
					for (term_level& s : asTermsCompiled)
					{
						if (s.uLevel > uLevel)
							uLevel = s.uLevel;
						else if (s.uLevel < uLevel)
							level_down(uLevel, s.uLevel);

						address_term(s, uLevel);
					}

					// go down to level zero and set the value
					level_down(uLevel, 0);
					if ((aasEvaluationValues.size()) && (aasEvaluationValues[0].size()))
						*pb = get_bool(aasEvaluationValues[0][0]);
				}
			}
		}

	private:

		/// <summary>
		/// possible term types, operator or constant
		/// </summary>
		enum struct term_type : unsigned
		{
			floating_const,
			boolean_const,
			operative,
		};

		/// <summary>
		/// possible term types, operator or constant or variable
		/// </summary>
		enum struct term_level_type : unsigned
		{
			floating,
			floating_const,
			boolean,
			boolean_const,
			operative,
		};

		/// <summary>
		/// intermediate evaluation term
		/// </summary>
		struct term
		{
			/// <summary>the type of this term, operator or constant</summary>
			term_type eType;
			/// <summary>term value depending on type</summary>
			std::variant<state::token_compare_type, te_type, bool> sValue;
		};

		/// <summary>
		/// compiled term
		/// </summary>
		struct term_level
		{
			/// <summary>the type of this term, operator or constant or variable</summary>
			term_level_type eType;
			/// <summary>braces level of this term</summary>
			unsigned uLevel;
			/// <summary>term value depending on type</summary>
			std::variant<unsigned, state::token_compare_type, te_type, bool> sValue;
		};

		/// <summary>add a level to a term</summary>
		[[nodiscard]] term_level level_term(term& sT, unsigned uLevel)
		{
			switch (sT.eType)
			{
			case ts_parser::ts_statement_bool_expr::term_type::floating_const:
				return { term_level_type::floating_const, uLevel, (te_type)std::get<te_type>(sT.sValue) };
			case ts_parser::ts_statement_bool_expr::term_type::boolean_const:
				return { term_level_type::boolean_const, uLevel, (bool)std::get<bool>(sT.sValue) };
			case ts_parser::ts_statement_bool_expr::term_type::operative:
				return { term_level_type::operative, uLevel, (state::token_compare_type)std::get<state::token_compare_type>(sT.sValue) };
			default:
				break;
			}

			return {};
		}

		/// <summary>level down the evaluation value level</summary>
		void level_down(unsigned& uLevel, unsigned uTarget)
		{
			if (uLevel < aasEvaluationValues.size())
			{
				for (; uLevel > uTarget;)
				{
					if (aasEvaluationValues[uLevel].size())
					{
						term_level sT = level_term(aasEvaluationValues[uLevel].front(), uLevel - 1);
						aasEvaluationValues[uLevel].clear();
						address_term(sT, --uLevel);
					}
					else
						uLevel--;
				}
			}
		}

		/// <summary>handle this term</summary>
		void address_term(term_level& s, unsigned uLevel)
		{
			if (uLevel < aasEvaluationValues.size())
			{
				if (s.eType == term_level_type::operative)
				{
					term sTev = { term_type::operative, std::get<state::token_compare_type>(s.sValue) };
					aasEvaluationValues[uLevel].push_back(sTev);
				}
				else
				{
					if (aasEvaluationValues[uLevel].size())
					{
						if (aasEvaluationValues[uLevel].back().eType == term_type::operative)
						{
							state::token_compare_type eCType = std::get<state::token_compare_type>(aasEvaluationValues[uLevel].back().sValue);
							switch (eCType)
							{
							case ts_parser::state::token_compare_type::TOK_EQUAL:
							case ts_parser::state::token_compare_type::TOK_UNEQUAL:
							case ts_parser::state::token_compare_type::TOK_GREATER:
							case ts_parser::state::token_compare_type::TOK_LESS:
							case ts_parser::state::token_compare_type::TOK_GREATER_EQUAL:
							case ts_parser::state::token_compare_type::TOK_LESS_EQUAL:
							{
								// do actual bool compare
								std::array<std::variant<bool, te_type>, 2> as = { get_float(aasEvaluationValues[uLevel].front()), get_float(s) };
								aasEvaluationValues[uLevel] = { { term_type::boolean_const, compare(eCType, as) } };
							}
							break;
							case ts_parser::state::token_compare_type::TOK_AND:
							case ts_parser::state::token_compare_type::TOK_OR:
							{
								// do actual bool compare
								std::array<std::variant<bool, te_type>, 2> as = { get_bool(aasEvaluationValues[uLevel].front()), get_bool(s) };
								aasEvaluationValues[uLevel] = { { term_type::boolean_const, compare(eCType, as) } };
							}
							break;
							default:
								break;
							}
						}
					}
					else
					{
						term sTev = {};
						switch (s.eType)
						{
						case ts_parser::ts_statement_bool_expr::term_level_type::floating:
						case ts_parser::ts_statement_bool_expr::term_level_type::floating_const:
						{
							te_type fV = get_float(s);
							sTev = { term_type::floating_const, fV };
						}
						break;
						case ts_parser::ts_statement_bool_expr::term_level_type::boolean:
						case ts_parser::ts_statement_bool_expr::term_level_type::boolean_const:
						{
							bool bV = get_bool(s);
							sTev = { term_type::boolean_const, bV };
						}
						break;
						// shouldnt be possible to get here...
						case ts_parser::ts_statement_bool_expr::term_level_type::operative:
						default: break;
						}
						aasEvaluationValues[uLevel].push_back(sTev);
					}
				}
			}
		}

		/// <summary>get boolean value from term</summary>
		[[nodiscard]] bool get_bool(term& sTerm)
		{
			switch (sTerm.eType)
			{
			case ts_parser::ts_statement_bool_expr::term_type::floating_const:
			{
				te_type fV = std::get<te_type>(sTerm.sValue);
				return (fV != 0.f);
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_type::boolean_const:
				return std::get<bool>(sTerm.sValue);
			case ts_parser::ts_statement_bool_expr::term_type::operative:
			default: break;
			}
			return false;
		}

		/// <summary>get boolean value from term</summary>
		[[nodiscard]] bool get_bool(term_level& sTerm)
		{
			switch (sTerm.eType)
			{
			case ts_parser::ts_statement_bool_expr::term_level_type::floating:
			{
				unsigned uIx = std::get<unsigned>(sTerm.sValue);
				if (uIx < pasVars->size())
				{
					te_type* pf = std::next(pasVars->begin(), uIx)->m_value;
					if (pf) return ((*pf) != 0.f);
				}
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::floating_const:
			{
				te_type fV = std::get<te_type>(sTerm.sValue);
				return (fV != 0.f);
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::boolean:
			{
				unsigned uIx = std::get<unsigned>(sTerm.sValue);
				if (uIx < pasBools->size())
				{
					bool* pb = std::next(pasBools->begin(), uIx)->pbValue;
					if (pb) return *pb;
				}
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::boolean_const:
				return std::get<bool>(sTerm.sValue);
			case ts_parser::ts_statement_bool_expr::term_level_type::operative:
			default: break;
			}
			return false;
		}

		/// <summary>get floating value from term</summary>
		[[nodiscard]] float get_float(term& sTerm)
		{
			switch (sTerm.eType)
			{
			case ts_parser::ts_statement_bool_expr::term_type::floating_const:
			{
				te_type fV = std::get<te_type>(sTerm.sValue);
				return fV;
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_type::boolean_const:
				return (std::get<bool>(sTerm.sValue)) ? 1.f : 0.f;
			case ts_parser::ts_statement_bool_expr::term_type::operative:
			default: break;
			}
			return 0.f;
		}

		/// <summary>get floating value from term</summary>
		[[nodiscard]] float get_float(term_level& sTerm)
		{
			switch (sTerm.eType)
			{
			case ts_parser::ts_statement_bool_expr::term_level_type::floating:
			{
				unsigned uIx = std::get<unsigned>(sTerm.sValue);
				if (uIx < pasVars->size())
				{
					te_type* pf = std::next(pasVars->begin(), uIx)->m_value;
					if (pf) return *pf;
				}
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::floating_const:
			{
				te_type fV = std::get<te_type>(sTerm.sValue);
				return fV;
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::boolean:
			{
				unsigned uIx = std::get<unsigned>(sTerm.sValue);
				if (uIx < pasBools->size())
				{
					bool* pb = std::next(pasBools->begin(), uIx)->pbValue;
					if (pb) return (*pb) ? 1.f : 0.f;
				}
			}
			break;
			case ts_parser::ts_statement_bool_expr::term_level_type::boolean_const:
				return (std::get<bool>(sTerm.sValue)) ? 1.f : 0.f;
			case ts_parser::ts_statement_bool_expr::term_level_type::operative:
			default: break;
			}
			return 0.f;
		}

		/// <summary>compare two values</summary>
		[[nodiscard]] bool compare(state::token_compare_type eType, std::array<std::variant<bool, te_type>, 2>& asValue)
		{
			switch (eType)
			{
			case ts_parser::state::token_compare_type::TOK_EQUAL:
				return (std::get<te_type>(asValue[0]) == std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_UNEQUAL:
				return (std::get<te_type>(asValue[0]) != std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_GREATER:
				return (std::get<te_type>(asValue[0]) > std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_LESS:
				return (std::get<te_type>(asValue[0]) < std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_GREATER_EQUAL:
				return (std::get<te_type>(asValue[0]) >= std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_LESS_EQUAL:
				return (std::get<te_type>(asValue[0]) <= std::get<te_type>(asValue[1]));
			case ts_parser::state::token_compare_type::TOK_AND:
				return ((std::get<bool>(asValue[0])) && (std::get<bool>(asValue[1])));
			case ts_parser::state::token_compare_type::TOK_OR:
				return ((std::get<bool>(asValue[0])) || (std::get<bool>(asValue[1])));
			default:
				break;
			}
			return false;
		}

		/// <summary>the compiled boolean expression</summary>
		std::vector<term_level> asTermsCompiled = std::vector<term_level>();
		/// <summary>intermediate comparisation terms</summary>
		std::vector<std::vector<term>> aasEvaluationValues = std::vector<std::vector<term>>();
		/// <summary>the actual statement</summary>
		std::string atStatement;
		/// <summary>all variables used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_variable>> pasVars;
		/// <summary>all variables used within this script (type bool)</summary>
		std::shared_ptr<std::set<ts_boolean>> pasBools;
		/// <summary>destination boolean - if set uDestIx is ignored</summary>
		std::shared_ptr<bool> pbDestBool;
		/// <summary>the state with the embedded statement string</summary>
		std::shared_ptr<state> psState;
		/// <summary>destination index in pasBools</summary>
		unsigned uDestIx = 0;
		/// <summary>0 if statement compiled</summary>
		int64_t nErr = TS_OK;
	};

	/// <summary>TinyScript if statement class</summary>
	class ts_statement_if
	{
	public:
		ts_statement_if() {}

		/// <summary>operator needed to assign</summary>
		ts_statement_if& operator=(const ts_statement_if& s)
		{
			this->atBoolStatement = s.atBoolStatement;
			this->pasVars = s.pasVars;
			this->pasBools = s.pasBools;
			this->nErr = s.nErr;
			this->pbDestBool = s.pbDestBool;
			return *this;
		}

		/// <param name="_atStatement">the statement string</param>
		/// <param name="_pasVars">shared pointer to the script variables</param>
		ts_statement_if(std::string& _atBoolStatement,
			std::shared_ptr<std::set<ts_variable>> _pasVars,
			std::shared_ptr<std::set<ts_boolean>> _pasBools
		)
			: atBoolStatement(_atBoolStatement)
			, pasBools(_pasBools)
			, pasVars(_pasVars)
			, pbDestBool(std::make_shared<bool>(false))
		{
			cBoolExpr = ts_statement_bool_expr(_atBoolStatement, _pasVars, _pasBools, pbDestBool, 0);
		}
		/// <summary></summary>
		int64_t error() { return nErr; }
		/// <summary>evaluate compiled statement</summary>
		void evaluate()
		{
			cBoolExpr.evaluate();
		}
		/// <summary>return value of the boolean expression</summary>
		bool value()
		{
			return *(pbDestBool.get());
		}

	private:
		/// <summary>the boolean statement</summary>
		std::string atBoolStatement;
		/// <summary>all variables used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_variable>> pasVars;
		/// <summary>all variables used within this script (type bool)</summary>
		std::shared_ptr<std::set<ts_boolean>> pasBools;
		/// <summary>0 if statement compiled</summary>
		int64_t nErr = TS_OK;
		/// <summary>destination boolean pointer</summary>
		std::shared_ptr<bool> pbDestBool;
		/// <summary>embedded boolean expression</summary>
		ts_statement_bool_expr cBoolExpr;
	};

	/// <summary>a single code line within the script</summary>
	class ts_statement
	{
	public:
		/// <param name="_atStatement">the statement string</param>
		/// <param name="_pasVars">shared pointer to the script variables</param>
		/// <param name="_pasBools">shared pointer to the script booleans</param>
		explicit ts_statement(std::string& _atStatement,
			std::shared_ptr<std::set<ts_variable>> _pasVars,
			std::shared_ptr<std::set<ts_boolean>> _pasBools,
			unsigned _uBlockLevel
		) : uBlockLevel(_uBlockLevel)
		{
			// create state class
			psState = std::make_unique<state>(state(_atStatement, _pasVars, _pasBools));

			// get first token, create statement class
			psState->next_token();
			switch (psState->get_type())
			{
			case ts_parser::state::token_type::TOK_NULL:
			case ts_parser::state::token_type::TOK_ERROR:
			case ts_parser::state::token_type::TOK_END:
			case ts_parser::state::token_type::TOK_OPEN:
			case ts_parser::state::token_type::TOK_CLOSE:
			case ts_parser::state::token_type::TOK_OPEN_CURLY:
			case ts_parser::state::token_type::TOK_CLOSE_CURLY:
			case ts_parser::state::token_type::TOK_NUMBER:
			case ts_parser::state::token_type::TOK_ASSIGN:
			case ts_parser::state::token_type::TOK_EQUAL:
			case ts_parser::state::token_type::TOK_UNEQUAL:
			case ts_parser::state::token_type::TOK_GREATER:
			case ts_parser::state::token_type::TOK_LESS:
			case ts_parser::state::token_type::TOK_GREATER_EQUAL:
			case ts_parser::state::token_type::TOK_LESS_EQUAL:
				nErr = TS_FAIL;
				return;
			case ts_parser::state::token_type::TOK_IF:
			{
				// create if (in case boolean) statement
				std::string atS = psState->remaining();
				cStatement = ts_statement_if(atS, _pasVars, _pasBools);
				auto nE = std::get<ts_statement_if>(cStatement).error();
				if (nE == TS_OK)
					eType = ts_types::sm_if;
				else
					nErr = nE;
			}
			break;
			case ts_parser::state::token_type::TOK_ELSE:
				OutputDebugStringA("TOK_ELSE");
				break;
			case ts_parser::state::token_type::TOK_VAR_FLOAT:
			{
				// get the variable index
				unsigned uIx = psState->value_unsigned();

				// return error if wrong index
				if (uIx >= _pasVars->size())
				{
					nErr = TS_FAIL;
					return;
				}

				// next token must be TOK_ASSIGN
				psState->next_token();
				if (psState->get_type() != ts_parser::state::token_type::TOK_ASSIGN)
				{
					nErr = TS_FAIL;
					return;
				}

				// create TinyExpr statement
				std::string atS = psState->remaining();
				cStatement = ts_statement_float_expr(atS, _pasVars, uIx);
				auto nE = std::get<ts_statement_float_expr>(cStatement).error();
				if (nE == TS_OK)
					eType = ts_types::sm_expr_float;
				else
					nErr = nE;
			}
			break;
			case ts_parser::state::token_type::TOK_VAR_BOOL:
			{
				// get the variable index
				unsigned uIx = psState->value_unsigned();

				// return error if wrong index
				if (uIx >= _pasBools->size())
				{
					nErr = TS_FAIL;
					return;
				}

				// next token must be TOK_ASSIGN
				psState->next_token();
				if (psState->get_type() != ts_parser::state::token_type::TOK_ASSIGN)
				{
					nErr = TS_FAIL;
					return;
				}

				// create boolean statement
				std::string atS = psState->remaining();
				cStatement = ts_statement_bool_expr(atS, _pasVars, _pasBools, nullptr, uIx);
				auto nE = std::get<ts_statement_bool_expr>(cStatement).error();
				if (nE == TS_OK)
					eType = ts_types::sm_expr_bool;
				else
					nErr = nE;
			}
			break;
			default:
				break;
			}
		}
		/// <summary>evaluate statement based on type</summary>
		void evaluate()
		{
			if (eType == ts_types::sm_expr_float)
				std::get<ts_statement_float_expr>(cStatement).evaluate();
			else if (eType == ts_types::sm_expr_bool)
				std::get<ts_statement_bool_expr>(cStatement).evaluate();
			else if (eType == ts_types::sm_if)
				std::get<ts_statement_if>(cStatement).evaluate();
		}
		/// <summary></summary>
		ts_types type() { return eType; }
		/// <summary></summary>
		int64_t error() { return nErr; }
		/// <summary></summary>
		unsigned level() { return uBlockLevel; }
		/// <summary>return value for if statements</summary>
		bool boolean() { return (eType == ts_types::sm_if) ? std::get<ts_statement_if>(cStatement).value() : false; }

	private:
		/// <summary>type of the statement</summary>
		ts_types eType = ts_types::sm_undefined;
		/// <summary>the actual statement</summary>
		std::variant<ts_statement_bool_expr, ts_statement_float_expr, ts_statement_if> cStatement;
		/// <summary>the state with the embedded statement string</summary>
		std::unique_ptr<state> psState;
		/// <summary>all variables used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_variable>> pasVars;
		/// <summary>all booleans used within this script (type te_type : float or double)</summary>
		std::shared_ptr<std::set<ts_boolean>> pasBools;
		/// <summary>the block level of this statement</summary>
		unsigned uBlockLevel;
		/// <summary>0 if statement compiled</summary>
		int64_t nErr = TS_OK;
	};

	/// <summary>block level helper</summary>
	unsigned block_level_up() { if (++uBlockLevel >= (unsigned)aeFlags.size()) aeFlags.resize(uBlockLevel + 1); return uBlockLevel; }
	/// <summary>block level helper</summary>
	unsigned block_level_down() { return --uBlockLevel; }

	/// <summary>the unmodified script</summary>
	std::string atScript;
	/// <summary>the compiled script statements</summary>
	std::vector<std::shared_ptr<ts_statement>> apsStatements_compiled;
	/// <summary>flags for each block level used during evaluation process	</summary>
	std::vector<ts_runtime_flags> aeFlags;
	/// <summary>all variables used within this script (type te_type : float or double)</summary>
	std::shared_ptr<std::set<ts_variable>> pasVars;
	/// <summary>all booleans used within this script (type te_type : float or double)</summary>
	std::shared_ptr<std::set<ts_boolean>> pasBools;
	/// <summary>the code block level in opened braces</summary>
	unsigned uBlockLevel = 0;
	/// <summary>0 if script compiled</summary>
	int64_t nErr = TS_OK;
};

#endif /// __TINYSCRIPT_PLUS_PLUS_H__
