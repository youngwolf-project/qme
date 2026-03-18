
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <type_traits>
#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <map>

#if defined(_MSC_VER) && defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

namespace qme
{

//without any exchange between any data, nor merging of immediate values.
class O0 {public: static int level() {return 0;}};

//without any exchange between any data, but merge adjacent immediate values.
class O1 {public: static int level() {return 1;}};

//don't exchange the data order for divide operations, nor transform them to multiply operations, for example:
// 'b / (b / 3)' will not be transformed to '3', because the latter never triggers divide zero for integer (1 ~ 8 bytes)
// '2 * a / 3' will not be transformed to '(2 / 3) * a', because the latter will always be zero for integer (1 ~ 8 bytes)
// 'a * 2 / 3' will not be transformed to 'a * (2 / 3)', because the latter will always be zero for integer (1 ~ 8 bytes)
// 'a / 2 * 3' will not be transformed to '(3 / 2) * a'
// '2 / a * 3' will not be transformed to '(2 * 3) / a'
class O2 {public: static int level() {return 2;}};

//full optimization
class O3 {public: static int level() {return 3;}};

/////////////////////////////////////////////////////////////////////////////////////////
inline bool is_operator_1(char input) {return '+' == input || '-' == input;}
inline bool is_operator_1(const char* input) {return is_operator_1(*input);}
inline bool is_operator_1(const std::string& input) {return is_operator_1(input.data());}
inline bool is_operator_2(char input) {return '*' == input || '/' == input;}
inline bool is_operator_2(const char* input) {return is_operator_2(*input);}
inline bool is_operator_2(const std::string& input) {return is_operator_2(input.data());}
inline bool is_operator(char input) {return is_operator_1(input) || is_operator_2(input);}
inline bool is_operator(const char* input) {return is_operator(*input);}
inline bool is_operator(const std::string& input) {return is_operator(input.data());}
inline bool is_logical_operator(const char* input) {return 0 == strncmp(input, "&&", 2) || 0 == strncmp(input, "||", 2);}
inline bool is_logical_operator(const std::string& input) {return is_logical_operator(input.data());}
inline bool is_comparer(const char* input)
{
	return '>' == *input || '<' == *input ||
		0 == strncmp(input, ">=", 2) || 0 == strncmp(input, "<=", 2) || 0 == strncmp(input, "==", 2) || 0 == strncmp(input, "!=", 2);
}
inline bool is_comparer(const std::string& input) {return is_comparer(input.data());}

inline bool is_key_1(char input)
{
	return '!' == input || '(' == input || ')' == input || '>' == input || '<' == input || '?' == input || ':' == input ||
		is_operator(input);
}
inline bool is_key_1(const char* input) {return is_key_1(*input);}
inline bool is_key_1(const std::string& input) {return is_key_1(input.data());}

inline bool is_key_2(const char* input)
{
	return 0 == strncmp(input, ">=", 2) || 0 == strncmp(input, "<=", 2) ||
		0 == strncmp(input, "==", 2) || 0 == strncmp(input, "!=", 2) || is_logical_operator(input);
}
inline bool is_key_2(const std::string& input) {return is_key_2(input.data());}

template<typename O> inline bool is_same_operator_level(char op_1, char op_2)
{
	if (O::level() > 2)
		return (is_operator_1(op_1) && is_operator_1(op_2)) || (is_operator_2(op_1) && is_operator_2(op_2));
	return (is_operator_1(op_1) && is_operator_1(op_2)) || ('*' == op_1 && '*' == op_2);
}
/////////////////////////////////////////////////////////////////////////////////////////

template <typename T> class exp;
template <typename T> using exp_type = std::shared_ptr<exp<T>>;
template <typename T> using exp_ctype = const exp_type<T>;
template <typename T> class negative_data_exp;
template <typename T> class not_judge_exp;
template <typename T> class exp
{
public:
	static inline exp_type<T> final_optimize_1(exp_ctype<T>& exp_l, const std::function<exp_type<T>(exp_ctype<T>&)>& creator)
	{
		auto l = exp_l->final_optimize();
		return l ? creator(l) : exp_type<T>();
	}

	static inline exp_type<T> final_optimize_2(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r,
		const std::function<exp_type<T>(exp_ctype<T>&, exp_ctype<T>&)>& creator)
	{
		auto l = exp_l->final_optimize();
		auto r = exp_r->final_optimize();
		return (l || r) ? creator(l ? l : exp_l, r ? r : exp_r) : exp_type<T>();
	}

protected:
	virtual ~exp() {}

public:
	virtual bool is_data() const {return false;}
	virtual bool is_composite() const {return false;} //used for O::level() < 2 to avoid recursion, operator, left item and right item must be valid
	virtual bool is_reverser() const {return false;} //true to false, value to -value, and vice versa, just left item is valid
	virtual bool need_to_bool() const {return false;} //value to bool (via (bool) (0 != value)), just left item is valid
	virtual int get_depth() const {return 1;}
	virtual void show_immediate_value() const {}

	virtual exp_type<T> clone() const = 0;
	virtual const std::string& get_operator() const {throw("unsupported get operator operation");} // * / + - > >= < <= == != && ||
	virtual exp_ctype<T>& get_road_map() const {static exp_ctype<T> na; return na;}
	virtual exp_ctype<T>& get_left_item() const {return get_road_map();}
	virtual exp_ctype<T>& get_right_item() const {return get_road_map();}

	inline T operator()(const std::function<T(const std::string&)>& cb) const {return data(cb);}
	virtual T data(const std::function<T(const std::string&)>&) const = 0;
	virtual bool judge(const std::function<T(const std::string&)>&) const = 0;
	virtual exp_type<T> to_negative() const {return std::make_shared<negative_data_exp<T>>(clone());}
	virtual exp_type<T> bang() const {return std::make_shared<not_judge_exp<T>>(clone());}

	virtual void clear() {}
	virtual exp_type<T> final_optimize() const {return exp_type<T>();}

	//for data expression only
	/////////////////////////////////////////////////////////////////////////////////////////
	virtual bool is_immediate() const {return false;}
	virtual bool is_composite_variable() const {return false;}
	//whether this expression can be transformed to negative without introducing negation operations, for example '2 * a' to '-2 * a' or
	//with reducing existed negation operations, for example '-a to a'.
	virtual bool is_easy_to_negative() const {return false;}
	virtual bool is_negative() const {return false;} //needs negation operation at runtime
	virtual T get_immediate_value() const {throw("unsupported get immediate value operation!");} //valid if is_immediate()
	virtual int get_exponent() const {throw("unsupported get exponent operation!");} //valid if is_composite_variable()
	virtual T get_multiplier() const {throw("unsupported get multiplier operation!");} //valid if is_composite_variable()
	virtual const std::string& get_variable_name() const {throw("unsupported get variable name operation!");} //valid if is_composite_variable()

	virtual bool merge_with(char, exp_ctype<T>&) {return false;}
	virtual bool merge_with(exp_ctype<T>&, char) {return false;}
	virtual exp_type<T> trim_myself() {return exp_type<T>();}
	/////////////////////////////////////////////////////////////////////////////////////////
	//for data expression only
};

template <typename T, template <typename> class EXP> class unitary_exp : public EXP<T>
{
protected:
	unitary_exp(exp_ctype<T>& _exp_l) : exp_l(_exp_l) {}

public:
	virtual int get_depth() const {return 1 + exp_l->get_depth();}
	virtual void show_immediate_value() const {exp_l->show_immediate_value();}
	virtual exp_ctype<T>& get_left_item() const {return exp_l;}

	virtual void clear() {exp_l.reset();}

private:
	exp_type<T> exp_l;
};

template <typename T, typename O> class binary_data_exp;
template <typename T, template <typename> class EXP> class binary_exp : public EXP<T>
{
protected:
	binary_exp(exp_ctype<T>& _exp_l, exp_ctype<T>& _exp_r, char c) : op(1, c), exp_l(_exp_l), exp_r(_exp_r) {}
	binary_exp(exp_ctype<T>& _exp_l, exp_ctype<T>& _exp_r, const std::string& _op) : op(_op), exp_l(_exp_l), exp_r(_exp_r) {}

public:
	virtual int get_depth() const {return 1 + std::max(exp_l->get_depth(), exp_r->get_depth());}
	virtual void show_immediate_value() const {exp_l->show_immediate_value(); exp_r->show_immediate_value();}
	virtual const std::string& get_operator() const {return op;}
	virtual exp_ctype<T>& get_left_item() const {return exp_l;}
	virtual exp_ctype<T>& get_right_item() const {return exp_r;}

	virtual void clear() {exp_l.reset(); exp_r.reset();}

private:
	template <typename, typename> friend class binary_data_exp;
	exp_type<T>& left() {return exp_l;}
	exp_type<T>& right() {return exp_r;}

private:
	std::string op;
	exp_type<T> exp_l, exp_r;
};

template <typename T> class data_exp;
template <typename T> using data_exp_type = std::shared_ptr<data_exp<T>>;
template <typename T> using data_exp_ctype = const data_exp_type<T>;
template <typename T> class data_exp : public exp<T>
{
public:
	virtual bool is_data() const {return true;}
	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return 0 != this->operator()(cb);}
};

template <typename T> class judge_exp;
template <typename T> using judge_exp_type = std::shared_ptr<judge_exp<T>>;
template <typename T> using judge_exp_ctype = const judge_exp_type<T>;
template <typename T> class judge_exp : public exp<T>
{
public:
	virtual T data(const std::function<T(const std::string&)>& cb) const {return (T) this->judge(cb);}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> inline bool is_same_composite_variable(const std::string& variable_name, exp_ctype<T>& other_exp)
	{return other_exp->is_composite_variable() && variable_name == other_exp->get_variable_name();}

template <typename T> inline bool is_same_composite_variable(exp_ctype<T>& dexp_l, exp_ctype<T>& dexp_r)
	{return dexp_l->is_composite_variable() && is_same_composite_variable(dexp_l->get_variable_name(), dexp_r);}

template <typename T> inline bool is_divisible(T dividend, T divisor)
{
	if (0 == divisor)
		throw("divide zero");
	return 0 == dividend || 1 == divisor || -1 == divisor || dividend == divisor || -dividend == divisor;
}

template <typename T> class negative_data_exp : public unitary_exp<T, data_exp>
{
public:
	static inline bool is_my_type(exp_ctype<T>& exp) {return exp->is_data() && exp->is_reverser();}

public:
	negative_data_exp(exp_ctype<T>& exp_l) : unitary_exp<T, data_exp>(exp_l) {assert(!is_my_type(exp_l));}

	virtual bool is_reverser() const {return true;}
	virtual exp_type<T> clone() const {throw("unsupported clone operation!");}

	virtual T data(const std::function<T(const std::string&)>& cb) const {return -(*this->get_left_item())(cb);}
	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return this->get_left_item()->judge(cb);} //equals to 0 != data(cb), but more effective
	virtual exp_type<T> to_negative() const {return this->get_left_item();}
	virtual exp_type<T> bang() const //'!(-!a)' equals to 'a?', '!(-a)' equals to '!a'
		{auto& exp_l = this->get_left_item(); return not_judge_exp<T>::is_my_type(exp_l) ? exp_l->bang() : std::make_shared<not_judge_exp<T>>(exp_l);}

	virtual exp_type<T> final_optimize() const
	{
		return exp<T>::final_optimize_1(this->get_left_item(), [](exp_ctype<T>& l) {
			return is_my_type(l) ? l->to_negative() : std::make_shared<negative_data_exp<T>>(l);
		});
	}

	virtual bool is_easy_to_negative() const {return true;}
	virtual bool is_negative() const {return true;}
};

template <typename T> inline T calculate(T& operand, char op, T v)
{
	switch (op)
	{
	case '+':
		return operand += v;
	case '-':
		return operand -= v;
	case '*':
		return operand *= v;
	case '/':
		if (0 == v)
			throw("divide zero");
		return operand /= v;
	default:
		throw("undefined operator " + std::string(1, op));
	}
}

template <typename T> class immediate_data_exp : public data_exp<T>
{
public:
	immediate_data_exp(T v) : value(v) {}

	virtual void show_immediate_value() const {std::cout << ' ' << value;}
	exp_type<T> clone() const {return std::make_shared<immediate_data_exp<T>>(value);}
	virtual T data(const std::function<T(const std::string&)>&) const
	{
#ifdef DEBUG
		std::cout << " get immediate value " << value << std::endl;
#endif
		return value;
	}
	virtual exp_type<T> to_negative() const {return std::make_shared<immediate_data_exp<T>>(-value);} //more effective than exp<T>::to_negative()

	virtual bool is_immediate() const {return true;}
	virtual bool is_easy_to_negative() const {return true;}
	virtual T get_immediate_value() const {return value;}

	virtual bool merge_with(char op, exp_ctype<T>& exp)
		{return exp->is_immediate() ? (calculate(value, op, exp->get_immediate_value()), true) : false;}

private:
	T value;
};

template <typename T, typename O> class composite_variable_data_exp;
template <typename T, typename O> inline exp_type<T> merge_data_exp(exp_ctype<T>&, exp_ctype<T>&, char);
template <typename T, typename O> inline exp_type<T> make_binary_data_exp(exp_ctype<T>&, exp_ctype<T>&, char);
template <typename T, typename O> class binary_data_exp : public binary_exp<T, data_exp>
{
protected:
	using binary_exp<T, data_exp>::binary_exp;

public:
	virtual bool is_composite() const {return true;}
	virtual exp_type<T> clone() const {return make_binary_data_exp<T, O>(this->get_left_item(), this->get_right_item(), this->get_operator().front());}

	virtual exp_type<T> to_negative() const
	{
		auto op = this->get_operator().front();
		const auto& exp_l = this->get_left_item();
		const auto& exp_r = this->get_right_item();
		switch (op)
		{
		case '+':
			if (exp_l->is_easy_to_negative())
				return merge_data_exp<T, O>(exp_l->to_negative(), exp_r, '-');
			else if (exp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(exp_r->to_negative(), exp_l, '-');
			else
			{
				auto depth_l = exp_l->get_depth(), depth_r = exp_r->get_depth();
				if (depth_l <= depth_r)
					return merge_data_exp<T, O>(exp_l->to_negative(), exp_r, '-');
				else
					return merge_data_exp<T, O>(exp_r->to_negative(), exp_l, '-');
			}
			break;
		case '-':
			//without this branch, '-a - b' will be transformed to 'b + a' (from b - -a) instead of 'a + b', the former looks strange.
			if (exp_l->is_negative())
				return merge_data_exp<T, O>(exp_l->to_negative(), exp_r, '+');
			return merge_data_exp<T, O>(exp_r, exp_l, '-');
			break;
		case '*':
		case '/':
			if (exp_l->is_easy_to_negative())
				return merge_data_exp<T, O>(exp_l->to_negative(), exp_r, op);
			else if (exp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(exp_l, exp_r->to_negative(), op);
			else
			{
				auto depth_l = exp_l->get_depth(), depth_r = exp_r->get_depth();
				if (depth_l <= depth_r)
					return merge_data_exp<T, O>(exp_l->to_negative(), exp_r, op);
				else
					return merge_data_exp<T, O>(exp_l, exp_r->to_negative(), op);
			}
			break;
		}

		return exp_type<T>();
	}

	virtual exp_type<T> final_optimize() const
	{
		return exp<T>::final_optimize_2(this->get_left_item(), this->get_right_item(), [this](exp_ctype<T>& l, exp_ctype<T>& r) {
			return merge_data_exp<T, O>(l, r, this->get_operator().front());
		});
	}

	virtual bool is_easy_to_negative() const
	{
		return is_negative() || (is_operator_2(this->get_operator()) &&
			(this->get_left_item()->is_easy_to_negative() || this->get_right_item()->is_easy_to_negative()));
	}

	virtual bool is_negative() const
	{
		//'-a - b', '-a * b', 'a * -b', '-a / b' and 'a / -b' are considered to be negative,
		// introduce negative property to binary_data_exp is to eliminate negation operations if possible.
		//following expressions cannot be eventual outcomes (we'll transform them to corresponding right ones), see trim_myself for more details:
		//any immediate value (represented by C below) is considered to be NOT negative since it needs no negation operation at runtime, please note.
		// '-a + -b'	will be transformed to '-a - b'
		// '-a + b '	will be transformed to 'b - a'
		// '-a - -b'	will be transformed to 'b - a'
		// '-a - C '	will be transformed to '-C - a'
		// '-a * -b'	will be transformed to 'a * b'
		// '-a * C '	will be transformed to 'a * -C'
		// 'C * -b '	will be transformed to '-C * b'
		// '-a / -b'	will be transformed to 'a / b'
		// '-a / C '	will be transformed to 'a / -C'
		// 'C / -b '	will be transformed to '-C / b'
		switch (this->get_operator().front())
		{
		case '-':
			return this->get_left_item()->is_negative();
			break;
		case '*':
		case '/':
			return this->get_left_item()->is_negative() || this->get_right_item()->is_negative();
			break;
		}

		return false;
	}

	virtual bool merge_with(char other_op, exp_ctype<T>& other_exp)
	{
		auto op = this->get_operator().front();
		auto& exp_l = this->left();
		auto& exp_r = this->right();
		if (is_same_operator_level<O>(op, other_op))
		{
			if (O::level() < 2)
			{
				if (!other_exp->is_immediate())
					return false;
				else if (exp_l->is_immediate())
					return exp_l->merge_with(other_op, other_exp);
				else if (!exp_r->is_immediate())
					return false;

				if ('-' == op)
					other_op = '+' == other_op ? '-' : '+';
				else if ('/' == op) //other_op must also be '/'
					other_op = '*';
				return exp_r->merge_with(other_op, other_exp);
			}
			else if (exp_l->merge_with(other_op, other_exp))
				return true;
			else if ('+' == op || '*' == op)
				return exp_r->merge_with(other_op, other_exp);
			else if ('-' == op)
				other_op = '+' == other_op ? '-' : '+';
			else if ('/' == op)
				other_op = '*' == other_op ? '/' : '*';
			return exp_r->merge_with(other_op, other_exp);
		}
		else if (2 == O::level() && '+' == other_op && '/' == op && //'N1*a^M / C + N2*a^M' -> '(N1 + N2*C)*a^M / C'
			exp_r->is_immediate() && is_same_composite_variable(exp_l, other_exp) &&
			exp_l->get_exponent() == other_exp->get_exponent())
		{
			exp_l = std::make_shared<composite_variable_data_exp<T, O>>(exp_l->get_variable_name(),
				exp_l->get_multiplier() + other_exp->get_multiplier() * exp_r->get_immediate_value(), exp_l->get_exponent());
			return true;
		}

		return false;
	}

	virtual exp_type<T> trim_myself()
	{
		auto& exp_l = this->left();
		auto& exp_r = this->right();

		auto data = exp_l->trim_myself();
		if (data)
			exp_l = data;

		data = exp_r->trim_myself();
		if (data)
			exp_r = data;

		switch (this->get_operator().front())
		{
		case '+':
			if (exp_l->is_immediate() && 0 == exp_l->get_immediate_value())
				return exp_r;
			else if (exp_r->is_immediate() && 0 == exp_r->get_immediate_value())
				return exp_l;
			else if (exp_r->is_negative())
				return merge_data_exp<T, O>(exp_l, exp_r->to_negative(), '-');
			else if (exp_l->is_negative())
				return merge_data_exp<T, O>(exp_r, exp_l->to_negative(), '-');
			break;
		case '-':
			if (exp_l->is_immediate() && 0 == exp_l->get_immediate_value())
				return exp_r->to_negative();
			else if (exp_r->is_immediate() && 0 == exp_r->get_immediate_value())
				return exp_l;
			else if (exp_r->is_negative())
				return merge_data_exp<T, O>(exp_l, exp_r->to_negative(), '+');
			else if (exp_l->is_negative() && exp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(exp_r->to_negative(), exp_l->to_negative(), '-');
			break;
		case '*':
			if (exp_l->is_immediate())
			{
				auto v = exp_l->get_immediate_value();
				if (1 == v)
					return exp_r;
				else if (0 == v)
					return exp_l;
				else if (-1 == v)
					return exp_r->to_negative();
			}
			else if (exp_r->is_immediate())
			{
				auto v = exp_r->get_immediate_value();
				if (1 == v)
					return exp_l;
				else if (0 == v)
					return exp_r;
				else if (-1 == v)
					return exp_l->to_negative();
			}
			if ((exp_l->is_negative() && (exp_r->is_negative() || exp_r->is_easy_to_negative())) ||
				(exp_r->is_negative() && (exp_l->is_negative() || exp_l->is_easy_to_negative())))
				return merge_data_exp<T, O>(exp_l->to_negative(), exp_r->to_negative(), '*');
			break;
		case '/':
			if (exp_l->is_immediate())
			{
				if (0 == exp_l->get_immediate_value())
					return exp_l;
			}
			else if (exp_r->is_immediate())
			{
				auto v = exp_r->get_immediate_value();
				if (1 == v)
					return exp_l;
				else if (0 == v)
					throw("divide zero");
				else if (-1 == v)
					return exp_l->to_negative();
			}
			if ((exp_l->is_negative() && (exp_r->is_negative() || exp_r->is_easy_to_negative())) ||
				(exp_r->is_negative() && (exp_l->is_negative() || exp_l->is_easy_to_negative())))
				return merge_data_exp<T, O>(exp_l->to_negative(), exp_r->to_negative(), '/');
			else if (2 == O::level() && is_same_composite_variable(exp_l, exp_r))
			{
				//'N1*a^M1 / N2*a^M2' -> 'N1*a^(M1-M2) / N2' where M1 > M2 > 0
				//for other conditions, we will handle them in composite_variable_data_exp's:
				// merge_with, or
				// trim_myself, or
				// final_optimize, or
				// cannot handle them with optimization level < 3
				auto exponent_l = exp_l->get_exponent(), exponent_r = exp_r->get_exponent();
				if (exponent_l > exponent_r && exponent_r > 0)
				{
					exp_l = std::make_shared<composite_variable_data_exp<T, O>>(exp_l->get_variable_name(),
						exp_l->get_multiplier(), exponent_l - exponent_r);
					exp_r = std::make_shared<immediate_data_exp<T>>(exp_r->get_multiplier());
					return trim_myself();
				}
			}
			break;
		}

		return exp_type<T>();
	}
};

template <typename T, typename O> class add_data_exp : public binary_data_exp<T, O>
{
public:
	add_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_data_exp<T, O>(exp_l, exp_r, '+') {}

	virtual T data(const std::function<T(const std::string&)>& cb) const
		{return (*this->get_left_item())(cb) + (*this->get_right_item())(cb);}
};

template <typename T, typename O> class sub_data_exp : public binary_data_exp<T, O>
{
public:
	sub_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_data_exp<T, O>(exp_l, exp_r, '-') {}

	virtual T data(const std::function<T(const std::string&)>& cb) const
		{return (*this->get_left_item())(cb) - (*this->get_right_item())(cb);}
};

template <typename T, typename O> class multi_data_exp : public binary_data_exp<T, O>
{
public:
	multi_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_data_exp<T, O>(exp_l, exp_r, '*') {}

	virtual T data(const std::function<T(const std::string&)>& cb) const
		{return (*this->get_left_item())(cb) * (*this->get_right_item())(cb);}
};

template <typename T, typename O> class div_data_exp : public binary_data_exp<T, O>
{
public:
	div_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_data_exp<T, O>(exp_l, exp_r, '/') {}

	virtual T data(const std::function<T(const std::string&)>& cb) const
	{
#ifndef _MSC_VER //keep the order of data fetching for debuging
		auto dividend = (*this->get_left_item())(cb);
#endif
		auto divisor = (*this->get_right_item())(cb);
		if (0 == divisor)
			throw("divide zero");
#ifdef _MSC_VER
		return (*this->get_left_item())(cb) / divisor;
#else
		return dividend / divisor;
#endif
	}
};

template <typename T> class variable_data_exp : public data_exp<T>
{
public:
	variable_data_exp(const std::string& _variable_name) : variable_name(_variable_name) {}

	exp_type<T> clone() const {return std::make_shared<variable_data_exp<T>>(variable_name);}
	virtual T data(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return v;
#endif
		return cb(variable_name);
	}

private:
	std::string variable_name;
};

template <typename T> class exponent_data_exp : public data_exp<T>
{
public:
	exponent_data_exp(const std::string& _variable_name, int _exponent) : variable_name(_variable_name), exponent(_exponent) {}

	virtual void show_immediate_value() const {std::cout << ' ' << exponent;}
	exp_type<T> clone() const {return std::make_shared<exponent_data_exp<T>>(variable_name, exponent);}
	virtual T data(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return (T) pow(v, exponent);
#endif
		return (T) pow(cb(variable_name), exponent);
	}

private:
	std::string variable_name;
	int exponent;
};

template <typename T, typename O> class composite_variable_data_exp : public data_exp<T>
{
public:
	composite_variable_data_exp(const std::string& _variable_name, T _multiplier = 1, int _exponent = 1)
		: variable_name(_variable_name), multiplier(_multiplier), exponent(_exponent) {}

	virtual void show_immediate_value() const {std::cout << ' ' << multiplier << ' ' << exponent;}
	virtual exp_type<T> clone() const {return std::make_shared<composite_variable_data_exp<T, O>>(variable_name, multiplier, exponent);}
	virtual T data(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return multiplier * (T) pow(v, exponent);
#endif
		return multiplier * (T) pow(cb(variable_name), exponent);
	}
	virtual exp_type<T> to_negative() const
		{return std::make_shared<composite_variable_data_exp<T, O>>(variable_name, -multiplier, exponent);} //more effective than exp<T>::to_negative()

	virtual exp_type<T> final_optimize() const
	{
		exp_type<T> data = std::make_shared<immediate_data_exp<T>>(multiplier);
		if (0 == multiplier || 0 == exponent)
			return data;
		else if (1 != multiplier && -1 != multiplier && exponent > 1)
			return exp_type<T>();
		else if (1 == multiplier && exponent < 0)
			return std::make_shared<exponent_data_exp<T>>(variable_name, exponent);
		else if (-1 == multiplier && exponent < 0)
			return std::make_shared<negative_data_exp<T>>(std::make_shared<exponent_data_exp<T>>(variable_name, exponent));
		else if (-1 == exponent)
			return std::make_shared<div_data_exp<T, O>>(data, std::make_shared<variable_data_exp<T>>(variable_name));
		else if (exponent < -1)
			return std::make_shared<div_data_exp<T, O>>(data, std::make_shared<exponent_data_exp<T>>(variable_name, -exponent));

		if (1 == exponent)
			data = std::make_shared<multi_data_exp<T, O>>(data, std::make_shared<variable_data_exp<T>>(variable_name));
		else // > 1
			data = std::make_shared<multi_data_exp<T, O>>(data, std::make_shared<exponent_data_exp<T>>(variable_name, exponent));

		auto re = data->trim_myself();
		return re ? re : data;
	}

	virtual bool is_composite_variable() const {return true;}
	virtual bool is_easy_to_negative() const {return true;}
	virtual int get_exponent() const {return exponent;}
	virtual T get_multiplier() const {return multiplier;}
	virtual const std::string& get_variable_name() const {return variable_name;}

	virtual bool merge_with(char other_op, exp_ctype<T>& other_exp)
	{
		if (other_exp->is_immediate())
		{
			auto v = other_exp->get_immediate_value();
			if ('*' == other_op && (O::level() > 2 || exponent >= 0))
				multiplier *= v;
			else if ('/' == other_op && (O::level() > 2 || exponent <= 0 || is_divisible(multiplier, v)))
				multiplier /= v;
			else
				return false;
		}
		else if (!is_same_composite_variable(variable_name, other_exp))
			return false;
		else
		{
			auto other_exponent = other_exp->get_exponent();
			auto other_multiplier = other_exp->get_multiplier();
			switch (other_op)
			{
			case '+':
				if ((O::level() < 3 && exponent < 0) || exponent != other_exponent)
					return false;
				multiplier += other_multiplier;
				break;
			case '-':
				if ((O::level() < 3 && exponent < 0) || exponent != other_exponent)
					return false;
				multiplier -= other_multiplier;
				break;
			case '*':
				if (O::level() > 2 || (exponent >= 0 && other_exponent >= 0))
				{
					multiplier *= other_multiplier;
					exponent += other_exponent;
				}
				else
					return false;
				break;
			case '/':
				if (O::level() > 2 || (other_exponent >= 0 && //for this exponent, negative is also okay
					(other_exponent >= exponent || is_divisible(multiplier, other_multiplier))))
				{
					multiplier /= other_multiplier;
					exponent -= other_exponent;
				}
				else
					return false;
				break;
			default:
				throw("undefined operator " + std::string(1, other_op));
				break;
			}
		}

		return true;
	}

	//handle 'C * Na^M' and 'C / Na^M' only.
	virtual bool merge_with(exp_ctype<T>& other_exp, char other_op)
	{
		if (!other_exp->is_immediate())
			return false;
		else if ('*' == other_op)
			return merge_with('*', other_exp);
		else if ('/' == other_op && (O::level() > 2 || exponent >= 0))
		{
			if (0 == multiplier)
				throw("divide zero");

			multiplier = other_exp->get_immediate_value() / multiplier;
			exponent = -exponent;
			return true;
		}

		return false;
	}

	virtual exp_type<T> trim_myself()
	{
		if (0 == multiplier || 0 == exponent)
			return std::make_shared<immediate_data_exp<T>>(multiplier);
		return exp_type<T>();
	}

private:
	std::string variable_name;
	T multiplier;
	int exponent;
};

template <typename T, typename O>
inline exp_type<T> make_binary_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r, char op)
{
	switch (op)
	{
	case '+':
		return std::make_shared<add_data_exp<T, O>>(exp_l, exp_r);
		break;
	case '-':
		return std::make_shared<sub_data_exp<T, O>>(exp_l, exp_r);
		break;
	case '*':
		return std::make_shared<multi_data_exp<T, O>>(exp_l, exp_r);
		break;
	case '/':
		return std::make_shared<div_data_exp<T, O>>(exp_l, exp_r);
		break;
	default:
		throw("undefined operator " + std::string(1, op));
		break;
	}
}

template <typename T, typename O> inline exp_type<T> merge_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r, char op)
{
	if (0 == O::level())
		return make_binary_data_exp<T, O>(exp_l, exp_r, op);
	else if (exp_r->merge_with(exp_l, op)) //parse 'C * Na^M' and 'C / Na^M' to composite_variable_data_exp instead of binary_data_exp
	{
		auto data = exp_r->trim_myself();
		return data ? data : exp_r;
	}
	else if (exp_l->merge_with(op, exp_r)) //composite_variable_data_exp is involved here
	{
		auto data = exp_l->trim_myself();
		return data ? data : exp_l;
	}
	else if (exp_r->is_composite())
	{
		auto op_2 = exp_r->get_operator().front();
		if (is_same_operator_level<O>(op, op_2))
		{
			exp_type<T> data;
			if (exp_l->merge_with(op, exp_r->get_left_item()))
			{
				data = exp_l->trim_myself();
				if (!data)
					data = exp_l;
			}

			if ('-' == op)
				op_2 = '-' == op_2 ? '+' : '-';
			else if ('/' == op)
				op_2 = '/' == op_2 ? '*' : '/';
			if (!data)
			{
				if (exp_l->merge_with(op_2, exp_r->get_right_item()))
				{
					data = exp_l->trim_myself();
					if (!data)
						data = exp_l;

					return merge_data_exp<T, O>(data, exp_r->get_left_item(), op);
				}
				else
					data = merge_data_exp<T, O>(exp_l, exp_r->get_left_item(), op);
			}

			return merge_data_exp<T, O>(data, exp_r->get_right_item(), op_2);
		}
		else if (2 == O::level() && '+' == op && '/' == op_2 && //'N1*a^M + N2*a^M / C' -> '(N1*C + N2)*a^M / C'
			exp_r->get_right_item()->is_immediate() && is_same_composite_variable(exp_l, exp_r->get_left_item()) &&
			exp_l->get_exponent() == exp_r->get_left_item()->get_exponent())
		{
			auto multiplier = exp_l->get_multiplier() * exp_r->get_right_item()->get_immediate_value() +
				exp_r->get_left_item()->get_multiplier();
			return merge_data_exp<T, O>(
				std::make_shared<composite_variable_data_exp<T, O>>(exp_l->get_variable_name(), multiplier, exp_l->get_exponent()),
				exp_r->get_right_item(), '/');
		}
	}

	auto data = make_binary_data_exp<T, O>(exp_l, exp_r, op);
	auto re = data->trim_myself();
	return re ? re : data;
}
template <typename T, typename O>
inline exp_type<T> merge_data_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r, const std::string& op)
	{return merge_data_exp<T, O>(exp_l, exp_r, *op.data());}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> class transparent_judge_exp : public unitary_exp<T, judge_exp>
{
public:
	static inline bool is_my_type(exp_ctype<T>& exp) {return !exp->is_data() && exp->need_to_bool();}

public:
	transparent_judge_exp(exp_ctype<T>& exp_l) :
		unitary_exp<T, judge_exp>(negative_data_exp<T>::is_my_type(exp_l) ? exp_l->to_negative() : exp_l) //'(-a)?' equals to 'a?'
		{assert(exp_l->is_data());}

	virtual bool need_to_bool() const {return true;}
	virtual exp_type<T> clone() const {return std::make_shared<transparent_judge_exp<T>>(this->get_left_item());}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return this->get_left_item()->judge(cb);}
	virtual exp_type<T> bang() const {return std::make_shared<not_judge_exp<T>>(this->get_left_item());} //'!(a?)' equals to '!a'

	virtual exp_type<T> final_optimize() const
	{
		return exp<T>::final_optimize_1(this->get_left_item(), [](exp_ctype<T>& l) {
			return is_my_type(l) ? l : std::make_shared<transparent_judge_exp<T>>(l);
		});
	}
};

template <typename T> class not_judge_exp : public unitary_exp<T, judge_exp>
{
public:
	static inline bool is_my_type(exp_ctype<T>& exp) {return !exp->is_data() && exp->is_reverser();}

public:
	not_judge_exp(exp_ctype<T>& exp_l) :
		unitary_exp<T, judge_exp>(negative_data_exp<T>::is_my_type(exp_l) ? exp_l->to_negative() : exp_l) //'!(-a)' equals to '!a'
		{assert(!is_my_type(exp_l));}

	virtual bool is_reverser() const {return true;}
	virtual exp_type<T> clone() const {return std::make_shared<not_judge_exp<T>>(this->get_left_item());}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return !this->get_left_item()->judge(cb);}
	virtual exp_type<T> bang() const
		{auto& exp_l = this->get_left_item(); return exp_l->is_data() ? std::make_shared<transparent_judge_exp<T>>(exp_l) : exp_l;}

	virtual exp_type<T> final_optimize() const
	{
		return exp<T>::final_optimize_1(this->get_left_item(), [](exp_ctype<T>& l) {
			return is_my_type(l) ? l->bang() : std::make_shared<not_judge_exp<T>>(l);
		});
	}
};

template <typename T> inline exp_type<T> make_binary_judge_exp(exp_ctype<T>&, exp_ctype<T>&, const std::string&);
//doesn't need to be composite since for binary_judge_exp, no recursion will happen in function bang and to_negative with any optimization level.
template <typename T> class binary_judge_exp : public binary_exp<T, judge_exp>
{
protected:
	using binary_exp<T, judge_exp>::binary_exp;

public:
	virtual exp_type<T> clone() const {return make_binary_judge_exp<T>(this->get_left_item(), this->get_right_item(), this->get_operator());}

	virtual exp_type<T> final_optimize() const
	{
		auto exp_l = this->get_left_item(), exp_r = this->get_right_item();
		auto final_exp = exp<T>::final_optimize_2(exp_l, exp_r, [&](exp_ctype<T>& l, exp_ctype<T>& r) {
			exp_l = l;
			exp_r = r;
			return make_binary_judge_exp<T>(l, r, this->get_operator());
		});

		exp_type<T> useful_exp;
		if (exp_l->is_immediate() && 0 == exp_l->get_immediate_value())
			useful_exp = exp_r;
		else if (exp_r->is_immediate() && 0 == exp_r->get_immediate_value())
			useful_exp = exp_l;

		if (useful_exp)
		{
			auto& c = this->get_operator();
			if ("==" == c) //'a == 0' equals to '!a'
				return not_judge_exp<T>::is_my_type(useful_exp) ? useful_exp->bang() : std::make_shared<not_judge_exp<T>>(useful_exp);
			else if ("!=" == c) //'a != 0' equals to 'a?'
				return useful_exp->is_data() ? std::make_shared<transparent_judge_exp<T>>(useful_exp) : useful_exp;
		}
		return final_exp;
	}
};

template <typename T> class smaller_equal_judge_exp;
template <typename T> class bigger_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, ">") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) > (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<smaller_equal_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> class smaller_judge_exp;
template <typename T> class bigger_equal_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_equal_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, ">=") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) >= (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<smaller_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> class smaller_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, "<") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) < (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<bigger_equal_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> class smaller_equal_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_equal_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, "<=") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) <= (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<bigger_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> class not_equal_judge_exp;
template <typename T> class equal_judge_exp : public binary_judge_exp<T>
{
public:
	equal_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, "==") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) == (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<not_equal_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> class not_equal_judge_exp : public binary_judge_exp<T>
{
public:
	not_equal_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : binary_judge_exp<T>(exp_l, exp_r, "!=") {}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const {return (*this->get_left_item())(cb) != (*this->get_right_item())(cb);}

	virtual exp_type<T> bang() const {return std::make_shared<equal_judge_exp<T>>(this->get_left_item(), this->get_right_item());}
};

template <typename T> inline exp_type<T> make_binary_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r, const std::string& c)
{
	if (">" == c)
		return std::make_shared<bigger_judge_exp<T>>(exp_l, exp_r);
	else if (">=" == c)
		return std::make_shared<bigger_equal_judge_exp<T>>(exp_l, exp_r);
	else if ("<" == c)
		return std::make_shared<smaller_judge_exp<T>>(exp_l, exp_r);
	else if ("<=" == c)
		return std::make_shared<smaller_equal_judge_exp<T>>(exp_l, exp_r);
	else if ("==" == c)
		return std::make_shared<equal_judge_exp<T>>(exp_l, exp_r);
	else if ("!=" == c)
		return std::make_shared<not_equal_judge_exp<T>>(exp_l, exp_r);
	else
		throw("unknown compare operator " + c);
}

template <typename T>inline exp_type<T> make_logical_exp(exp_ctype<T>&, exp_ctype<T>&, const std::string&);
template <typename T> class logical_exp : public binary_exp<T, judge_exp>
{
protected:
	using binary_exp<T, judge_exp>::binary_exp;

public:
	virtual bool is_composite() const {return true;}
	virtual exp_type<T> clone() const {return make_logical_exp<T>(this->get_left_item(), this->get_right_item(), this->get_operator());}

	virtual exp_type<T> final_optimize() const
	{
		auto exp_l = this->get_left_item(), exp_r = this->get_right_item();
		auto final_exp = exp<T>::final_optimize_2(exp_l, exp_r, [&](exp_ctype<T>& l, exp_ctype<T>& r) {
			exp_l = l;
			exp_r = r;
			return make_logical_exp<T>(l, r, this->get_operator());
		});

		if (not_judge_exp<T>::is_my_type(exp_l) && not_judge_exp<T>::is_my_type(exp_r)) //'!a && !b' equals to '!(a || b)', '!a || !b' equals to '!(a && b)'
			return std::make_shared<not_judge_exp<T>>(make_logical_exp(exp_l->bang(), exp_r->bang(), "&&" == this->get_operator() ? "||" : "&&"));
		return final_exp;
	}
};

template <typename T> class or_judge_exp;
template <typename T> class and_judge_exp : public logical_exp<T>
{
public:
	and_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : logical_exp<T>(exp_l, exp_r, "&&") {}

	virtual exp_type<T> bang() const
		{return std::make_shared<or_judge_exp<T>>(this->get_left_item()->bang(), this->get_right_item()->bang());}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const
		{return this->get_left_item()->judge(cb) && this->get_right_item()->judge(cb);}
};

template <typename T> class or_judge_exp : public logical_exp<T>
{
public:
	or_judge_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r) : logical_exp<T>(exp_l, exp_r, "||") {}

	virtual exp_type<T> bang() const
		{return std::make_shared<and_judge_exp<T>>(this->get_left_item()->bang(), this->get_right_item()->bang());}

	virtual bool judge(const std::function<T(const std::string&)>& cb) const
		{return this->get_left_item()->judge(cb) || this->get_right_item()->judge(cb);}
};

template <typename T>inline exp_type<T> make_logical_exp(exp_ctype<T>& exp_l, exp_ctype<T>& exp_r, const std::string& lop)
{
	if ("&&" == lop)
		return std::make_shared<and_judge_exp<T>>(exp_l, exp_r);
	else if ("||" == lop)
		return std::make_shared<or_judge_exp<T>>(exp_l, exp_r);
	else
		throw("undefined logical operator " + lop);
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> class question_exp : public data_exp<T>
{
public:
	question_exp(exp_ctype<T>& _exp, exp_ctype<T>& _exp_l, exp_ctype<T>& _exp_r) : exp(_exp), exp_l(_exp_l), exp_r(_exp_r) {}

	virtual int get_depth() const {return 1 + std::max(exp->get_depth(), std::max(exp_l->get_depth(), exp_r->get_depth()));}
	virtual void show_immediate_value() const
		{exp->show_immediate_value(); exp_l->show_immediate_value(); exp_r->show_immediate_value();}
	exp_type<T> clone() const {return std::make_shared<question_exp<T>>(exp, exp_l, exp_r);}
	virtual exp_ctype<T>& get_road_map() const {return exp;}
	virtual exp_ctype<T>& get_left_item() const {return exp_l;}
	virtual exp_ctype<T>& get_right_item() const {return exp_r;}

	virtual T data(const std::function<T(const std::string&)>& cb) const {return exp->judge(cb) ? (*exp_l)(cb) : (*exp_r)(cb);}
	virtual void clear() {exp.reset(); exp_l.reset(); exp_r.reset();}

	virtual exp_type<T> final_optimize() const
	{
		auto m = exp->final_optimize();
		auto l = exp_l->final_optimize();
		auto r = exp_r->final_optimize();

		return (m || l || r) ? std::make_shared<question_exp<T>>(m ? m : exp, l ? l : exp_l, r ? r : exp_r) : exp_type<T>();
	}

private:
	exp_type<T> exp, exp_l, exp_r;
};
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> inline bool is_selector(exp_ctype<T>& exp) {return (bool) exp->get_road_map();}
template <typename T> inline bool is_composite(exp_ctype<T>& exp) {return is_selector(exp) || exp->get_left_item();}

template <typename T> inline bool to_not(T& operand) {return (bool) (operand = (T) (0 == operand));}
template <typename T> inline bool to_bool(T& operand) {return (bool) (operand = (T) (0 != operand));}
template <typename T> inline T negate(T& operand) {return operand = -operand;}
template <typename T> inline bool compare(T& operand, const std::string& c, T v)
{
	if (">" == c)
		return (bool) (operand = (T) (operand > v));
	else if (">=" == c)
		return (bool) (operand = (T) (operand >= v));
	else if ("<" == c)
		return (bool) (operand = (T) (operand < v));
	else if ("<=" == c)
		return (bool) (operand = (T) (operand <= v));
	else if ("==" == c)
		return (bool) (operand = (T) (operand == v));
	else if ("!=" == c)
		return (bool) (operand = (T) (operand != v));
	else
		throw("unknown compare operator " + c);
}

//since recursion is used during the whole compilation and execution, if your expression is too complicated to
// be compiled and executed (stack overflow), use
// qme::O0/qme::O1 to compile it,
// qme::safe_data/qme::safe_judge to execute it and
// qme::safe_delete to delete it,
//then recursion will be eliminated, but additional runtime judgement will be performed, we have no choice.
//
//with optimization level qme::O0/qme::O1, following functions are still available, if you're encountering above situation,
//you should not call them manually, please note:
// get_depth
// show_immediate_value
// to_negative
// final_optimize
// is_easy_to_negative
// is_negative
// merge_with
// trim_myself
//return the data and max depth (just traveled branches).
template <typename T> inline std::pair<T, size_t> safe_data(exp_ctype<T>& exp, const std::function<T(const std::string&)>& cb)
{
	if (!is_composite(exp))
		return std::make_pair((*exp)(cb), 1);

	size_t max_depth = 1;
	//the max depth is the max size that this list ever get, so please use at least gcc 5 since we need std::list::size() to have O(1) complexity
	std::list<std::pair<exp_type<T>, int>> exps; //current branch: -1 - before handling, 0 - road map, 1 - left branch, 2 - right branch
	auto direction = 0; //0 - road map, 1 - left-bottom, 2 - right-bottom, 3 - top-left
	exps.emplace_back(exp, -1);
	std::list<T> res;
	for (auto iter = exps.rbegin(); iter != exps.rend();)
		if (0 == direction) //road map
		{
			iter->second = 0;
			auto road_map = iter->first->get_road_map();
			if (!road_map)
				direction = 1;
			else if (is_composite(road_map))
			{
				exps.emplace_back(road_map, -1);
				iter = exps.rbegin();
			}
			else
			{
				res.emplace_back((T) road_map->judge(cb));
				direction = 3;
				max_depth = std::max(exps.size() + 1, max_depth);
			}
		}
		else if (1 == direction) //left
		{
			iter->second = 1;
			auto left = iter->first->get_left_item();
			if (is_composite(left))
			{
				exps.emplace_back(left, -1);
				iter = exps.rbegin();
				direction = 0;
			}
			else
			{
				res.emplace_back((*left)(cb));
				direction = is_selector(iter->first) ? 3 : 2;
				max_depth = std::max(exps.size() + 1, max_depth);
			}
		}
		else if (2 == direction) //right
		{
			iter->second = direction++;
			if (iter->first->is_reverser() || iter->first->need_to_bool())
				continue;
			else if (!is_selector(iter->first))
			{
				auto& lop = iter->first->get_operator();
				if (is_logical_operator(lop))
				{
					assert(!res.empty());
					auto re = 0 != res.back();
					if ("&&" == lop ? !re : re)
						continue; //short circuit control
					res.pop_back();
				}
			}

			auto right = iter->first->get_right_item();
			if (is_composite(right))
			{
				exps.emplace_back(right, -1);
				iter = exps.rbegin();
				direction = 0;
			}
			else
			{
				res.emplace_back((*right)(cb));
				max_depth = std::max(exps.size() + 1, max_depth);
			}
		}
		else //3 == direction, backtrace
		{
			if (iter->first->is_reverser())
			{
				assert(!res.empty());
				if (iter->first->is_data())
					negate(res.back());
				else
					to_not(res.back());
			}
			else if (iter->first->need_to_bool())
			{
				assert(!res.empty());
				to_bool(res.back());
			}
			else if (!is_selector(iter->first))
			{
				auto& op = iter->first->get_operator();
				if (is_logical_operator(op))
				{
					assert(!res.empty());
					to_bool(res.back());
				}
				else
				{
					assert(res.size() > 1);
					auto re = res.back();
					res.pop_back();
					if (is_comparer(op))
						compare(res.back(), op, re);
					else //+-*/
						calculate(res.back(), op.front(), re);
				}
			}

			if (0 == iter->second) //road map
			{
				assert(!res.empty());
				direction = 0 != res.back() ? 1 : 2;
				res.pop_back();
			}
			else if (++iter != exps.rend() && !is_selector(iter->first) && 1 == iter->second)
				direction = 2;
			iter = decltype(iter)(exps.erase(iter.base(), std::end(exps)));
		}

	assert(max_depth > 1 && exps.empty() && 1 == res.size());
#ifdef DEBUG
	std::cout << " max depth: " << max_depth << std::endl;
#endif
	return std::make_pair(res.back(), max_depth);
}

//return the data and max depth (just traveled branches).
template <typename T> inline std::pair<bool, size_t> safe_judge(exp_ctype<T>& exp, const std::function<T(const std::string&)>& cb)
{
	auto re = safe_data(exp, cb);
	return std::make_pair(0 != re.first, re.second);
}

#define TRAVEL_EXP(branch_name) \
{ \
	iter->second = direction++; \
	auto branch = iter->first->branch_name(); \
	if (!branch) \
		; \
	else if (is_composite(branch)) \
	{ \
		exps.emplace_back(branch, -1); \
		iter = exps.rbegin(); \
		direction = 0; \
	} \
	else \
		max_depth = std::max(exps.size() + 1, max_depth); \
}

//return the max depth.
template <typename T> inline size_t safe_delete(exp_ctype<T>& exp)
{
	size_t max_depth = 1;
	//the max depth is the max size that this list ever get, so please use at least gcc 5 since we need std::list::size() to have O(1) complexity
	std::list<std::pair<exp_type<T>, int>> exps; //current branch: -1 - before handling, 0 -road map,  1 - left branch, 2 - right branch
	auto direction = 0; //0 - road map, 1 - left-bottom, 2 - right-bottom, 3 - top-left
	exps.emplace_back(exp, -1);
	for (auto iter = exps.rbegin(); iter != exps.rend();)
		if (0 == direction) //road map
			TRAVEL_EXP(get_road_map)
		else if (1 == direction) //left
			TRAVEL_EXP(get_left_item)
		else if (2 == direction) //right
			TRAVEL_EXP(get_right_item)
		else //3 == direction, backtrace
		{
			iter++->first->clear();
			if (iter != exps.rend())
				direction = iter->second + 1;
			iter = decltype(iter)(exps.erase(iter.base(), std::end(exps)));
		}

	assert(max_depth > 0 && exps.empty() && !exp->get_left_item() && !exp->get_right_item());
	return max_depth;
}
/////////////////////////////////////////////////////////////////////////////////////////

template <typename T = float, typename O = O3> class compiler
{
private:
	struct sub_exp
	{
		std::string name, raw_exp;
		std::vector<std::string> items;
		exp_type<T> parsed_exp;
	};

public:
	static exp_type<T> to_judge_exp(exp_ctype<T>& exp) {return exp->is_data() ? std::make_shared<transparent_judge_exp<T>>(exp) : exp;}
	static exp_type<T> compile(const char* statement) {return compile(std::string(statement));}
	static exp_type<T> compile(const std::string& statement)
	{
		try
		{
			auto expression = statement;
			pre_parse_1(expression);
#ifdef DEBUG
			auto old_exp = expression;
#endif
			std::map<std::string, sub_exp> sub_exps;
			size_t index = 0;
			try
			{
				auto end_index = expression.size();
				auto num = 0;
				pre_parse_2(expression, sub_exps, index, end_index, num, 0);

				auto base_level = 0;
				for (auto first = true; first || pre_parse_3(expression, sub_exps, index, end_index, num, 0, base_level);
					first = false, base_level += 100, index = 0, end_index = expression.size(), num = 0);

				auto tmp_exps = sub_exps;
				for (auto& s_exp : tmp_exps)
				{
					base_level -= 100;
					for (auto first = true; first || pre_parse_3(s_exp.second.raw_exp, sub_exps, index, end_index, num, 0, base_level);
						first = false, base_level += 100, index = 0, end_index = s_exp.second.raw_exp.size(), num = 0)
						if (!first)
							sub_exps[s_exp.first].raw_exp = s_exp.second.raw_exp;
				}
			}
			catch (const std::exception& e) {on_error(expression, index); throw(e);}
			catch (const std::string& e) {on_error(expression, index); throw(e);}
			catch (const char* e) {on_error(expression, index); throw(e);}

			if (!sub_exps.empty())
			{
#ifdef DEBUG
				printf(" get %s from %s, where:\n", expression.data(), old_exp.data());
				for (auto& item : sub_exps)
				{
					printf("  %s = %s\n", item.first.data(), item.second.raw_exp.data());
					item.second.items = split(item.second.raw_exp);
				}
#else
				for (auto& item : sub_exps)
					item.second.items = split(item.second.raw_exp);
#endif
				auto parsed_num = 0;
				do
				{
					parsed_num = 0;
					for (auto& item : sub_exps)
						if (!item.second.parsed_exp && (item.second.parsed_exp = compile(item.second.items, sub_exps)))
							++parsed_num;
				} while (parsed_num > 0);
			}

			auto re = compile(split(expression), sub_exps);
			if (!re)
				throw("incomplete expression!");
			else if (O::level() > 1)
			{
				auto final_re = re->final_optimize();
				if (final_re)
					re = final_re;
#ifdef DEBUG
				printf(" max depth: %d\n immediate values:", re->get_depth());
				re->show_immediate_value();
				putchar('\n');
#endif
			}

			return re;
		}
		catch (const std::exception& e) {printf("\033[31m%s\033[0m\n", e.what());}
		catch (const std::string& e) {printf("\033[31m%s\033[0m\n", e.data());}
		catch (const char* e) {printf("\033[31m%s\033[0m\n", e);}
		catch (...) {puts("\033[31munknown exception happened!\033[0m");}

		return exp_type<T>();
	}

private:
	static void pre_parse_1(std::string& expression)
	{
#ifdef DEBUG
		printf(" pre-parsing phase 1 from [%s] ", expression.data());
#endif
		char blanks[] = {' ', '\t', '\n', '\r'};
		for (const auto& c : blanks)
			for (auto pos = std::string::npos; std::string::npos != (pos = expression.rfind(c, pos));)
				expression.erase(pos, 1);

#ifdef DEBUG
		printf("to [%s]\n", expression.data());
#endif
	}

	static void pre_parse_2(std::string& expression, std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t& end_index, int& p_num, int level)
	{
		if (level >= 100)
			throw("parentheses layer cannot exceed 99!");

		auto p_start = std::string::npos;
		while (index < end_index)
		{
			const auto c = expression[index];
			if ('(' == c)
			{
				if (std::string::npos == p_start)
				{
					p_start = index;
					++p_num;
				}
				else
					pre_parse_2(expression, sub_exps, index, end_index, p_num, level + 1);
			}
			else if (')' == c)
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
				std::string raw_exp = std::string(std::next(expression.data(), p_start + 1), index - p_start - 1);
				sub_exps[name] = sub_exp {name, raw_exp};

				auto old_size = expression.size();
#ifdef DEBUG
				printf(" pre-parsing phase 2 from [%s] ", expression.data());
				expression.replace(p_start, raw_exp.size() + 2, name);
				printf("to [%s]\n", expression.data());
#else
				expression.replace(p_start, raw_exp.size() + 2, name);
#endif
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
			const auto c = expression[index];
			if ('?' == c)
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
			else if (':' == c || index + 1 == end_index)
			{
				auto is_c = ':' == c;
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
#ifdef DEBUG
					printf(" pre-parsing phase 3 from [%s] ", expression.data());
					expression.replace(start, index - start, name);
					printf("to [%s]\n", expression.data());
#else
					expression.replace(start, index - start, name);
#endif
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
					items.emplace_back(start, input - start);

				items.emplace_back(input++, 2);
				start = input + 1;
			}
			else if (is_key_1(input))
			{
				if (start < input)
					items.emplace_back(start, input - start);

				items.emplace_back(input, 1);
				start = input + 1;
			}
		if (start < input)
			items.emplace_back(start, input - start);

		return items;
	}

	static void finish_data_exp(exp_type<T>& data_1, exp_type<T>&& data_2, std::string& op_1, const std::string& op_2 = std::string())
	{
		if (!data_1)
			throw("missing (logical) operand!");
		else if (!op_2.empty())
			throw("missing operand!");
		else if (data_2)
		{
			if (op_1.empty())
				throw("missing operator!");

			data_1 = qme::merge_data_exp<T, O>(data_1, data_2, op_1);
			op_1.clear();
			data_2.reset();
		}
		else if (!op_1.empty())
			throw("missing operand!");
	}

	static void merge_data_exp(exp_type<T>& data_1, exp_type<T>& data_2, std::string& op_1, std::string& op_2, exp_type<T>&& data)
	{
		assert(data);
		if (data_2)
			finish_data_exp(data_2, std::forward<exp_type<T>>(data), op_2);
		else if (data_1)
		{
			assert(op_2.empty());
			if (is_operator_1(op_1))
				data_2 = data;
			else
				finish_data_exp(data_1, std::forward<exp_type<T>>(data), op_1);
		}
		else
		{
			assert(op_1.empty() && op_2.empty());
			data_1 = data;
		}
	}

	static void finish_judge_exp(exp_type<T>& judge_1, exp_type<T>&& judge_2, std::string& lop_1, const std::string& lop_2 = std::string())
	{
		if (!lop_2.empty() || !judge_1)
			throw("missing logical operand!");
		else if (judge_2)
		{
			if (lop_1.empty())
				throw("missing logical operator!");

			judge_1 = make_logical_exp(judge_1, judge_2, lop_1);
			lop_1.clear();
			judge_2.reset();
		}
		else if (!lop_1.empty())
			throw("missing logical operand!");
	}

	static void merge_judge_exp(exp_type<T>& judge_1, exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2, exp_type<T>&& judge)
	{
		assert(judge);
		if (judge_2)
			finish_judge_exp(judge_2, std::forward<exp_type<T>>(judge), lop_2);
		else if (judge_1)
		{
			assert(lop_2.empty());
			if ("||" == lop_1)
				judge_2 = judge;
			else
				finish_judge_exp(judge_1, std::forward<exp_type<T>>(judge), lop_1);
		}
		else
		{
			assert(lop_1.empty() && lop_2.empty());
			judge_1 = judge;
		}
	}

	static void finish_data_and_merge_judge_exp(exp_type<T>& dc, std::string& c,
		exp_type<T>& data_1, exp_type<T>& data_2, std::string& op_1, std::string& op_2,
		exp_type<T>& judge_1, exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2)
	{
		if (data_1)
			finish_data_exp(data_1, std::move(data_2), op_1, op_2);

		if (!c.empty() || dc)
		{
			assert(!c.empty() && dc);
			if (data_1)
				merge_judge_exp(judge_1, judge_2, lop_1, lop_2, make_binary_judge_exp(dc, data_1, c));
			else if (judge_1)
			{
				if (judge_2 || !lop_1.empty() || !lop_2.empty())
					throw ("missing logical operand!");

				judge_1 = make_binary_judge_exp(dc, judge_1, c);
			}
			dc.reset();
			c.clear();
		}
		else if (data_1)
			merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::forward<exp_type<T>>(data_1));

		data_1.reset();
	}

	static void finish_data_and_judge_exp(exp_type<T>& dc, std::string& c, bool to_data,
		exp_type<T>& data_1, exp_type<T>& data_2, std::string& op_1, std::string& op_2,
		exp_type<T>& judge_1, exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2)
	{
		if (!c.empty() || dc || !data_1 || judge_1)
		{
			finish_data_and_merge_judge_exp(dc, c, data_1, data_2, op_1, op_2, judge_1, judge_2, lop_1, lop_2);
			finish_judge_exp(judge_1, std::move(judge_2), lop_1, lop_2);
			if (to_data)
			{
				data_1 = judge_1;
				judge_1.reset();
			}
		}
		else if (data_1)
			finish_data_exp(data_1, std::move(data_2), op_1, op_2);
	}

	static exp_type<T> bang(exp_ctype<T>& exp)
	{
		if (O::level() < 2 && exp->is_composite() && (exp->get_left_item()->is_composite() || exp->get_right_item()->is_composite()))
			return std::make_shared<not_judge_exp<T>>(exp);
		return exp->bang();
	}

	static exp_type<T> to_negative(exp_ctype<T>& exp)
	{
		if (O::level() < 2 && exp->is_composite() && (exp->get_left_item()->is_composite() || exp->get_right_item()->is_composite()))
			return std::make_shared<negative_data_exp<T>>(exp);
		return exp->to_negative();
	}

	static exp_type<T> compile(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps)
	{
		size_t index = 0, end_index = items.size();
		try {return compile(items, sub_exps, index, end_index);}
		catch (const std::exception& e) {on_error(items, index); throw(e);}
		catch (const std::string& e) {on_error(items, index); throw(e);}
		catch (const char* e) {on_error(items, index); throw(e);}
	}

	static data_exp_type<T> parse_data(const std::string& vov)
	{
		if (is_key_2(vov) || is_key_1(vov))
			throw("unexpected " + vov);
		else if (0 == isdigit(vov[0])) //variable
		{
			if (O::level() < 2)
				return std::make_shared<variable_data_exp<T>>(vov);
			return std::make_shared<composite_variable_data_exp<T, O>>(vov);
		}

		T value;
		char* endptr;
		errno = 0;
		if (std::is_same<T, float>::value || std::is_same<T, double>::value)
			value = (T) strtod(vov.data(), &endptr); //(T) atof(vov.data())
		else
		{
			value = (T) strtoll(vov.data(), &endptr, 0); //(T) atoll(vov.data())
			if (0 == errno && '\0' != *endptr)
				value = (T) strtod(vov.data(), &endptr); //(T) atof(vov.data())
		}
		if (0 != errno || '\0' != *endptr)
			throw("invalid immediate data " + vov);

		return std::make_shared<immediate_data_exp<T>>(value);
	}

	static void merge_unary_operator(char& last_operator, int& negative, int& revert, char op)
	{
		assert('!' == op || is_operator_1(op));

		if ('!' == op)
			++revert;
		else if (op == last_operator)
			throw("redundant " + std::string(1, op) + " operator!");
		else if (0 == revert) //!a always equals to !+a and !-a no matter a is data or judgement, so following + and - can be ignored
		{
			if (negative < 0)
				negative = 0; //keep leading +/- operator so we can transform judgement to data
			if ('-' == op) //leading + is useless except to transform judgement to data
				++negative;
		}
		last_operator = op;
	}

	static exp_type<T> compile(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t end_index)
	{
		if (index >= end_index)
			throw("empty expresson!");

		auto last_operator = '\0'; //+-!
		auto negative = -1; //number of -, 0 means at least one +
		auto revert = 0; //number of !

		exp_type<T> data_1, data_2; //treat as data (which means they can be judgements)
		std::string op_1, op_2;

		exp_type<T> dc; //merged comparand (from data_1 and data_2 with op_1)
		std::string c; //comparer

		exp_type<T> judge_1, judge_2; //treat as judge (which means they can be data)
		std::string lop_1, lop_2;

		exp_type<T> fj; //for final question exp, treat as judge (which means they can be data)
		exp_type<T> fd_1, fd_2; //for final question exp, treat as data (which means they can be judgements)

		while (index < end_index)
		{
			const auto& item = items[index];
			if ("!" == item)
			{
				merge_unary_operator(last_operator, negative, revert, '!');
				++index;
				continue;
			}
			else if (is_operator(item))
			{
				exp_type<T> data;
				if (judge_2)
				{
					if (lop_2.empty())
					{
						data = judge_2;
						judge_2.reset();
					}
				}
				else if (judge_1 && lop_1.empty())
				{
					data = judge_1;
					judge_1.reset();
				}

				if (data)
				{
					assert(!data_1 && !data_2);
					data_1.swap(data);
				}

				auto check = false;
				if (data_2)
				{
					if (!op_2.empty())
						check = true;
					else if (is_operator_1(item))
					{
						finish_data_exp(data_1, std::move(data_2), op_1);
						op_1 = item;
					}
					else
						op_2 = item;
				}
				else if (data_1)
				{
					if (!op_1.empty())
						check = true;
					else
						op_1 = item;
				}
				else if (is_operator_2(item))
					throw("missing operand!");
				else
					check = true;

				if (check)
				{
					if (!is_operator_1(item))
						throw("redundant operand!");

					merge_unary_operator(last_operator, negative, revert, item[0]);
					++index;
					continue;
				}
			}
			else if (is_comparer(item))
			{
				if (!c.empty() || dc)
					throw("redundant comparer!");
				else if (!data_1)
				{
					assert(!data_2 && op_1.empty() && op_2.empty());
					if (judge_2)
					{
						assert(judge_1 && !lop_1.empty());
						if (!lop_2.empty())
							throw("missing operand!");

						data_1 = judge_2;
						judge_2.reset();
					}
					else if (judge_1)
					{
						assert(lop_2.empty());
						if (!lop_1.empty())
							throw("missing operand!");

						data_1 = judge_1;
						judge_1.reset();
					}
				}

				finish_data_exp(data_1, std::move(data_2), op_1, op_2);
				dc = std::move(data_1);
				c = item;
			}
			else if (is_logical_operator(item))
			{
				finish_data_and_merge_judge_exp(dc, c, data_1, data_2, op_1, op_2, judge_1, judge_2, lop_1, lop_2);
				if (judge_2)
				{
					if (!lop_2.empty())
						throw("redundant logical operator!");
					else if ("||" == item)
					{
						finish_judge_exp(judge_1, std::move(judge_2), lop_1, lop_2);
						lop_1 = item;
					}
					else
						lop_2 = item;
				}
				else if (judge_1)
				{
					if (!lop_1.empty())
						throw("redundant logical operator!");
					else
						lop_1 = item;
				}
				else
					throw("missing logical operand!");
			}
			else if ("?" == item)
			{
				if (fj)
					throw("redundant ? operator!");

				finish_data_and_merge_judge_exp(dc, c, data_1, data_2, op_1, op_2, judge_1, judge_2, lop_1, lop_2);
				finish_judge_exp(judge_1, std::move(judge_2), lop_1, lop_2);
				fj.swap(judge_1);
			}
			else if (":" == item)
			{
				if (!fj)
					throw("missing ? operator!");
				else if (fd_1)
					throw("redundant : operator!");

				finish_data_and_judge_exp(dc, c, true, data_1, data_2, op_1, op_2, judge_1, judge_2, lop_1, lop_2);
				fd_1.swap(data_1);
			}
			else
			{
				exp_type<T> parsed_exp;
				if ('$' == item[0])
				{
					auto iter = sub_exps.find(item);
					if (iter == std::end(sub_exps))
						throw("undefined symbol " + item);
					else if (!(parsed_exp = iter->second.parsed_exp)) //dependancy not ready
						return parsed_exp;
				}
				else
					parsed_exp = parse_data(item);

				auto is_judge = !parsed_exp->is_data();
				if (revert > 0)
				{
					is_judge = true;
					parsed_exp = 1 == (revert & 1) ? bang(parsed_exp) : to_judge_exp(parsed_exp);
				}
				if (negative >= 0)
				{
					is_judge = false;
					if (1 == (negative & 1))
						parsed_exp = to_negative(parsed_exp);
				}
				last_operator = '\0';
				negative = -1;
				revert = 0;

				if (!dc && !data_1 && is_judge)
					merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::move(parsed_exp));
				else
					merge_data_exp(data_1, data_2, op_1, op_2, std::move(parsed_exp));
			}

			if (negative >= 0)
				throw("unexpected +/- operator!");
			else if (revert > 0)
				throw("unexpected ! operator!");

			++index;
		}

		auto to_data = fj && fd_1;
		finish_data_and_judge_exp(dc, c, to_data, data_1, data_2, op_1, op_2, judge_1, judge_2, lop_1, lop_2);
		if (to_data)
		{
			assert(!fd_2);
			fd_2.swap(data_1);
		}

		assert(c.empty() && op_1.empty() && op_2.empty() && lop_1.empty() && lop_2.empty());
		assert(!dc && !data_2 && !judge_2);
		if (fj)
		{
			if (!fd_1 || !fd_2)
				throw("incomplete question exp!");

			assert(!data_1 && !judge_1);
			return std::make_shared<question_exp<T>>(fj, fd_1, fd_2);
		}
		else if (data_1)
		{
			assert(!judge_1);
			return data_1;
		}
		else if (judge_1)
			return judge_1;
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
