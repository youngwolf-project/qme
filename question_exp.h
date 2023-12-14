
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <map>

namespace qme
{

class exp
{
public:
	virtual ~exp() {}

	virtual bool is_data() const {return false;}
	virtual bool is_judge() const {return false;}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T = float> class data_exp : public exp
{
public:
	virtual bool is_data() const {return true;}
	virtual bool is_immediate() const {return false;}
	virtual T get_immediate_value() const {return 0;}
	virtual bool merge_immediate_value(const std::shared_ptr<data_exp<T>>&, char op) {return false;}

	virtual T operator()(const std::map<std::string, T>&) const = 0;
};
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T = float> class binary_data_exp : public data_exp<T>
{
public:
	binary_data_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		dexp_l(_dexp_l), dexp_r(_dexp_r) {}

	virtual bool merge_immediate_value(const std::shared_ptr<data_exp<T>>& other_exp, char op)
		{return dexp_l->merge_immediate_value(other_exp, op) || dexp_r->merge_immediate_value(other_exp, op);}

protected:
	std::shared_ptr<data_exp<T>> dexp_l, dexp_r;
};

template <typename T = float> class add_data_exp : public binary_data_exp<T>
{
public:
	add_data_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_data_exp<T>(_dexp_l, _dexp_r) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) + (*this->dexp_r)(data_map);}
};

template <typename T = float> class sub_data_exp : public binary_data_exp<T>
{
public:
	sub_data_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_data_exp<T>(_dexp_l, _dexp_r) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) - (*this->dexp_r)(data_map);}
};

template <typename T = float> class multi_data_exp : public binary_data_exp<T>
{
public:
	multi_data_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_data_exp<T>(_dexp_l, _dexp_r) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) * (*this->dexp_r)(data_map);}
};

template <typename T = float> class div_data_exp : public binary_data_exp<T>
{
public:
	div_data_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_data_exp<T>(_dexp_l, _dexp_r) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) / (*this->dexp_r)(data_map);}
};

template <typename T = float> class immediate_data_exp : public data_exp<T>
{
public:
	immediate_data_exp() : T(0) {}
	immediate_data_exp(T v) : value(v) {}

	virtual bool is_immediate() const {return true;}
	virtual T get_immediate_value() const {return value;}
	virtual bool merge_immediate_value(const std::shared_ptr<data_exp<T>>& other_exp, char op)
	{
		if (!other_exp->is_immediate())
			return false;

		switch (op)
		{
		case '+':
			value += other_exp->get_immediate_value();
			break;
		case '-':
			value -= other_exp->get_immediate_value();
			break;
		case '*':
			value *= other_exp->get_immediate_value();
			break;
		case '/':
			value /= other_exp->get_immediate_value();
			break;
		default:
			throw(("undefined operator " + std::string(1, op)).data());
			break;
		}
		return true;
	}

	virtual T operator()(const std::map<std::string, T>&) const {return value;}

private:
	T value;
};

template <typename T = float> class negative_data_exp : public data_exp<T>
{
public:
	negative_data_exp(const std::shared_ptr<data_exp<T>>& _dexp) : dexp(_dexp) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return -(*dexp)(data_map);}

protected:
	std::shared_ptr<data_exp<T>> dexp;
};

template <typename T = float> class variable_data_exp : public data_exp<T>
{
public:
	variable_data_exp() {}
	variable_data_exp(const std::string& _variable_name) : variable_name(_variable_name) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const
	{
		typename std::map<std::string, T>::const_iterator iter = data_map.find(variable_name);
		if (iter != data_map.end())
		{
			std::cout << "get " << variable_name << " returns " << iter->second << std::endl;
			return iter->second;
		}

		throw(("undefined symbol " + variable_name).data());
	}

private:
	std::string variable_name;
};

template <typename T = float>
inline std::shared_ptr<data_exp<T>> merge_data_exp(const std::shared_ptr<data_exp<T>>& exp_l, const std::shared_ptr<data_exp<T>>& exp_r, const std::string& op)
{
	switch (op[0])
	{
	case '+':
		return std::make_shared<add_data_exp<T>>(exp_l, exp_r);
		break;
	case '-':
		return std::make_shared<sub_data_exp<T>>(exp_l, exp_r);
		break;
	case '*':
		return std::make_shared<multi_data_exp<T>>(exp_l, exp_r);
		break;
	case '/':
		return std::make_shared<div_data_exp<T>>(exp_l, exp_r);
		break;
	default:
		throw(("undefined operator " + op).data());
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T = float> class judge_exp : public exp
{
public:
	virtual bool is_judge() const {return true;}
	virtual std::shared_ptr<data_exp<T>> get_data_exp() const {return std::shared_ptr<data_exp<T>>();}

	virtual bool operator()(const std::map<std::string, T>&) const = 0;
};

template <typename T = float> class not_judge_exp : public judge_exp<T>
{
public:
	not_judge_exp(const std::shared_ptr<judge_exp<T>>& _jexp) : jexp(_jexp) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return !(*jexp)(data_map);}

private:
	std::shared_ptr<judge_exp<T>> jexp;
};

template <typename T = float> class equal_0_judge_exp : public judge_exp<T>
{
public:
	equal_0_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp) : dexp(_dexp) {}

	virtual std::shared_ptr<data_exp<T>> get_data_exp() const {return dexp;}
	virtual bool operator()(const std::map<std::string, T>& data_map) const {return 0 == (*dexp)(data_map);}

private:
	std::shared_ptr<data_exp<T>> dexp;
};

template <typename T = float> class not_equal_0_judge_exp : public judge_exp<T>
{
public:
	not_equal_0_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp) : dexp(_dexp) {}

	virtual std::shared_ptr<data_exp<T>> get_data_exp() const {return dexp;}
	virtual bool operator()(const std::map<std::string, T>& data_map) const {return 0 != (*dexp)(data_map);}

private:
	std::shared_ptr<data_exp<T>> dexp;
};

template <typename T = float> class binary_judge_exp : public judge_exp<T>
{
public:
	binary_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		dexp_l(_dexp_l), dexp_r(_dexp_r) {}

protected:
	std::shared_ptr<data_exp<T>> dexp_l, dexp_r;
};

template <typename T = float> class bigger_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) > (*this->dexp_r)(data_map);}
};

template <typename T = float> class bigger_equal_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_equal_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) >= (*this->dexp_r)(data_map);}
};

template <typename T = float> class smaller_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) < (*this->dexp_r)(data_map);}
};

template <typename T = float> class smaller_equal_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_equal_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) <= (*this->dexp_r)(data_map);}
};

template <typename T = float> class equal_judge_exp : public binary_judge_exp<T>
{
public:
	equal_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) == (*this->dexp_r)(data_map);}
};

template <typename T = float> class not_equal_judge_exp : public binary_judge_exp<T>
{
public:
	not_equal_judge_exp(const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_judge_exp<T>(_dexp_l, _dexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*this->dexp_l)(data_map) != (*this->dexp_r)(data_map);}
};

template <typename T = float> class and_judge_exp : public judge_exp<T>
{
public:
	and_judge_exp(const std::shared_ptr<judge_exp<T>>& _jexp_l, const std::shared_ptr<judge_exp<T>>& _jexp_r) :
		jexp_l(_jexp_l), jexp_r(_jexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*jexp_l)(data_map) && (*jexp_r)(data_map);}

private:
	std::shared_ptr<judge_exp<T>> jexp_l;
	std::shared_ptr<judge_exp<T>> jexp_r;
};

template <typename T = float> class or_judge_exp : public judge_exp<T>
{
public:
	or_judge_exp(const std::shared_ptr<judge_exp<T>>& _jexp_l, const std::shared_ptr<judge_exp<T>>& _jexp_r) :
		jexp_l(_jexp_l), jexp_r(_jexp_r) {}

	virtual bool operator()(const std::map<std::string, T>& data_map) const {return (*jexp_l)(data_map) || (*jexp_r)(data_map);}

private:
	std::shared_ptr<judge_exp<T>> jexp_l;
	std::shared_ptr<judge_exp<T>> jexp_r;
};

template <typename T = float>
inline std::shared_ptr<judge_exp<T>> merge_judge_exp(const std::shared_ptr<judge_exp<T>>& jexp_l, const std::shared_ptr<judge_exp<T>>& jexp_r, const std::string& lop)
{
	if ("&&" == lop)
		return std::make_shared<and_judge_exp<T>>(jexp_l, jexp_r);
	else if ("||" == lop)
		return std::make_shared<or_judge_exp<T>>(jexp_l, jexp_r);
	else
		throw(("undefined logical operator " + lop).data());
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T = float> class question_exp : public binary_data_exp<T>
{
public:
	question_exp(const std::shared_ptr<judge_exp<T>>& _jexp, const std::shared_ptr<data_exp<T>>& _dexp_l, const std::shared_ptr<data_exp<T>>& _dexp_r) :
		binary_data_exp<T>(_dexp_l, _dexp_r), jexp(_jexp) {}

	virtual T operator()(const std::map<std::string, T>& data_map) const {return (*jexp)(data_map) ? (*this->dexp_l)(data_map) : (*this->dexp_r)(data_map);}

private:
	std::shared_ptr<judge_exp<T>> jexp;
};
/////////////////////////////////////////////////////////////////////////////////////////

template <typename T = float>
class question_exp_parser
{
private:
	struct sub_exp
	{
		std::string name, raw_exp;
		std::vector<std::string> items;
		std::shared_ptr<exp> parsed_exp;
	};

public:
	static bool is_operator_1(const char* input) {return '+' == *input || '-' == *input;}
	static bool is_operator_1(const std::string& input) {return is_operator_1(input.data());}
	static bool is_operator_2(const char* input) {return '*' == *input || '/' == *input;}
	static bool is_operator_2(const std::string& input) {return is_operator_2(input.data());}
	static bool is_operator(const char* input) {return is_operator_1(input) || is_operator_2(input);}
	static bool is_operator(const std::string& input) {return is_operator(input.data());}

	static bool is_comparer(const char* input)
	{
		return '>' == *input || '<' == *input ||
			0 == strncmp(input, ">=", 2) || 0 == strncmp(input, "<=", 2) ||
			0 == strncmp(input, "==", 2) || 0 == strncmp(input, "!=", 2);
	}
	static bool is_comparer(const std::string& input) {return is_comparer(input.data());}

	static bool is_logical_operator(const char* input) {return 0 == strncmp(input, "&&", 2) || 0 == strncmp(input, "||", 2);}
	static bool is_logical_operator(const std::string& input) {return is_logical_operator(input.data());}

	static bool is_key_1(const char* input)
	{
		return '!' == *input || '(' == *input || ')' == *input || is_operator(input) ||
			'>' == *input || '<' == *input || '?' == *input || ':' == *input;
	}
	static bool is_key_1(const std::string& input) {return is_key_1(input.data());}

	static bool is_key_2(const char* input)
	{
		return 0 == strncmp(input, ">=", 2) || 0 == strncmp(input, "<=", 2) ||
			0 == strncmp(input, "==", 2) || 0 == strncmp(input, "!=", 2) || is_logical_operator(input);
	}
	static bool is_key_2(const std::string& input) {return is_key_2(input.data());}

	static std::shared_ptr<data_exp<T>> parse(const char* statement) {return parse(std::string(statement));}
	static std::shared_ptr<data_exp<T>> parse(const std::string& statement)
	{
		try
		{
			auto expression = statement;
			pre_parse_1(expression);

			auto old_exp = expression;
			std::map<std::string, sub_exp> sub_exps;

			size_t index = 0;
			try
			{
				auto end_index = expression.size();
				auto num = 0;
				pre_parse_2(expression, sub_exps, index, end_index, num, 0);

				auto base_level = 100;
				while (true)
				{
					index = 0;
					end_index = expression.size();
					num = 0;
					if (!pre_parse_3(expression, sub_exps, index, end_index, num, 0, base_level))
						break;

					base_level += 100;
				}
		
				auto tmp_exps = sub_exps;
				for (auto& s_exp : tmp_exps)
					while (true)
					{
						index = 0;
						end_index = s_exp.second.raw_exp.size();
						num = 0;
						if (!pre_parse_3(s_exp.second.raw_exp, sub_exps, index, end_index, num, 0, base_level))
							break;

						base_level += 100;
						sub_exps[s_exp.first].raw_exp = s_exp.second.raw_exp;
					}
			}
			catch (const std::exception& e) {on_error(expression, index); throw(e);}
			catch (const std::string& e) {on_error(expression, index); throw(e);}
			catch (const char* e) {on_error(expression, index); throw(e);}

			if (!sub_exps.empty())
			{
				printf("get %s from %s, where:\n", expression.data(), old_exp.data());
				for (auto& item : sub_exps)
				{
					printf("%s = %s\n", item.first.data(), item.second.raw_exp.data());
					item.second.items = split(item.second.raw_exp);
				}

				while(true)
				{
					auto parsed_num = 0;
					for (auto& item : sub_exps)
					{
						if (!item.second.parsed_exp)
							if ((item.second.parsed_exp = parse(item.second.items, sub_exps)))
								++parsed_num;
					}

					if (0 == parsed_num)
						break;
				}
			}

			auto items = split(expression);
			auto re = parse(items, sub_exps);
			if (!re || !re->is_data())
				throw("incomplete judgement!");

			return std::dynamic_pointer_cast<data_exp<T>>(re);
		}
		catch (const std::exception& e) {puts(e.what());}
		catch (const std::string& e) {puts(e.data());}
		catch (const char* e) {puts(e);}
		catch (...) {puts("unknown exception happened!");}

		return std::shared_ptr<data_exp<T>>();
	}

private:
	static void pre_parse_1(std::string& expression)
	{
		printf("pre-parsing phase 1 from [%s] ", expression.data());

		char blanks[] = {' ', '\t', '\n', '\r'};
		for (auto& c : blanks)
		{
			auto pos = std::string::npos;
			while (true)
			{
				pos = expression.rfind(c, pos);
				if (std::string::npos == pos)
					break;

				expression.erase(pos, 1);
			}
		}

		printf("to [%s]\n", expression.data());
	}

	static void pre_parse_2(std::string& expression, std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t& end_index, int& p_num, int level)
	{
		if (level >= 100)
			throw("parentheses layer cannot exceed 99!");

		auto p_start = std::string::npos;
		while (index < end_index)
		{
			if ('(' == expression[index])
			{
				if (std::string::npos == p_start)
				{
					p_start = index;
					++p_num;
				}
				else
					pre_parse_2(expression, sub_exps, index, end_index, p_num, level + 1);
			}
			else if (')' == expression[index])
			{
				if (std::string::npos == p_start)
				{
					--index;
					return;
				}
				else if (index <= p_start + 1)
					throw("empty parentheses!");

				char buf[32];
				std::string name(buf, snprintf(buf, sizeof(buf), "$%d_%d", level, p_num));
				std::string raw_exp = std::string(expression.data() + p_start + 1, index - p_start - 1);
				sub_exps[name] = sub_exp {name, raw_exp};

				auto old_size = expression.size();
				printf("pre-parsing phase 2 from [%s] ", expression.data());
				expression.replace(p_start, raw_exp.size() + 2, name);
				printf("to [%s]\n", expression.data());

				auto size_change = old_size - expression.size();
				index -= size_change;
				end_index -= size_change;

				p_start = std::string::npos;
			}

			++index;
		}

		--index;
		if (std::string::npos != p_start)
			throw("parentheses not match!");
	}

	static bool pre_parse_3(std::string& expression, std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t& end_index, int& q_num, int level, int base_level)
	{
		if (level >= 100)
			throw("question layer cannot exceed 99!");

		auto changed = false;
		size_t q_pos = 0, c_pos = 0, start = index;
		while (index < end_index)
		{
			if ('?' == expression[index])
			{
				if (0 == q_pos)
				{
					if (0 == index)
						throw("missing judgement!");
					else if (0 != c_pos)
						throw(": cannot appears before ?");

					q_pos = index;
					++q_num;
				}
				else
				{
					index = 1 + (c_pos > 0 ? c_pos : q_pos);
					if (pre_parse_3(expression, sub_exps, index, end_index, q_num, level + 1, base_level))
						changed = true;
				}
			}
			else if (':' == expression[index] || index + 1 == end_index)
			{
				auto is_c = ':' == expression[index];
				if (!is_c)
					++index;

				if (0 == c_pos)
				{
					if (0 == index)
						throw("missing judgement!");

					c_pos = index;
				}
				else if (level > 0)
				{
					char buf[32];
					std::string name(buf, snprintf(buf, sizeof(buf), "$%d_%d", level + base_level, q_num));
					std::string raw_exp = std::string(std::next(expression.data(), start), index - start);
					sub_exps[name] = sub_exp {name, raw_exp};

					auto old_size = expression.size();
					printf("pre-parsing phase 3 from [%s] ", expression.data());
					expression.replace(start, index - start, name);
					printf("to [%s]\n", expression.data());

					auto size_change = old_size - expression.size();
					index -= size_change;
					end_index -= size_change;
					--index;

					return true;
				}
			}

			++index;
		}

		return changed;
	}

	static std::vector<std::string> split(const std::string& expression)
	{
		std::vector<std::string> items;

		auto input = expression.data();
		auto start = input;
		for (; '\0' != *input; ++input)
			if (is_key_2(input))
			{
				if (start < input)
					items.push_back(std::string(start, input - start));

				items.push_back(std::string(input++, 2));
				start = input + 1;
			}
			else if (is_key_1(input))
			{
				if (start < input)
					items.push_back(std::string(start, input - start));

				items.push_back(std::string(input, 1));
				start = input + 1;
			}
		if (start < input)
			items.push_back(std::string(start, input - start));

		return items;
	}

	static void finish_data_exp(std::shared_ptr<data_exp<T>>& data_1, std::shared_ptr<data_exp<T>>& data_2,
		std::string& op_1, std::string& op_2)
	{
		if (!op_2.empty())
			throw("missing operand!");
		else if (!data_1)
			throw("missing operand!");
		else if (data_2)
			data_1 = merge_data_exp(data_1, data_2, op_1);
		else if (!op_1.empty())
			throw("missing operand!");

		op_1.clear();
		data_2.reset();
	}

	static void finish_data_exp(std::shared_ptr<data_exp<T>>& data_1, std::shared_ptr<data_exp<T>>& data_2,
		std::string& op_1, std::string& op_2, std::shared_ptr<data_exp<T>>& dc)
	{
		finish_data_exp(data_1, data_2, op_1, op_2);
		dc = data_1;
		data_1.reset();
	}

	static void merge_judge_exp(std::shared_ptr<judge_exp<T>>& judge_1, std::shared_ptr<judge_exp<T>>& judge_2,
		std::string& lop_1, std::string& lop_2, const std::shared_ptr<judge_exp<T>>& judge)
	{
		if (judge_2)
		{
			if (lop_2.empty())
				throw("missing logical operator!");

			judge_2 = qme::merge_judge_exp(judge_2, judge, lop_2);
			lop_2.clear();
		}
		else if (judge_1)
		{
			if (lop_1.empty())
				throw("missing logical operator!");

			if ("||" == lop_1)
				judge_2 = judge;
			else
			{
				judge_1 = qme::merge_judge_exp(judge_1, judge, lop_1);
				lop_1.clear();
			}
		}
		else
			judge_1 = judge;
	}

	static void finish_data_and_merge_judge_exp(
		std::shared_ptr<data_exp<T>>& data_1, std::shared_ptr<data_exp<T>>& data_2, std::string& op_1, std::string& op_2,
		std::shared_ptr<data_exp<T>>& dc, std::string& c,
		std::shared_ptr<judge_exp<T>>& judge_1, std::shared_ptr<judge_exp<T>>& judge_2, std::string& lop_1, std::string& lop_2)
	{
		if (!c.empty())
		{
			std::shared_ptr<data_exp<T>> dc_2;
			finish_data_exp(data_1, data_2, op_1, op_2, dc_2);
			merge_judge_exp(judge_1, judge_2, lop_1, lop_2, make_judge_exp(dc, dc_2, c));
			dc.reset();
			c.clear();
		}
	}

	static std::shared_ptr<judge_exp<T>> make_judge_exp(const std::shared_ptr<data_exp<T>>& dc_1,
		const std::shared_ptr<data_exp<T>>& dc_2, const std::string& c)
	{
		if (">" == c)
			return std::make_shared<bigger_judge_exp<T>>(dc_1, dc_2);
		else if (">=" == c)
			return std::make_shared<bigger_equal_judge_exp<T>>(dc_1, dc_2);
		else if ("<" == c)
			return std::make_shared<smaller_judge_exp<T>>(dc_1, dc_2);
		else if ("<=" == c)
			return std::make_shared<smaller_equal_judge_exp<T>>(dc_1, dc_2);
		else if ("==" == c)
			return std::make_shared<equal_judge_exp<T>>(dc_1, dc_2);
		else if ("!=" == c)
			return std::make_shared<not_equal_judge_exp<T>>(dc_1, dc_2);
		else
			throw("unknown compare operator " + c);
	}

	static void finish_judge_exp(std::shared_ptr<judge_exp<T>>& judge_1, std::shared_ptr<judge_exp<T>>& judge_2,
		std::string& lop_1, std::string& lop_2)
	{
		if (!lop_2.empty())
			throw("missing logical operand!");
		else if (!judge_1)
			throw("missing logical operand!");
		else if (judge_2)
		{
			judge_1 = qme::merge_judge_exp(judge_1, judge_2, lop_1);
			lop_1.clear();
			judge_2.reset();
		}
		else if (!lop_1.empty())
			throw("missing logical operand!");
	}

	static std::shared_ptr<exp> parse(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps)
	{
		size_t index = 0, end_index = items.size();
		try {return parse(items, sub_exps, index, end_index);}
		catch (const std::exception& e) {on_error(items, index); throw(e);}
		catch (const std::string& e) {on_error(items, index); throw(e);}
		catch (const char* e) {on_error(items, index); throw(e);}
	}

	static std::shared_ptr<data_exp<T>> parse_data(const std::string& vov)
	{
		if (is_key_2(vov) || is_key_1(vov))
			throw("unexpected " + vov);
		else if (0 == isdigit(vov[0]))
			return std::make_shared<variable_data_exp<T>>(vov); //variable
		else if (std::string::npos != vov.find('.') || std::string::npos != vov.find('e') || std::string::npos != vov.find('E'))
			//todo, verify float data
			return std::make_shared<immediate_data_exp<T>>((T) atof(vov.data())); //float
		else
			//todo, verify integer data
			return std::make_shared<immediate_data_exp<T>>((T) atoll(vov.data())); //integer
	}

	static std::shared_ptr<exp> parse(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t end_index)
	{
		if (index >= end_index)
			throw("empty expresson!");

		auto revert = 0;
		auto negative = false, is_q = false, is_c = false;
		std::shared_ptr<data_exp<T>> data_1, data_2;
		std::string op_1, op_2;
		std::shared_ptr<judge_exp<T>> judge_1, judge_2;
		std::string lop_1, lop_2;
		std::shared_ptr<data_exp<T>> dc;
		std::string c;
		std::shared_ptr<data_exp<T>> fd_1, fd_2;

		while (index < end_index)
		{
			if ("!" == items[index])
			{
				if (is_q || is_c)
					throw("only arithmetic operator can appears after ? or : operator!");

				++revert;
				++index;
				continue;
			}
			else if (is_operator(items[index]))
			{
				if (data_2)
				{
					if (!op_2.empty())
						throw("redundant operator!");

					if (is_operator_1(items[index]))
					{
						data_1 = merge_data_exp(data_1, data_2, op_1);
						op_1 = items[index];
						data_2.reset();
					}
					else
						op_2 = items[index];
				}
				else if (data_1)
				{
					if (!op_1.empty())
						throw("redundant operator!");

					op_1 = items[index];
				}
				else if ("-" == items[index])
				{
					if (negative)
						throw("redundant - operator!");
					else
					{
						negative = true;
						++index;
						continue;
					}
				}
				else
					throw("missing operand!");
			}
			else if (is_comparer(items[index]))
			{
				if (is_q || is_c)
					throw("only arithmetic operator can appears after ? or : operator!");
				else if (!c.empty() || dc)
					throw("redundant comparer!");

				c = items[index];
				finish_data_exp(data_1, data_2, op_1, op_2, dc);
			}
			else if (is_logical_operator(items[index]))
			{
				if (is_q || is_c)
					throw("only arithmetic operator can appears after ? or : operator!");

				finish_data_and_merge_judge_exp(data_1, data_2, op_1, op_2, dc, c, judge_1, judge_2, lop_1, lop_2);
				if (data_1 || data_2)
				{
					finish_data_exp(data_1, data_2, op_1, op_2);
					merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::make_shared<not_equal_0_judge_exp<T>>(data_1));
					data_1.reset();
				}

				if (judge_2)
				{
					if (!lop_2.empty())
						throw("redundant logical operator!");

					if ("||" == items[index])
					{
						judge_1 = qme::merge_judge_exp(judge_1, judge_2, lop_1);
						lop_1 = items[index];
						judge_2.reset();
					}
					else
						lop_2 = items[index];
				}
				else if (judge_1)
				{
					if (!lop_1.empty())
						throw("redundant logical operator!");

					lop_1 = items[index];
				}
				else
					throw("missing logical operand!");
			}
			else if ("?" == items[index])
			{
				if (is_q)
					throw("redundant ? operator!");
				else
					is_q = true;

				finish_data_and_merge_judge_exp(data_1, data_2, op_1, op_2, dc, c, judge_1, judge_2, lop_1, lop_2);
				if (data_1 || data_2)
				{
					finish_data_exp(data_1, data_2, op_1, op_2);
					merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::make_shared<not_equal_0_judge_exp<T>>(data_1));
					data_1.reset();
				}

				finish_judge_exp(judge_1, judge_2, lop_1, lop_2);
			}
			else if (":" == items[index])
			{
				if (!is_q)
					throw("missing ? operator!");
				else if (is_c)
					throw("redundant : operator!");
				else
					is_c = true;

				finish_data_exp(data_1, data_2, op_1, op_2);
				fd_1 = data_1;
				data_1.reset();
			}
			else
			{
				std::shared_ptr<exp> parsed_exp;
				if ('$' == items[index][0])
				{
					auto iter = sub_exps.find(items[index]);
					if (iter == std::end(sub_exps))
						throw("undefined symbol " + items[index]);
					else if (!iter->second.parsed_exp) //dependancy not ready
						return std::shared_ptr<exp>();
					else
						parsed_exp = iter->second.parsed_exp;
				}
				else
					parsed_exp = parse_data(items[index]);

				if (revert > 0)
				{
					if (parsed_exp->is_data())
					{
						auto data = std::dynamic_pointer_cast<data_exp<T>>(parsed_exp);
						if (1 == (revert & 1))
							parsed_exp = std::make_shared<equal_0_judge_exp<T>>(data);
						else
							parsed_exp = std::make_shared<not_equal_0_judge_exp<T>>(data);
					}
					else if (revert)
						parsed_exp = std::make_shared<not_judge_exp<T>>(std::dynamic_pointer_cast<judge_exp<T>>(parsed_exp));

					revert = 0;
				}

				if (parsed_exp->is_data())
				{
					auto data = std::dynamic_pointer_cast<data_exp<T>>(parsed_exp);
					if (negative)
					{
						negative = false;
						data = std::make_shared<negative_data_exp<T>>(data);
					}

					if (data_2)
					{
						if (op_2.empty())
							throw("missing operator!");

						data_2 = merge_data_exp(data_2, data, op_2);
						op_2.clear();
					}
					else if (data_1)
					{
						if (op_1.empty())
							throw("missing operator!");

						if (is_operator_1(op_1))
							data_2 = data;
						else
						{
							data_1 = merge_data_exp(data_1, data, op_1);
							op_1.clear();
						}
					}
					else
						data_1 = data;
				}
				else if (is_q || is_c)
					throw("only arithmetic operator can appears after ? or : operator!");
				else
					merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::dynamic_pointer_cast<judge_exp<T>>(parsed_exp));
			}

			if (revert > 0)
				throw("unexpected ! operator!");
			else if (negative)
				throw("unexpected - operator!");

			++index;
		}

		if (!c.empty())
			finish_data_and_merge_judge_exp(data_1, data_2, op_1, op_2, dc, c, judge_1, judge_2, lop_1, lop_2);
		else if (data_1)
		{
			finish_data_exp(data_1, data_2, op_1, op_2);
			if (is_c)
			{
				fd_2 = data_1;
				data_1.reset();
			}
		}

		if (judge_1)
		{
			if (data_1 || data_2)
				throw("incomplete question exp!");

			finish_judge_exp(judge_1, judge_2, lop_1, lop_2);
			if (fd_1)
			{
				if (!fd_2)
					throw("missing operand!");

				return std::make_shared<question_exp<T>>(judge_1, fd_1, fd_2);
			}
			else
				return judge_1;
		}
		else if (data_1)
			return data_1;
		else
			throw("incomplete exp!");
	}

	static void on_error(const std::string& expression, size_t index)
	{
		puts("failed to parse the statement:");
		for (size_t i = 0; i < expression.size(); ++i)
			if (i + 1 == index || i == index || i == index + 1)
				printf("\033[31m%c\033[0m", expression[i]);
			else
				printf("%c", expression[i]);

		putchar('\n');
	}

	static void on_error(const std::vector<std::string>& items, size_t index)
	{
		puts("failed to parse the statement:");
		for (size_t i = 0; i < items.size(); ++i)
			if (i + 1 == index || i == index || i == index + 1)
				printf("\033[31m%s\033[0m", items[i].data());
			else
				printf("%s", items[i].data());

		putchar('\n');
	}
};

}
