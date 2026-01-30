
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

//without any exchange between any data, nor merging of immediate values
class O0 {public: static int level() {return 0;}};

//without any exchange between any data, but merge adjacent immediate values
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

class exp
{
protected:
	virtual ~exp() {}

public:
	virtual bool is_data() const {return false;}
	virtual bool is_judge() const {return false;}
	virtual bool is_composite() const {return false;}
	virtual int get_depth() const {return 1;}
	virtual void show_immediate_value() const {}

private:
	template <typename T, template<typename> class Exp> friend inline void safe_delete(const std::shared_ptr<Exp<T>>&);
	virtual void safe_delete() const {}
	virtual void clear() {}
};

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> class data_exp;
template <typename T> using data_exp_type = std::shared_ptr<data_exp<T>>;
template <typename T> using data_exp_ctype = const data_exp_type<T>;
template <typename T> inline T safe_execute(data_exp_ctype<T>&, const std::function<T(const std::string&)>&);
template <typename T> class data_exp : public exp
{
public:
	virtual bool is_data() const {return true;}
	virtual bool is_immediate() const {return false;}
	virtual bool is_composite_variable() const {return false;}
	//whether this expression can be transformed to negative
	// 1 - without introducing negation operations, for example 2 * a to -2 *a
	// 2 - with reducing existed negation operations, for example -a to a
	virtual bool is_easy_to_negative() const {return false;}
	virtual bool is_negative() const {return false;} //needs negation operation at runtime
	virtual int get_exponent() const {throw("unsupported get exponent operation!");}
	virtual T get_multiplier() const {throw("unsupported get multiplier operation!");}
	virtual T get_immediate_value() const {throw("unsupported get immediate value operation!");}
	virtual const std::string& get_variable_name() const {throw("unsupported get variable name operation!");}
	virtual char get_operator() const {throw("unsupported get operator operation!");}
	virtual data_exp_ctype<T>& get_left_item() const {throw("unsupported get left item operation!");}
	virtual data_exp_ctype<T>& get_right_item() const {throw("unsupported get right item operation!");}

	virtual bool merge_with(char, data_exp_ctype<T>&) {return false;}
	virtual bool merge_with(data_exp_ctype<T>&, char) {return false;}
	virtual data_exp_type<T> trim_myself() {return data_exp_type<T>();}
	virtual data_exp_type<T> to_negative() const = 0;
	virtual data_exp_type<T> final_optimize() const {return data_exp_type<T>();}

	virtual T operator()(const std::function<T(const std::string&)>&) const = 0;

private:
	friend T safe_execute<T>(data_exp_ctype<T>&, const std::function<T(const std::string&)>&);
	virtual T safe_execute(const std::function<T(const std::string&)>& cb) const {return operator()(cb);}
};
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> inline bool is_same_composite_variable(const std::string& variable_name, data_exp_ctype<T>& other_exp)
	{return other_exp->is_composite_variable() && variable_name == other_exp->get_variable_name();}

template <typename T> inline bool is_same_composite_variable(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r)
	{return dexp_l->is_composite_variable() && is_same_composite_variable(dexp_l->get_variable_name(), dexp_r);}

template <typename T> inline bool is_divisible(T dividend, T divisor)
{
	if (0 == divisor)
		throw("divide zero");
	return 0 == dividend || 1 == divisor || -1 == divisor || dividend == divisor || -dividend == divisor;
}

template <typename T> class immediate_data_exp : public data_exp<T>
{
public:
	immediate_data_exp() : T(0) {}
	immediate_data_exp(T v) : value(v) {}

	virtual bool is_immediate() const {return true;}
	virtual bool is_easy_to_negative() const {return true;}
	virtual T get_immediate_value() const {return value;}
	virtual void show_immediate_value() const {std::cout << ' ' << value;}

	virtual bool merge_with(char other_op, data_exp_ctype<T>& other_exp)
		{return other_exp->is_immediate() ? merge(other_op, other_exp->get_immediate_value()) : false;}
	virtual data_exp_type<T> to_negative() const {return std::make_shared<immediate_data_exp<T>>(-value);}

	virtual T operator()(const std::function<T(const std::string&)>&) const
	{
#ifdef DEBUG
		std::cout << " get immediate value " << value << std::endl;
#endif
		return value;
	}

	operator T() const {return value;}
	bool merge(char other_op, T v)
	{
		switch (other_op)
		{
		case '+':
			value += v;
			break;
		case '-':
			value -= v;
			break;
		case '*':
			value *= v;
			break;
		case '/':
			if (0 == v)
				throw("divide zero");
			value /= v;
			break;
		default:
			throw("undefined operator " + std::string(1, other_op));
			break;
		}

		return true;
	}

private:
	T value;
};

template <typename T, typename O> class composite_variable_data_exp;
template <typename T, typename O> inline data_exp_type<T> merge_data_exp(data_exp_ctype<T>&, data_exp_ctype<T>&, char);
template <typename T, typename O> class binary_data_exp : public data_exp<T>
{
protected:
	binary_data_exp(data_exp_ctype<T>& _dexp_l, data_exp_ctype<T>& _dexp_r, char _op) : op(_op), dexp_l(_dexp_l), dexp_r(_dexp_r) {}

public:
	virtual bool is_composite() const {return true;}
	virtual bool is_easy_to_negative() const
		{return is_negative() || (is_operator_2(op) && (dexp_l->is_easy_to_negative() || dexp_r->is_easy_to_negative()));}
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
		switch (op)
		{
		case '-':
			return dexp_l->is_negative();
			break;
		case '*':
		case '/':
			return dexp_l->is_negative() || dexp_r->is_negative();
			break;
		}

		return false;
	}
	virtual int get_depth() const {return 1 + std::max(dexp_l->get_depth(), dexp_r->get_depth());}
	virtual char get_operator() const {return op;}
	virtual data_exp_ctype<T>& get_left_item() const {return dexp_l;}
	virtual data_exp_ctype<T>& get_right_item() const {return dexp_r;}
	virtual void show_immediate_value() const {dexp_l->show_immediate_value(); dexp_r->show_immediate_value();}

	virtual bool merge_with(char other_op, data_exp_ctype<T>& other_exp)
	{
		if (is_same_operator_level<O>(op, other_op))
		{
			if (O::level() < 2)
			{
				if (!other_exp->is_immediate())
					return false;
				else if (dexp_l->is_immediate())
					return dexp_l->merge_with(other_op, other_exp);
				else if (!dexp_r->is_immediate())
					return false;

				if ('-' == op)
					other_op = '+' == other_op ? '-' : '+';
				else if ('/' == op) //other_op must also be '/'
					other_op = '*';
				return dexp_r->merge_with(other_op, other_exp);
			}
			else if (dexp_l->merge_with(other_op, other_exp))
				return true;
			else if ('+' == op || '*' == op)
				return dexp_r->merge_with(other_op, other_exp);
			else if ('-' == op)
				other_op = '+' == other_op ? '-' : '+';
			else if ('/' == op)
				other_op = '*' == other_op ? '/' : '*';
			return dexp_r->merge_with(other_op, other_exp);
		}
		else if (2 == O::level() && '+' == other_op && '/' == op && //'N1*a^M / C + N2*a^M' -> '(N1 + N2*C)*a^M / C'
			dexp_r->is_immediate() && is_same_composite_variable(dexp_l, other_exp) &&
			dexp_l->get_exponent() == other_exp->get_exponent())
		{
			dexp_l = std::make_shared<composite_variable_data_exp<T, O>>(dexp_l->get_variable_name(),
				dexp_l->get_multiplier() + other_exp->get_multiplier() * dexp_r->get_immediate_value(), dexp_l->get_exponent());
			return true;
		}

		return false;
	}

	virtual data_exp_type<T> trim_myself()
	{
		auto data = dexp_l->trim_myself();
		if (data)
			dexp_l = data;

		data = dexp_r->trim_myself();
		if (data)
			dexp_r = data;

		switch (op)
		{
		case '+':
			if (dexp_l->is_immediate() && 0 == dexp_l->get_immediate_value())
				return dexp_r;
			else if (dexp_r->is_immediate() && 0 == dexp_r->get_immediate_value())
				return dexp_l;
			else if (dexp_r->is_negative())
				return merge_data_exp<T, O>(dexp_l, dexp_r->to_negative(), '-');
			else if (dexp_l->is_negative())
				return merge_data_exp<T, O>(dexp_r, dexp_l->to_negative(), '-');
			break;
		case '-':
			if (dexp_l->is_immediate() && 0 == dexp_l->get_immediate_value())
				return dexp_r->to_negative();
			else if (dexp_r->is_immediate() && 0 == dexp_r->get_immediate_value())
				return dexp_l;
			else if (dexp_r->is_negative())
				return merge_data_exp<T, O>(dexp_l, dexp_r->to_negative(), '+');
			else if (dexp_l->is_negative() && dexp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(dexp_r->to_negative(), dexp_l->to_negative(), '-');
			break;
		case '*':
			if (dexp_l->is_immediate())
			{
				auto v = dexp_l->get_immediate_value();
				if (1 == v)
					return dexp_r;
				else if (0 == v)
					return dexp_l;
				else if (-1 == v)
					return dexp_r->to_negative();
			}
			else if (dexp_r->is_immediate())
			{
				auto v = dexp_r->get_immediate_value();
				if (1 == v)
					return dexp_l;
				else if (0 == v)
					return dexp_r;
				else if (-1 == v)
					return dexp_l->to_negative();
			}
			if ((dexp_l->is_negative() && (dexp_r->is_negative() || dexp_r->is_easy_to_negative())) ||
				(dexp_r->is_negative() && (dexp_l->is_negative() || dexp_l->is_easy_to_negative())))
				return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r->to_negative(), '*');
			break;
		case '/':
			if (dexp_l->is_immediate())
			{
				if (0 == dexp_l->get_immediate_value())
					return dexp_l;
			}
			else if (dexp_r->is_immediate())
			{
				auto v = dexp_r->get_immediate_value();
				if (1 == v)
					return dexp_l;
				else if (0 == v)
					throw("divide zero");
				else if (-1 == v)
					return dexp_l->to_negative();
			}
			if ((dexp_l->is_negative() && (dexp_r->is_negative() || dexp_r->is_easy_to_negative())) ||
				(dexp_r->is_negative() && (dexp_l->is_negative() || dexp_l->is_easy_to_negative())))
				return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r->to_negative(), '/');
			else if (2 == O::level() && is_same_composite_variable(dexp_l, dexp_r))
			{
				//'N1*a^M1 / N2*a^M2' -> 'N1*a^(M1-M2) / N2' where M1 > M2 > 0
				//for other conditions, we will handle them in composite_variable_data_exp's:
				// merge_with, or
				// trim_myself, or
				// final_optimize, or
				// cannot handle them with optimization level < 3
				auto exponent_l = dexp_l->get_exponent(), exponent_r = dexp_r->get_exponent();
				if (exponent_l > exponent_r && exponent_r > 0)
				{
					dexp_l = std::make_shared<composite_variable_data_exp<T, O>>(dexp_l->get_variable_name(),
						dexp_l->get_multiplier(), exponent_l - exponent_r);
					dexp_r = std::make_shared<immediate_data_exp<T>>(dexp_r->get_multiplier());
					return trim_myself();
				}
			}
			break;
		}

		return data_exp_type<T>();
	}

	virtual data_exp_type<T> to_negative() const
	{
		switch (op)
		{
		case '+':
			if (dexp_l->is_easy_to_negative())
				return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r, '-');
			else if (dexp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(dexp_r->to_negative(), dexp_l, '-');
			else
			{
				auto depth_l = dexp_l->get_depth(), depth_r = dexp_r->get_depth();
				if (depth_l <= depth_r)
					return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r, '-');
				else
					return merge_data_exp<T, O>(dexp_r->to_negative(), dexp_l, '-');
			}
			break;
		case '-':
			//without this branch, '-a - b' will be transformed to 'b + a' (from b - -a) instead of 'a + b', the former looks strange
			if (dexp_l->is_negative())
				return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r, '+');
			return merge_data_exp<T, O>(dexp_r, dexp_l, '-');
			break;
		case '*':
		case '/':
			if (dexp_l->is_easy_to_negative())
				return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r, op);
			else if (dexp_r->is_easy_to_negative())
				return merge_data_exp<T, O>(dexp_l, dexp_r->to_negative(), op);
			else
			{
				auto depth_l = dexp_l->get_depth(), depth_r = dexp_r->get_depth();
				if (depth_l <= depth_r)
					return merge_data_exp<T, O>(dexp_l->to_negative(), dexp_r, op);
				else
					return merge_data_exp<T, O>(dexp_l, dexp_r->to_negative(), op);
			}
			break;
		}

		return data_exp_type<T>();
	}

	virtual data_exp_type<T> final_optimize() const
	{
		auto l = dexp_l->final_optimize();
		auto r = dexp_r->final_optimize();

		return (l || r) ? merge_data_exp<T, O>(l ? l : dexp_l, r ? r : dexp_r, op) : data_exp_type<T>();
	}

private:
	virtual T safe_execute(const std::function<T(const std::string&)>&) const {throw("unsupported safe execute operation!");}
	virtual void clear() {dexp_l.reset(); dexp_r.reset();}

private:
	char op;
	data_exp_type<T> dexp_l, dexp_r;
};

template <typename T, typename O> class add_data_exp : public binary_data_exp<T, O>
{
public:
	add_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_data_exp<T, O>(dexp_l, dexp_r, '+') {}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
		{return (*binary_data_exp<T, O>::get_left_item())(cb) + (*binary_data_exp<T, O>::get_right_item())(cb);}
};

template <typename T, typename O> class sub_data_exp : public binary_data_exp<T, O>
{
public:
	sub_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_data_exp<T, O>(dexp_l, dexp_r, '-') {}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
		{return (*binary_data_exp<T, O>::get_left_item())(cb) - (*binary_data_exp<T, O>::get_right_item())(cb);}
};

template <typename T, typename O> class multi_data_exp : public binary_data_exp<T, O>
{
public:
	multi_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_data_exp<T, O>(dexp_l, dexp_r, '*') {}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
		{return (*binary_data_exp<T, O>::get_left_item())(cb) * (*binary_data_exp<T, O>::get_right_item())(cb);}
};

template <typename T, typename O> class div_data_exp : public binary_data_exp<T, O>
{
public:
	div_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_data_exp<T, O>(dexp_l, dexp_r, '/') {}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
	{
#ifndef _MSC_VER //keep the order of data fetching for debuging
		auto dividend = (*binary_data_exp<T, O>::get_left_item())(cb);
#endif
		auto divisor = (*binary_data_exp<T, O>::get_right_item())(cb);
		if (0 == divisor)
			throw("divide zero");
#ifdef _MSC_VER
		return (*binary_data_exp<T, O>::get_left_item())(cb) / divisor;
#else
		return dividend / divisor;
#endif
	}
};

template <typename T> class negative_variable_data_exp;
template <typename T> class variable_data_exp : public data_exp<T>
{
public:
	variable_data_exp(const std::string& _variable_name) : variable_name(_variable_name) {}

	virtual data_exp_type<T> to_negative() const {return std::make_shared<negative_variable_data_exp<T>>(variable_name);}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return v;
#endif
		return cb(variable_name);
	}

protected:
	data_exp_type<T> clone() const {return std::make_shared<variable_data_exp<T>>(variable_name);}

private:
	std::string variable_name;
};

template <typename T> class negative_variable_data_exp : public variable_data_exp<T>
{
public:
	negative_variable_data_exp(const std::string& variable_name) : variable_data_exp<T>(variable_name) {}

	virtual bool is_easy_to_negative() const {return true;}
	virtual bool is_negative() const {return true;}

	virtual data_exp_type<T> to_negative() const {return variable_data_exp<T>::clone();}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const {return -variable_data_exp<T>::operator()(cb);}
};

template <typename T> class negative_exponent_data_exp;
template <typename T> class exponent_data_exp : public data_exp<T>
{
public:
	exponent_data_exp(const std::string& _variable_name, int _exponent) : variable_name(_variable_name), exponent(_exponent) {}

	virtual void show_immediate_value() const {std::cout << ' ' << exponent;}

	virtual data_exp_type<T> to_negative() const {return std::make_shared<negative_exponent_data_exp<T>>(variable_name, exponent);}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return (T) pow(v, exponent);
#endif
		return (T) pow(cb(variable_name), exponent);
	}

protected:
	data_exp_type<T> clone() const {return std::make_shared<exponent_data_exp<T>>(variable_name, exponent);}

private:
	std::string variable_name;
	int exponent;
};

template <typename T> class negative_exponent_data_exp : public exponent_data_exp<T>
{
public:
	negative_exponent_data_exp(const std::string& variable_name, int exponent) : exponent_data_exp<T>(variable_name, exponent) {}

	virtual bool is_easy_to_negative() const {return true;}
	virtual bool is_negative() const {return true;}

	virtual data_exp_type<T> to_negative() const {return exponent_data_exp<T>::clone();}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const {return -exponent_data_exp<T>::operator()(cb);}
};

template <typename T, typename O> class composite_variable_data_exp : public data_exp<T>
{
public:
	composite_variable_data_exp(const std::string& _variable_name, T _multiplier = 1, int _exponent = 1)
		: variable_name(_variable_name), multiplier(_multiplier), exponent(_exponent) {}

	virtual bool is_composite_variable() const {return true;}
	virtual bool is_easy_to_negative() const {return true;}
	virtual int get_exponent() const {return exponent;}
	virtual T get_multiplier() const {return multiplier;}
	virtual const std::string& get_variable_name() const {return variable_name;}
	virtual void show_immediate_value() const {std::cout << ' ' << multiplier << ' ' << exponent;}

	virtual bool merge_with(char other_op, data_exp_ctype<T>& other_exp)
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

	//handle 'C * Na^M' and 'C / Na^M' only
	virtual bool merge_with(data_exp_ctype<T>& other_exp, char other_op)
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

	virtual data_exp_type<T> trim_myself()
	{
		if (0 == multiplier || 0 == exponent)
			return std::make_shared<immediate_data_exp<T>>(multiplier);
		return data_exp_type<T>();
	}

	virtual data_exp_type<T> to_negative() const
		{return std::make_shared<composite_variable_data_exp<T, O>>(variable_name, -multiplier, exponent);}

	virtual data_exp_type<T> final_optimize() const
	{
		data_exp_type<T> data = std::make_shared<immediate_data_exp<T>>(multiplier);
		if (0 == multiplier || 0 == exponent)
			return data;
		else if (1 != multiplier && -1 != multiplier && exponent > 1)
			return data_exp_type<T>();
		else if (1 == multiplier && exponent < 0)
			return std::make_shared<exponent_data_exp<T>>(variable_name, exponent);
		else if (-1 == multiplier && exponent < 0)
			return std::make_shared<negative_exponent_data_exp<T>>(variable_name, exponent);
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

	virtual T operator()(const std::function<T(const std::string&)>& cb) const
	{
#ifdef DEBUG
		auto v = cb(variable_name);
		std::cout << " get " << variable_name << " returns " << v << std::endl;
		return multiplier * (T) pow(v, exponent);
#endif
		return multiplier * (T) pow(cb(variable_name), exponent);
	}

private:
	std::string variable_name;
	T multiplier;
	int exponent;
};

template <typename T, typename O>
inline data_exp_type<T> direct_merge_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r, char op)
{
	switch (op)
	{
	case '+':
		return std::make_shared<add_data_exp<T, O>>(dexp_l, dexp_r);
		break;
	case '-':
		return std::make_shared<sub_data_exp<T, O>>(dexp_l, dexp_r);
		break;
	case '*':
		return std::make_shared<multi_data_exp<T, O>>(dexp_l, dexp_r);
		break;
	case '/':
		return std::make_shared<div_data_exp<T, O>>(dexp_l, dexp_r);
		break;
	default:
		throw("undefined operator " + std::string(1, op));
		break;
	}
}

template <typename T, typename O> inline data_exp_type<T> merge_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r, char op)
{
	if (0 == O::level())
		return direct_merge_data_exp<T, O>(dexp_l, dexp_r, op);
	else if (dexp_r->merge_with(dexp_l, op)) //parse 'C * Na^M' and 'C / Na^M' to composite_variable_data_exp instead of binary_data_exp
	{
		auto data = dexp_r->trim_myself();
		return data ? data : dexp_r;
	}
	else if (dexp_l->merge_with(op, dexp_r)) //composite_variable_data_exp is involved here
	{
		auto data = dexp_l->trim_myself();
		return data ? data : dexp_l;
	}
	else if (dexp_r->is_composite())
	{
		auto op_2 = dexp_r->get_operator();
		if (is_same_operator_level<O>(op, op_2))
		{
			data_exp_type<T> data;
			if (dexp_l->merge_with(op, dexp_r->get_left_item()))
			{
				data = dexp_l->trim_myself();
				if (!data)
					data = dexp_l;
			}

			if ('-' == op)
				op_2 = '-' == op_2 ? '+' : '-';
			else if ('/' == op)
				op_2 = '/' == op_2 ? '*' : '/';
			if (!data)
			{
				if (dexp_l->merge_with(op_2, dexp_r->get_right_item()))
				{
					data = dexp_l->trim_myself();
					if (!data)
						data = dexp_l;

					return merge_data_exp<T, O>(data, dexp_r->get_left_item(), op);
				}
				else
					data = merge_data_exp<T, O>(dexp_l, dexp_r->get_left_item(), op);
			}

			return merge_data_exp<T, O>(data, dexp_r->get_right_item(), op_2);
		}
		else if (2 == O::level() && '+' == op && '/' == op_2 && //'N1*a^M + N2*a^M / C' -> '(N1*C + N2)*a^M / C'
			dexp_r->get_right_item()->is_immediate() && is_same_composite_variable(dexp_l, dexp_r->get_left_item()) &&
			dexp_l->get_exponent() == dexp_r->get_left_item()->get_exponent())
		{
			auto multiplier = dexp_l->get_multiplier() * dexp_r->get_right_item()->get_immediate_value() +
				dexp_r->get_left_item()->get_multiplier();
			return merge_data_exp<T, O>(
				std::make_shared<composite_variable_data_exp<T, O>>(dexp_l->get_variable_name(), multiplier, dexp_l->get_exponent()),
				dexp_r->get_right_item(), '/');
		}
	}

	auto data = direct_merge_data_exp<T, O>(dexp_l, dexp_r, op);
	auto re = data->trim_myself();
	return re ? re : data;
}
template <typename T, typename O>
inline data_exp_type<T> merge_data_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r, const std::string& op)
	{return merge_data_exp<T, O>(dexp_l, dexp_r, *op.data());}

//since recursion is used during the whole compilation and execution, if your expression is too complicated to
// be compiled and executed (stack overflow), use
// qme::O0/qme::O1 to compile it,
// qme::safe_execute to execute it and
// qme::safe_delete to delete it,
// then no recursion will be introduced (except question mark expression used as sub expression).
//with optimization level qme::O0/qme::O1, following functions are still available, if you're encountering above situation,
// you should not call them manually, please note:
// is_easy_to_negative
// is_negative
// get_depth
// show_immediate_value
// merge_with
// trim_myself
// final_optimize
// to_negative
//return the depth of this expression (not like the get_depth interface, recursion is not applied during the calculation)
// to get the max depth, the short_circuit_controller must always returns false, please note.
template <typename T> inline int travel_exp(const T& exp,
	const std::function<void(const T&)>& left_handler,
	const std::function<bool(const T&)>& short_circuit_controller, //false - continue, true - backtrack
	const std::function<void(const T&)>& right_handler,
	const std::function<void(const T&)>& backtrack_handler)
{
	if (!exp->is_composite())
		return left_handler(exp), 1;

	auto depth = 1, max_depth = 0;
	std::list<std::pair<T, bool>> exps; //true - left branch, false - right branch
	exps.emplace_back(exp, true);
	auto direction = 0; //0 - left-bottom, 1 - right-bottom, 2 - top-left
	for (auto iter = exps.crbegin(); iter != exps.crend();)
		if (0 == direction)
		{
			++depth;
			auto left = iter->first->get_left_item();
			if (left->is_composite())
			{
				exps.emplace_back(left, true);
				iter = exps.crbegin();
			}
			else
			{
				left_handler(left);
				direction = 1;
				max_depth = std::max(depth--, max_depth);
			}
		}
		else if (1 == direction)
		{
			++depth;
			if (short_circuit_controller(iter->first))
			{
				direction = 2;
				max_depth = std::max(depth--, max_depth);
			}
			else
			{
				auto right = iter->first->get_right_item();
				if (right->is_composite())
				{
					exps.emplace_back(right, false);
					iter = exps.crbegin();
					direction = 0;
				}
				else
				{
					right_handler(right);
					direction = 2;
					max_depth = std::max(depth--, max_depth);
				}
			}
		}
		else //2 == direction
		{
			--depth;
			backtrack_handler(iter->first);
			if (iter++->second)
			{
				iter = decltype(iter)(exps.erase(iter.base(), std::end(exps)));
				direction = 1;
			}
		}

	return max_depth;
}

template <typename T> inline T safe_execute(data_exp_ctype<T>& dexp, const std::function<T(const std::string&)>& cb)
{
	std::list<immediate_data_exp<T>> res;
	travel_exp<data_exp_type<T>>(dexp,
		[&](data_exp_ctype<T>& left) {res.emplace_back(left->safe_execute(cb));},
		[](data_exp_ctype<T>&) {return false;},
		[&](data_exp_ctype<T>& right) {res.emplace_back(right->safe_execute(cb));},
		[&](data_exp_ctype<T>& parent) {T re = res.back(); res.pop_back(); res.back().merge(parent->get_operator(), re);}
	);

	return res.front();
}

template <typename T, template<typename> class Exp> inline void safe_delete(const std::shared_ptr<Exp<T>>& exp)
{
	travel_exp<std::shared_ptr<Exp<T>>>(exp,
		[](const std::shared_ptr<Exp<T>>& left) {left->safe_delete();},
		[](const std::shared_ptr<Exp<T>>&) {return false;},
		[](const std::shared_ptr<Exp<T>>& right) {right->safe_delete();},
		[](const std::shared_ptr<Exp<T>>& parent) {parent->clear();}
	);
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> class judge_exp;
template <typename T> using judge_exp_type = std::shared_ptr<judge_exp<T>>;
template <typename T> using judge_exp_ctype = const judge_exp_type<T>;
template <typename T> inline bool safe_execute(judge_exp_ctype<T>&, const std::function<T(const std::string&)>&);
template <typename T> class judge_exp : public exp
{
public:
	virtual bool is_judge() const {return true;}
	virtual const std::string& get_operator() const {throw("unsupported get operator operation!");}
	virtual judge_exp_ctype<T>& get_left_item() const {throw("unsupported get left item operation!");}
	virtual judge_exp_ctype<T>& get_right_item() const {throw("unsupported get right item operation!");}

	virtual judge_exp_type<T> revert() const = 0;
	virtual void final_optimize() = 0;

	virtual bool operator()(const std::function<T(const std::string&)>&) const = 0;

private:
	friend bool safe_execute<T>(judge_exp_ctype<T>&, const std::function<T(const std::string&)>&);
	//is recursion fully eliminated or not depends on the data_exp(s) this judge_exp holds
	virtual bool safe_execute(const std::function<T(const std::string&)>&) const = 0;
};

template <typename T> inline bool safe_execute(judge_exp_ctype<T>& jexp, const std::function<T(const std::string&)>& cb)
{
	auto re = false;
	travel_exp<judge_exp_type<T>>(jexp,
		[&](judge_exp_ctype<T>& left) {re = left->safe_execute(cb);},
		//this judgement will impact efficiency, but we have no choice, return false - continue, true - backtrack
		[&](judge_exp_ctype<T>& parent) {return "&&" == parent->get_operator() ? !re : re;},
		[&](judge_exp_ctype<T>& right) {re = right->safe_execute(cb);},
		[](judge_exp_ctype<T>&) {}
	);

	return re;
}

template <typename T> class unitary_judge_exp : public judge_exp<T>
{
protected:
	unitary_judge_exp(data_exp_ctype<T>& _dexp) : dexp(_dexp) {}

public:
	virtual int get_depth() const {return 1 + dexp->get_depth();}
	virtual void show_immediate_value() const {dexp->show_immediate_value();}
	virtual void final_optimize()
	{
		auto data = dexp->final_optimize();
		if (data)
			dexp = data;
	}

private:
	virtual void safe_delete() const {qme::safe_delete(dexp);}

protected:
	data_exp_type<T> dexp;
};

template <typename T> class not_equal_0_judge_exp;
template <typename T> class equal_0_judge_exp : public unitary_judge_exp<T> //intentional optimization for !a ? b : c, for a > 0 .etc, use binary_judge_exp
{
public:
	equal_0_judge_exp(data_exp_ctype<T>& dexp) : unitary_judge_exp<T>(dexp) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<not_equal_0_judge_exp<T>>(this->dexp);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return 0 == (*this->dexp)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const {return 0 == qme::safe_execute(this->dexp, cb);}
};

template <typename T> class not_equal_0_judge_exp : public unitary_judge_exp<T> //intentional optimization for a ? b : c, for a < 0 .etc, use binary_judge_exp
{
public:
	not_equal_0_judge_exp(data_exp_ctype<T>& dexp) : unitary_judge_exp<T>(dexp) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<equal_0_judge_exp<T>>(this->dexp);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return 0 != (*this->dexp)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const {return 0 != qme::safe_execute(this->dexp, cb);}
};

template <typename T> class binary_judge_exp : public judge_exp<T>
{
protected:
	binary_judge_exp(data_exp_ctype<T>& _dexp_l, data_exp_ctype<T>& _dexp_r) : dexp_l(_dexp_l), dexp_r(_dexp_r) {}

public:
	virtual int get_depth() const {return 1 + std::max(dexp_l->get_depth(), dexp_r->get_depth());}
	virtual void show_immediate_value() const {dexp_l->show_immediate_value(); dexp_r->show_immediate_value();}
	virtual void final_optimize()
	{
		auto data = dexp_l->final_optimize();
		if (data)
			dexp_l = data;

		data = dexp_r->final_optimize();
		if (data)
			dexp_r = data;
	}

private:
	virtual void safe_delete() const {qme::safe_delete(dexp_l); qme::safe_delete(dexp_r);}

protected:
	data_exp_type<T> dexp_l, dexp_r;
};

template <typename T> class smaller_equal_judge_exp;
template <typename T> class bigger_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<smaller_equal_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) > (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) > qme::safe_execute(this->dexp_r, cb);}
};

template <typename T> class smaller_judge_exp;
template <typename T> class bigger_equal_judge_exp : public binary_judge_exp<T>
{
public:
	bigger_equal_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<smaller_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) >= (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) >= qme::safe_execute(this->dexp_r, cb);}
};

template <typename T> class smaller_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<bigger_equal_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) < (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) < qme::safe_execute(this->dexp_r, cb);}
};

template <typename T> class smaller_equal_judge_exp : public binary_judge_exp<T>
{
public:
	smaller_equal_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<bigger_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) <= (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) <= qme::safe_execute(this->dexp_r, cb);}
};

template <typename T> class not_equal_judge_exp;
template <typename T> class equal_judge_exp : public binary_judge_exp<T>
{
public:
	equal_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<not_equal_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) == (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) == qme::safe_execute(this->dexp_r, cb);}
};

template <typename T> class not_equal_judge_exp : public binary_judge_exp<T>
{
public:
	not_equal_judge_exp(data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) : binary_judge_exp<T>(dexp_l, dexp_r) {}

	virtual judge_exp_type<T> revert() const {return std::make_shared<equal_judge_exp<T>>(this->dexp_l, this->dexp_r);}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const {return (*this->dexp_l)(cb) != (*this->dexp_r)(cb);}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(this->dexp_l, cb) != qme::safe_execute(this->dexp_r, cb);}
};

template <typename T>
inline judge_exp_type<T> make_binary_judge_exp(data_exp_ctype<T>& dc_1, data_exp_ctype<T>& dc_2, const std::string& c)
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

template <typename T> class logical_exp : public judge_exp<T>
{
protected:
	logical_exp(judge_exp_ctype<T>& _jexp_l, judge_exp_ctype<T>& _jexp_r, const std::string& _lop) :
		lop(_lop), jexp_l(_jexp_l), jexp_r(_jexp_r) {}

public:
	virtual bool is_composite() const {return true;}
	virtual const std::string& get_operator() const {return lop;}
	virtual judge_exp_ctype<T>& get_left_item() const {return jexp_l;}
	virtual judge_exp_ctype<T>& get_right_item() const {return jexp_r;}
	virtual int get_depth() const {return 1 + std::max(jexp_l->get_depth(), jexp_l->get_depth());}
	virtual void show_immediate_value() const {jexp_l->show_immediate_value(); jexp_r->show_immediate_value();}

	virtual void final_optimize() {jexp_l->final_optimize(); jexp_r->final_optimize();}

private:
	virtual bool safe_execute(const std::function<T(const std::string&)>&) const {throw("unsupported safe execute operation!");}
	virtual void clear() {jexp_l.reset(); jexp_r.reset();}

private:
	std::string lop;
	judge_exp_type<T> jexp_l, jexp_r;
};

template <typename T> class or_judge_exp;
template <typename T> class and_judge_exp : public logical_exp<T>
{
public:
	and_judge_exp(judge_exp_ctype<T>& jexp_l, judge_exp_ctype<T>& jexp_r) : logical_exp<T>(jexp_l, jexp_r, "&&") {}

	virtual judge_exp_type<T> revert() const
		{return std::make_shared<or_judge_exp<T>>(this->get_left_item()->revert(), this->get_right_item()->revert());}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const
		{return (*this->get_left_item())(cb) && (*this->get_right_item())(cb);}
};

template <typename T> class or_judge_exp : public logical_exp<T>
{
public:
	or_judge_exp(judge_exp_ctype<T>& jexp_l, judge_exp_ctype<T>& jexp_r) : logical_exp<T>(jexp_l, jexp_r, "||") {}

	virtual judge_exp_type<T> revert() const
		{return std::make_shared<and_judge_exp<T>>(this->get_left_item()->revert(), this->get_right_item()->revert());}

	virtual bool operator()(const std::function<T(const std::string&)>& cb) const
		{return (*this->get_left_item())(cb) || (*this->get_right_item())(cb);}
};

template <typename T>
inline judge_exp_type<T> merge_judge_exp(judge_exp_ctype<T>& jexp_l, judge_exp_ctype<T>& jexp_r, const std::string& lop)
{
	if ("&&" == lop)
		return std::make_shared<and_judge_exp<T>>(jexp_l, jexp_r);
	else if ("||" == lop)
		return std::make_shared<or_judge_exp<T>>(jexp_l, jexp_r);
	else
		throw("undefined logical operator " + lop);
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
template <typename T> class negative_question_exp;
template <typename T> class question_exp : public data_exp<T>
{
	friend class negative_question_exp<T>;

public:
	question_exp(judge_exp_ctype<T>& _jexp) : jexp(_jexp)
	{
		static const auto _0 = std::make_shared<immediate_data_exp<T>>(0);
		static const auto _1 = std::make_shared<immediate_data_exp<T>>(1);

		dexp_l = _1;
		dexp_r = _0;
	}

	question_exp(judge_exp_ctype<T>& _jexp, data_exp_ctype<T>& _dexp_l, data_exp_ctype<T>& _dexp_r) :
		jexp(_jexp), dexp_l(_dexp_l), dexp_r(_dexp_r) {}

	virtual int get_depth() const {return 1 + std::max(jexp->get_depth(), std::max(dexp_l->get_depth(), dexp_r->get_depth()));}
	virtual void show_immediate_value() const
		{jexp->show_immediate_value(); dexp_l->show_immediate_value(); dexp_r->show_immediate_value();}

	virtual data_exp_type<T> final_optimize() const
	{
		jexp->final_optimize();
		auto l = dexp_l->final_optimize();
		auto r = dexp_r->final_optimize();

		return std::make_shared<question_exp<T>>(jexp, l ? l : dexp_l, r ? r : dexp_r);
	}
	virtual data_exp_type<T> to_negative() const {return std::make_shared<negative_question_exp<T>>(jexp, dexp_l, dexp_r);}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const {return (*jexp)(cb) ? (*dexp_l)(cb) : (*dexp_r)(cb);}

protected:
	data_exp_type<T> clone() const {return std::make_shared<question_exp<T>>(jexp, dexp_l, dexp_r);}

private:
	//for question_exp, recursion still happens here if this question_exp is a sub expression
	virtual T safe_execute(const std::function<T(const std::string&)>& cb) const
		{return qme::safe_execute(jexp, cb) ? qme::safe_execute(dexp_l, cb) : qme::safe_execute(dexp_r, cb);}
	//for question_exp, recursion still happens here if this question_exp is a sub expression
	virtual void safe_delete() const {qme::safe_delete(jexp); qme::safe_delete(dexp_l); qme::safe_delete(dexp_r);}

private:
	judge_exp_type<T> jexp;
	data_exp_type<T> dexp_l, dexp_r;
};

template <typename T> class negative_question_exp : public question_exp<T>
{
public:
	negative_question_exp(judge_exp_ctype<T>& jexp, data_exp_ctype<T>& dexp_l, data_exp_ctype<T>& dexp_r) :
		question_exp<T>(jexp, dexp_l, dexp_r) {}

	virtual bool is_easy_to_negative() const {return true;}
	virtual bool is_negative() const {return true;}

	virtual data_exp_type<T> final_optimize() const {return question_exp<T>::final_optimize()->to_negative();}
	virtual data_exp_type<T> to_negative() const {return question_exp<T>::clone();}

	virtual T operator()(const std::function<T(const std::string&)>& cb) const {return -question_exp<T>::operator()(cb);}

private:
	virtual T safe_execute(const std::function<T(const std::string&)>& cb) const {return -question_exp<T>::safe_execute(cb);}
};
/////////////////////////////////////////////////////////////////////////////////////////

using exp_type = std::shared_ptr<exp>;
template <typename T = float, typename O = O3> class compiler
{
private:
	struct sub_exp
	{
		std::string name, raw_exp;
		std::vector<std::string> items;
		exp_type parsed_exp;
	};

public:
	//data exp to judge exp, or judge exp to data exp
	static exp_type transform_exp(const exp_type& exp)
	{
		if (exp->is_data())
		{
			auto data = std::dynamic_pointer_cast<data_exp<T>>(exp);
			return !data ? judge_exp_type<T>() : std::make_shared<not_equal_0_judge_exp<T>>(data);
		}

		auto judge = std::dynamic_pointer_cast<judge_exp<T>>(exp);
		return !judge ? data_exp_type<T>() : std::make_shared<question_exp<T>>(judge);
	}

	template <template<typename> class Exp = data_exp> static std::shared_ptr<Exp<T>> compile(const char* statement)
		{return compile<Exp>(std::string(statement));}
	template <template<typename> class Exp = data_exp> static std::shared_ptr<Exp<T>> compile(const std::string& statement)
	{
		auto exp = compile(statement);
		if (!exp)
			return std::shared_ptr<Exp<T>>();

		auto re = std::dynamic_pointer_cast<Exp<T>>(exp);
		if (!re)
			re = std::dynamic_pointer_cast<Exp<T>>(transform_exp(exp));
		if (!re)
			puts("\033[31mincomplete expression!\033[0m");

		return re;
	}

	static exp_type compile(const char* statement) {return compile(std::string(statement));}
	static exp_type compile(const std::string& statement)
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
				if (re->is_data())
				{
					auto final_re = std::dynamic_pointer_cast<data_exp<T>>(re)->final_optimize();
					if (final_re)
						re = final_re;
				}
				else
					std::dynamic_pointer_cast<judge_exp<T>>(re)->final_optimize();
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

		return exp_type();
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

	static void finish_data_exp(data_exp_type<T>& data_1, data_exp_type<T>&& data_2, std::string& op_1, const std::string& op_2 = std::string())
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

	static void merge_data_exp(data_exp_type<T>& data_1, data_exp_type<T>& data_2, std::string& op_1, std::string& op_2,
		data_exp_type<T>&& data)
	{
		assert(data);
		if (data_2)
			finish_data_exp(data_2, std::forward<data_exp_type<T>>(data), op_2);
		else if (data_1)
		{
			assert(op_2.empty());
			if (is_operator_1(op_1))
				data_2 = data;
			else
				finish_data_exp(data_1, std::forward<data_exp_type<T>>(data), op_1);
		}
		else
		{
			assert(op_1.empty() && op_2.empty());
			data_1 = data;
		}
	}

	static void finish_judge_exp(judge_exp_type<T>& judge_1, judge_exp_type<T>&& judge_2, std::string& lop_1, const std::string& lop_2 = std::string())
	{
		if (!lop_2.empty() || !judge_1)
			throw("missing logical operand!");
		else if (judge_2)
		{
			if (lop_1.empty())
				throw("missing logical operator!");

			judge_1 = qme::merge_judge_exp(judge_1, judge_2, lop_1);
			lop_1.clear();
			judge_2.reset();
		}
		else if (!lop_1.empty())
			throw("missing logical operand!");
	}

	static void merge_judge_exp(judge_exp_type<T>& judge_1, judge_exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2,
		judge_exp_type<T>&& judge)
	{
		assert(judge);
		if (judge_2)
			finish_judge_exp(judge_2, std::forward<judge_exp_type<T>>(judge), lop_2);
		else if (judge_1)
		{
			assert(lop_2.empty());
			if ("||" == lop_1)
				judge_2 = judge;
			else
				finish_judge_exp(judge_1, std::forward<judge_exp_type<T>>(judge), lop_1);
		}
		else
		{
			assert(lop_1.empty() && lop_2.empty());
			judge_1 = judge;
		}
	}

	static void finish_data_and_merge_judge_exp(data_exp_type<T>& dc, std::string& c,
		data_exp_type<T>& data_1, data_exp_type<T>& data_2, std::string& op_1, std::string& op_2,
		judge_exp_type<T>& judge_1, judge_exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2)
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
				assert(!judge_2 && lop_1.empty() && lop_2.empty());
				judge_1 = make_binary_judge_exp(dc, std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_1)), c);
			}
			dc.reset();
			c.clear();
		}
		else if (data_1)
			merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::dynamic_pointer_cast<judge_exp<T>>(transform_exp(data_1)));

		data_1.reset();
	}

	static void finish_data_and_judge_exp(data_exp_type<T>& dc, std::string& c, bool to_data,
		data_exp_type<T>& data_1, data_exp_type<T>& data_2, std::string& op_1, std::string& op_2,
		judge_exp_type<T>& judge_1, judge_exp_type<T>& judge_2, std::string& lop_1, std::string& lop_2)
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
				assert(!judge_2 && lop_1.empty() && lop_2.empty());
				judge_1 = make_binary_judge_exp(dc, std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_1)), c);
			}
			dc.reset();
			c.clear();
		}
		else if (data_1)
		{
			if (!judge_1)
				return;

			merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::dynamic_pointer_cast<judge_exp<T>>(transform_exp(data_1)));
		}

		data_1.reset();
		finish_judge_exp(judge_1, std::move(judge_2), lop_1, lop_2);
		if (to_data)
		{
			data_1 = std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_1));
			judge_1.reset();
		}
	}

	static exp_type compile(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps)
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

	static exp_type compile(const std::vector<std::string>& items, const std::map<std::string, sub_exp>& sub_exps,
		size_t& index, size_t end_index)
	{
		if (index >= end_index)
			throw("empty expresson!");

		std::vector<char> unary_operators; //!-+

		data_exp_type<T> data_1, data_2;
		std::string op_1, op_2;

		data_exp_type<T> dc; //merged comparand (from data_1 and data_2 with op_1)
		std::string c; //comparer

		judge_exp_type<T> judge_1, judge_2;
		std::string lop_1, lop_2;

		judge_exp_type<T> fj; //for final question exp
		data_exp_type<T> fd_1, fd_2; //for final question exp

		while (index < end_index)
		{
			const auto& item = items[index];
			if ("!" == item)
			{
				unary_operators.push_back('!');
				++index;
				continue;
			}
			else if (is_operator(item))
			{
				data_exp_type<T> data;
				if (judge_2)
				{
					if (lop_2.empty())
					{
						data = std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_2));
						judge_2.reset();
					}
				}
				else if (judge_1 && lop_1.empty())
				{
					data = std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_1));
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

					unary_operators.push_back(item[0]);
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

						data_1 = std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_2));
						judge_2.reset();
					}
					else if (judge_1)
					{
						assert(lop_2.empty());
						if (!lop_1.empty())
							throw("missing operand!");

						data_1 = std::dynamic_pointer_cast<data_exp<T>>(transform_exp(judge_1));
						judge_1.reset();
					}
				}

				c = item;
				finish_data_exp(data_1, std::move(data_2), op_1, op_2);
				dc = std::move(data_1);
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
				exp_type parsed_exp;
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

				auto last_operator = '\0';
				for (auto iter = unary_operators.crbegin(); iter != unary_operators.crend(); ++iter)
				{
					if ('!' == *iter)
					{
						if (parsed_exp->is_data())
							parsed_exp = transform_exp(parsed_exp);
						parsed_exp = std::dynamic_pointer_cast<judge_exp<T>>(parsed_exp)->revert();
					}
					else
					{
						if (*iter == last_operator)
							throw ("redundant " + std::string(1, last_operator) + " operator!");
						else if (parsed_exp->is_judge())
							parsed_exp = transform_exp(parsed_exp);

						if ('-' == *iter)
							parsed_exp = std::dynamic_pointer_cast<data_exp<T>>(parsed_exp)->to_negative();
					}
					last_operator = *iter;
				}
				unary_operators.clear();

				if (!data_1 && parsed_exp->is_judge())
					merge_judge_exp(judge_1, judge_2, lop_1, lop_2, std::dynamic_pointer_cast<judge_exp<T>>(parsed_exp));
				else
					merge_data_exp(data_1, data_2, op_1, op_2,
						std::dynamic_pointer_cast<data_exp<T>>(parsed_exp->is_data() ? parsed_exp : transform_exp(parsed_exp)));
			}

			if (!unary_operators.empty())
				throw("unexpected " + std::string(1, unary_operators[0]) + " operator!");

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
