
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
	printf("spent %ld seconds %ld microseconds\n", seconds, microseconds);
}
#endif

int main(int argc, const char* argv[])
{
	const char* const inputs[] = {
		//test merging of immediate values at compilation time
		"a ? a + 1 + 2 + 3 : 0", //immediate values: 6, 0
		"a ? a + 1 / 2 + 3 : 0", //immediate values: 3.5, 0
		"a ? a / 1 / 2 + 3 : 0", //immediate values: 2, 3, 0
		"a ? a / 1 + 2 + 3 : 0", //immediate values: 5, 0
		"a ? (a + 1) + 2 + 3 : 0", //immediate values: 6, 0
		"a ? (a + 1) + (2 + 3) : 0", //immediate values: 6, 0
		"a ? ((a + 1) + (2 + 3)) : 0", //immediate values: 6, 0
		"a ? a + 1 + b + 1 : 0", //immediate values: 2, 0
		"a ? a - 1 - b - 1 : 0", //immediate values: 2, 0
		"a ? a - 1 + b - 1 : 0", //immediate values: 2, 0
		"a ? (a + 1) + b + 1 : 0", //immediate values: 2, 0
		"a ? a + 1 + (b + 1) : 0", //immediate values: 2, 0
		"a ? a + 1 + -(b + 1) : 0", //immediate values: 0
		"a ? a + 1 - (b + 1) : 0", //immediate values: 0
		"a ? (a + 1) + (b + 1) : 0", //immediate values: 2, 0
		"a ? (a + 1) + -(b + 1) : 0", //immediate values: 0
		"a ? 2 + (b + 2) : 0", //immediate values: 4, 0
		"a ? 3 * (b * 3) : 0", //immediate values: 9, 0
		"a ? 3 * (b / 3) : 0", //immediate values: 0
		"a ? (b - 2) + 3 : 0", //immediate values: -1, 0
		"a ? 3 + (b - 2) : 0", //immediate values: 1, 0
		"a ? 3 + (b - 3) : 0", //immediate values: 0
		"a ? 2 + -(b + 2) : 0", //immediate values: 0
		"a ? 2 - (b + 2) + 10 : 0", //immediate values: 10, 0
		"a ? 2 - (b - 2) : 0", //immediate values: 4, 0
		"a ? 3 / (b * 3) : 0", //immediate values: 1, 0
		"a ? 3 / (b / 3) : 0", //immediate values: 9, 0
		"a ? -(b + 2) + 2 : 0", //immediate values: 0
		"a ? b * 2 * -2 : 0", //immediate values: -4, 0
		"a ? b * 2 / -2 : 0", //immediate values: -1, 0
///*
		//normal test
		"(a + b > 0 && b > 0 || c > 0) ? ((a > 0 && b > 0) ? +a + b + 1 : - c + 1 + 2) : ((a < 0 || b < 0) ? a - b : c)",
		"a > 0 ? b > 0 ? b : 10 : c > 0 ? c : 100",
		"a > 0 ? b > 0 ? c > 0 ? c : 1 : 2 : 3",
		"a > 0 ? a : b > 0 ? b : 10",
		"a < 0 ? (b < 0 ? b : c > 0 ? c : 10) : (a + b > 0 ? a - b : c)",
		"(a > 0) ? a + 1 : b",
		"(-a > 0) ? a + 1 : b",
		"!a ? +a : -b",
		"!!a ? a : b",
		"(!!a) ? a : b",
		"+a + b",
		"a",
		"!(a > 0) ? a : b",
		"a && b ? a : -1.11",
		"a || b ? a + b : -2.22",
		"(a <= 0 && b <= 0 && c <= 0 && d <= 0 && e <= 0 && f <= 0) ? 0 : (a > 0) ? a : (b > 0) ? b : (c > 0) ? c : (d > 0) ? d : (e > 0) ? e : (f > 0) ? f : -1",
		"a > 0 ? b > 0 ? b : 100 : c + 1",
		"!(a > 0) ? a : b ? c : 0",
		"!-a ? 1 : 2",
		"!+a ? 1 : 2",
		"a ? -(b + 2) + 3 : 0",
		"a ? -b + 0 : 0",
		"a ? -(-(-(b))) : 0",
		"a ? -(-(-(-b))) : 0",

		//following expressions are supposed to be invalid:
		"!a",
		"?a",
		":a",
		"a : b",
		"(a >) 0 ? a : b",
		"-(a > 0) ? a : b",
		"+(a > 0) ? a : b",
		"(a > 0) ? a : b +",
		"(a > 0) ? a + : b",
		"a : b ? c : 0",
		": b ? c : 0",
		"? c : 0",
		"!a > 0 ? a : b",
		"a! > 0 ? a : b",
		"!(a > 0) ? a : b : c",
		"!(a > 0) ? a : b : c : 0",
		"-!a ? 1 : 2",
		"+!a ? 1 : 2",
//*/
	};
	for (size_t i = 0; i < sizeof(inputs) / sizeof(const char*); ++i)
	{
		printf("compile the question mark expression: %s\n", inputs[i]);
#ifdef __linux__
		struct timeval begin;
		gettimeofday(&begin, nullptr);
		auto exp = qme::question_exp_parser<>::parse(inputs[i]);
		print_time_spend(begin);
#else
		auto exp = qme::question_exp_parser<>::parse(inputs[i]);
#endif
		if (exp)
		{
			std::map<std::string, float> data_map;
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
				printf(" %f\n", (*exp)(data_map));
				print_time_spend(begin);
#else
				printf(" %f\n", (*exp)(data_map));
#endif

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
				printf(" %f\n", (*exp)(data_map));
				print_time_spend(begin);
#else
				printf(" %f\n", (*exp)(data_map));
#endif
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
