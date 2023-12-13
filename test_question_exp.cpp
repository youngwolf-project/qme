
#include "question_exp.h"

int main(int argc, const char* argv[])
{
	const char* const inputs[] = {
		"(a + b > 0 && b > 0 || c > 0) ? ((a > 0 && b > 0) ? a + b + 1 : - c + 1 + 2) : ((a < 0 || b < 0) ? a - b : c)",
		"a > 0 ? b > 0 ? b : 10 : c > 0 ? c : 100",
		"a > 0 ? b > 0 ? c > 0 ? c : 1 : 2 : 3",
		"a > 0 ? a : b > 0 ? b : 10",
		"a < 0 ? (b < 0 ? b : c > 0 ? c : 10) : (a + b > 0 ? a - b : c)",
		"(a > 0) ? a + 1 : b",
		"(-a > 0) ? a + 1 : b",
		"!a ? a : b",
		"!!a ? a : b",
		"(!!a) ? a : b",
		"a + b",
		"a",
		"!(a > 0) ? a : b",
		"a && b ? a : -1.11",
		"a || b ? a + b : -2.22",

		"!a",
		"?a",
		":a",
		"a : b",
		"(a >) 0 ? a : b",
		"-(a > 0) ? a : b",
		"(a > 0) ? a : b +",
		"(a > 0) ? a + : b",
		"a : b ? c : 0",
		": b ? c : 0",
		"? c : 0",
		"!a > 0 ? a : b",
		"a! > 0 ? a : b",
		"!(a > 0) ? a : b : c",
		"!(a > 0) ? a : b : c : 0",
		"!(a > 0) ? a : b ? c : 0",
		"(a <= 0 && b <= 0 && c <= 0 && d <= 0 && e <= 0 && f <= 0) ? 0 : (a > 0) ? a : (b > 0) ? b : (c > 0) ? c : (d > 0) ? d : (e > 0) ? e : (f > 0) ? f : -1",
		"a > 0 ? b > 0 ? b : 100 : c + 1",
	};
	for (size_t i = 0; i < sizeof(inputs) / sizeof(const char*); ++i)
	{
		auto exp = qme::question_exp_parser<>::parse(inputs[i]);
		if (exp)
		{
			std::map<std::string, float> data_map;
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
			try {printf("%f\n", (*exp)(data_map));}
			catch (const std::exception& e) {puts(e.what());}
			catch (const std::string& e) {puts(e.data());}
			catch (const char* e) {puts(e);}
			catch (...) {puts("unknown exception happened!");}
		}
		putchar('\n');
	}
	return 0;
}
