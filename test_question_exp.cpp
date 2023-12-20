
#include "question_exp.h"

#ifdef __linux__
#include <sys/time.h>
void print_time_spend(const struct timeval& begin)
{
	struct timeval end;
	gettimeofday(&end, nullptr);

	auto seconds = end.tv_sec - begin.tv_sec;
	auto microseconds = end.tv_usec - begin.tv_usec;
	if (microseconds < 0)
	{
		--seconds;
		microseconds += 1000000000;
	}
	printf("spent %ld seconds %ld microseconds:\n", seconds, microseconds);
}
#endif

template <typename T = float> struct ut_input_and_expectation
{
	const char* input;
	T exp_1, exp_2;
};

int main(int argc, const char* argv[])
{
	//qme::question_exp_parser<int>::parse("(a <= 0 && b <= 0 && c <= 0 && d <= 0 && e <= 0 && f <= 0) ? 0 : (a > 0) ? a : (b > 0) ? b : (c > 0) ? c : (d > 0) ? d : (e > 0) ? e : (f > 0) ? f : -1");
	const ut_input_and_expectation<> inputs[] = {
		//test merging of immediate values at compilation time
		{"a ? a + 1 + 2 + 3 : 0", -94.f, 106.f}, //immediate values: 6, 0
		{"a ? a + 1 / 2 + 3 : 0", -96.5f, 103.5f}, //immediate values: 3.5, 0
		{"a ? a / 1 / 2 + 3 : 0", -47.f, 53.f}, //immediate values: 2, 3, 0
		{"a ? a / 1 + 2 + 3 : 0", -95.f, 105.f}, //immediate values: 5, 0
		{"a ? (a + 1) + 2 + 3 : 0", -94.f, 106.f}, //immediate values: 6, 0
		{"a ? (a + 1) + (2 + 3) : 0", -94.f, 106.f}, //immediate values: 6, 0
		{"a ? ((a + 1) + (2 + 3)) : 0", -94.f, 106.f}, //immediate values: 6, 0
		{"a ? a + 1 + b + 1 : 0", -97.f, 101.f}, //immediate values: 2, 0
		{"a ? a - 1 - b - 1 : 0", -103.f, 99.f}, //immediate values: 2, 0
		{"a ? a - 1 + b - 1 : 0", -101.f, 97.f}, //immediate values: 2, 0
		{"a ? (a + 1) + b + 1 : 0", -97.f, 101.f}, //immediate values: 2, 0
		{"a ? a + 1 + (b + 1) : 0", -97.f, 101.f}, //immediate values: 2, 0
		{"a ? a + 1 + -(b + 1) : 0", -101.f, 101.f}, //immediate values: 0
		{"a ? a + 1 - (b + 1) : 0", -101.f, 101.f}, //immediate values: 0
		{"a ? (a + 1) + (b + 1) : 0", -97.f, 101.f}, //immediate values: 2, 0
		{"a ? (a + 1) + -(b + 1) : 0", -101.f, 101.f}, //immediate values: 0
		{"a ? 2 + (b + 2) : 0", 5.f, 3.f}, //immediate values: 4, 0
		{"a ? 3 * (b * 3) : 0", 9.f, -9.f}, //immediate values: 9, 0
		{"a ? 3 * (b / 3) : 0", 1.f, -1.f}, //immediate values: 0
		{"a ? (b - 2) + 3 : 0", 2.f, 0.f}, //immediate values: -1, 0
		{"a ? 3 + (b - 2) : 0", 2.f, 0.f}, //immediate values: 1, 0
		{"a ? 3 + (b - 3) : 0", 1.f, -1.f}, //immediate values: 0
		{"a ? 2 + -(b + 2) : 0", -1.f, 1.f}, //immediate values: 0
		{"a ? 2 - (b + 2) + 10 : 0", 9.f, 11.f}, //immediate values: 10, 0
		{"a ? 2 - (b - 2) : 0", 3.f, 5.f}, //immediate values: 4, 0
		{"a ? 3 / (b * 3) : 0", 1.f, -1.f}, //immediate values: 1, 0
		{"a ? 3 / (b / 3) : 0", 9.f, -9.f}, //immediate values: 9, 0
		{"a ? -(b + 2) + 2 : 0", -1.f, 1.f}, //immediate values: 0
		{"a ? b * 2 * -2 : 0", -4.f, 4.f}, //immediate values: -4, 0
		{"a ? b * 2 / -2 : 0", -1.f, 1.f}, //immediate values: -1, 0
		{"a ? -(a * 10 + b) : 0", 999.f, -999.f}, //immediate values: -10, 0
		{"a ? -(a / 10 - b) : 0", 11.f, -11.f}, //immediate values: 10, 0

		//sub question expressions' immediate values will not be merged into superiors'
		{"a ? 20 + (a > 0 ? a : 10) : 0", 30.f, 120.f}, //immediate values: 20, 10, 0
		{"a ? (a > 0 ? a : 10) + 20 : 0", 30.f, 120.f}, //immediate values: 10, 20, 0
		{"a ? 20 - (a > 0 ? a : 10) : 0", 10.f, -80.f}, //immediate values: 20, 10, 0
		{"a ? (a > 0 ? a : 10) - 20 : 0", -10.f, 80.f}, //immediate values: 10, 20, 0
		{"a ? -20 + -(a > 0 ? a : 10) : 0", -30.f, -120.f}, //immediate values: -20, 10, 0
		{"a ? -(a > 0 ? a : 10) - 20 : 0", -30.f, -120.f}, //immediate values: 10, 20, 0
		{"a ? -(a > 0 ? a : 10) + 20 : 0", 10.f, -80.f}, //immediate values: 20, 10, 0
///*
		//normal test
		{"a > 0 ? (b < 0 ? b : -b) + 1 >= 0 ? c : -c : c > 0 ? -c : c", -11.f, -11.f},
		{"(a + b > 0 && b > 0 || c > 0) ? ((a > 0 && b > 0) ? +a + b + 1 : - c + 1 + 2) : ((a < 0 || b < 0) ? a - b : c)", -8.f, 101.f},
		{"a > 0 ? b > 0 ? b : 10 : c > 0 ? c : 100", 11.f, 10.f},
		{"a > 0 ? b > 0 ? c > 0 ? c : 1 : 2 : 3", 3.f, 2.f},
		{"a > 0 ? a : b > 0 ? b : 10", 1.f, 100.f},
		{"a < 0 ? (b < 0 ? b : c > 0 ? c : 10) : (a + b > 0 ? a - b : c)", 11.f, 101.f},
		{"(a > 0) ? a + 1 : b", 1.f, 101.f},
		{"(-a > 0) ? a + 1 : b", -99.f, -1.f},
		{"!a ? +a : -b", -1.f, 1.f},
		{"!!a ? a : b", -100.f, 100.f},
		{"(!!a) ? a : b", -100.f, 100.f},
		{"+a + b", -99.f, 99.f},
		{"a", -100.f, 100.f},
		{"!(a > 0) ? a : b", -100.f, -1.f},
		{"a && b ? a : -1.11", -100.f, 100.f},
		{"a || b ? a + b : -2.22", -99.f, 99.f},
		{"(a <= 0 && b <= 0 && c <= 0 && d <= 0 && e <= 0 && f <= 0) ? 0 : (a > 0) ? a : (b > 0) ? b : (c > 0) ? c : (d > 0) ? d : (e > 0) ? e : (f > 0) ? f : -1", 1.f, 100.f},
		{"a > 0 ? b > 0 ? b : 100 : c + 1", 12.f, 100.f},
		{"!(a > 0) ? a : b ? c : 0", -100.f, -11.f},
		{"!-a ? 1 : 2", 2.f, 2.f},
		{"!+a ? 1 : 2", 2.f, 2.f},
		{"a ? -(b + 2) + 3 : 0", 0.f, 2.f},
		{"a ? -b + 0 : 0", -1.f, 1.f},
		{"a ? -(-(-(b))) : 0", -1.f, 1.f},
		{"a ? -(-(-(-b))) : 0", 1.f, -1.f},

		//following expressions are supposed to be invalid:
		{"!a", 0.f, 0.f},
		{"?a", 0.f, 0.f},
		{":a", 0.f, 0.f},
		{"a : b", 0.f, 0.f},
		{"(a >) 0 ? a : b", 0.f, 0.f},
		{"-(a > 0) ? a : b", 0.f, 0.f},
		{"+(a > 0) ? a : b", 0.f, 0.f},
		{"(a > 0) ? a : b +", 0.f, 0.f},
		{"(a > 0) ? a + : b", 0.f, 0.f},
		{"a : b ? c : 0", 0.f, 0.f},
		{": b ? c : 0", 0.f, 0.f},
		{"? c : 0", 0.f, 0.f},
		{"!a > 0 ? a : b", 0.f, 0.f},
		{"a! > 0 ? a : b", 0.f, 0.f},
		{"!(a > 0) ? a : b : c", 0.f, 0.f},
		{"!(a > 0) ? a : b : c : 0", 0.f, 0.f},
		{"-!a ? 1 : 2", 0.f, 0.f},
		{"+!a ? 1 : 2", 0.f, 0.f},
//*/
	};

	std::map<std::string, float> data_map;
	auto cb = [&](const std::string& variable_name) {
		auto iter = data_map.find(variable_name);
		if (iter == data_map.end())
			throw("undefined symbol " + variable_name);
#ifdef DEBUG
		std::cout << " get " << variable_name << " returns " << iter->second << std::endl;
#endif
		return iter->second;
	};
	for (size_t i = 0; i < sizeof(inputs) / sizeof(ut_input_and_expectation<>); ++i)
	{
		printf("compile the question mark expression: %s\n", inputs[i].input);
#ifdef __linux__
		struct timeval begin;
		gettimeofday(&begin, nullptr);
		auto exp = qme::question_exp_parser<>::parse(inputs[i].input);
		print_time_spend(begin);
#else
		auto exp = qme::question_exp_parser<>::parse(inputs[i].input);
#endif
		if (exp)
		{
			try
			{
				///*
				data_map["a"] = -100.f;
				data_map["b"] = 1.f;
				data_map["c"] = 11.f;
				//*/
				/*
				data_map["a"] = -1.f;
				data_map["b"] = -1.f;
				data_map["c"] = -1.f;
				data_map["d"] = -1.f;
				data_map["e"] = -1.f;
				data_map["f"] = -1.f;
				*/
				puts("perform the question mark expression:");
#ifdef __linux__
				gettimeofday(&begin, nullptr);
				auto re = (*exp)(cb);
				print_time_spend(begin);
#else
				auto re = (*exp)(cb);
#endif
				if (re == inputs[i].exp_1)
					printf(" %f\n", re);
				else
					printf(" UT failed, expected result: \033[31m%f\033[0m, actual result: \033[32m%f\033[0m\n", inputs[i].exp_1, re);

				data_map["a"] = 100.f;
				data_map["b"] = -1.f;
				data_map["c"] = -11.f;
				/*
				data_map["a"] = -1.f;
				data_map["b"] = -1.f;
				data_map["c"] = -1.f;
				data_map["d"] = -1.f;
				data_map["e"] = -1.f;
				data_map["f"] = -1.f;
				*/
				puts("perform the question mark expression again:");
#ifdef __linux__
				gettimeofday(&begin, nullptr);
				re = (*exp)(cb);
				print_time_spend(begin);
#else
				re = (*exp)(cb);
#endif
				if (re == inputs[i].exp_2)
					printf(" %f\n", re);
				else
					printf(" UT failed, expected result: \033[31m%f\033[0m, actual result: \033[32m%f\033[0m\n", inputs[i].exp_2, re);
			}
			catch (const std::exception& e) {puts(e.what());}
			catch (const std::string& e) {puts(e.data());}
			catch (const char* e) {puts(e);}
			catch (...) {puts("unknown exception happened!");}
		}
		putchar('\n');
	}
	return 0;
}
