
#include "question_exp.h"

#include <chrono>
class cpu_timer //a substitute of boost::timer::cpu_timer
{
public:
	cpu_timer() {restart();}

	void restart() {started = false; elapsed_seconds = .0f; start();}
	void start() {if (started) return; started = true; start_time = std::chrono::system_clock::now();}
	void resume() {start();}
	void stop() {if (!started) return; started = false; elapsed_seconds += std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - start_time).count();}

	bool stopped() const {return !started;}
	float elapsed() const {if (!started) return elapsed_seconds; return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - start_time).count();}

protected:
	bool started;
	float elapsed_seconds;
	std::chrono::system_clock::time_point start_time;
};

template <typename T = float> struct ut_input_and_expectation
{
	const char* input;
	T exp_1, exp_2;
};

int main(int argc, const char* argv[])
{
	const ut_input_and_expectation<> inputs[] = {
		//test merging of immediate values at compilation time (O3)
		//for O2, immediate values can be different, please note
		{"a ? a + 1 + 2 + 3 : 0", -94.f, 106.f}, //immediate values: 6, 0
		{"a ? a + 1 / 2 + 3 : 0", -96.5f, 103.5f}, //immediate values: 3.5, 0
		{"a ? a / 1 / 2 + 3 : 0", -47.f, 53.f}, //immediate values: 0.5, 3, 0
		{"a ? a / 1 / 2 + (a + 3) : 0", -147.f, 153.f}, //immediate values: 1.5, 3, 0
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
		{"a ? b * 2 / -2 : 0", -1.f, 1.f}, //immediate values: 0
		{"a ? -(a * 10 + b) : 0", 999.f, -999.f}, //immediate values: -10, 0
		{"a ? -(a / 10 - b) : 0", 11.f, -11.f}, //immediate values: 0.1, 0
		{"a ? 1 / a : 0", -.01f, .01f}, //immediate values: 1, 0

		//sub question expressions' immediate values will not be merged into superiors'
		{"a ? 20 + (a > 0 ? a : 10) : 0", 30.f, 120.f}, //immediate values: 20, 0 (from the judgement of the sub qme), 10, 0
		{"a ? (a > 0 ? a : 10) + 20 : 0", 30.f, 120.f}, //immediate values: 0 (from the judgement of the sub qme), 10, 20, 0
		{"a ? 20 - (a > 0 ? a : 10) : 0", 10.f, -80.f}, //immediate values: 20, 0 (from the judgement of the sub qme), 10, 0
		{"a ? (a > 0 ? a : 10) - 20 : 0", -10.f, 80.f}, //immediate values: 0 (from the judgement of the sub qme), 10, 20, 0
		{"a ? -20 + -(a > 0 ? a : 10) : 0", -30.f, -120.f}, //immediate values: -20, 0 (from the judgement of the sub qme), 10, 0
		{"a ? -(a > 0 ? a : 10) - 20 : 0", -30.f, -120.f}, //immediate values: 0 (from the judgement of the sub qme), 10, 20, 0
		{"a ? -(a > 0 ? a : 10) + 20 : 0", 10.f, -80.f}, //immediate values: 20, 0 (from the judgement of the sub qme), 10, 0

		//test merging of same variables -- use multiplier or exponent (O3)
		//for O2, immediate values can be different, please note
		{"a ? a + a + b + b : 0", -198.f, 198.f}, //convert to 2 * a + 2 * b
		{"a ? a + b + a + b : 0", -198.f, 198.f}, //convert to 2 * a + 2 * b
		{"a ? 2 * a + a + a : 0", -400.f, 400.f}, //convert to 4 * a
		{"a ? a + a + 2 * a : 0", -400.f, 400.f}, //convert to 4 * a
		{"a ? a + a + a * 2 : 0", -400.f, 400.f}, //convert to 4 * a
		{"a + a ? -a - a : 0", 200.f, -200.f}, //convert to 2 * a (judgement part) and -2 * a
		{"a * a * a ? a * b * a * b : 0", 10000.f, 10000.f}, //convert to a^3 (judgement part) and a^2 * b^2
		{"a ? 100 / a / a : 0", .01f, .01f}, //convert to 100 / (a^2)
		{"a ? a / a / a / a : 0", .0001f, .0001f}, //convert to 1 / a^2
		{"a ? a - (a + a) : 0", 100.f, -100.f}, //convert to -a
		{"a ? 2 * a * a * a / (3 * a * a) : 0", -66.666672f, 66.666672f}, //convert to 0.666667 * a
		{"a ? 2 * a * a / (3 * a * a * a) : 0", -0.006666667f, 0.006666667f}, //convert to 0.666667 / a
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
		{"!a"},
		{"?a"},
		{":a"},
		{"a : b"},
		{"(a >) 0 ? a : b"},
		{"-(a > 0) ? a : b"},
		{"+(a > 0) ? a : b"},
		{"(a > 0) ? a : b +"},
		{"(a > 0) ? a + : b"},
		{"a : b ? c : 0"},
		{": b ? c : 0"},
		{"? c : 0"},
		{"!a > 0 ? a : b"},
		{"a! > 0 ? a : b"},
		{"!(a > 0) ? a : b : c"},
		{"!(a > 0) ? a : b : c : 0"},
		{"-!a ? 1 : 2"},
		{"+!a ? 1 : 2"},
		//*/
	};

	auto cb = [](const std::map<std::string, float>& dm, const std::string& variable_name) {
		auto iter = dm.find(variable_name);
		if (iter == std::end(dm))
			throw("undefined symbol " + variable_name);
#ifdef DEBUG
		std::cout << " get " << variable_name << " returns " << iter->second << std::endl;
#endif
		return iter->second;
	};

	std::map<std::string, float> dm_1;
	///*
	dm_1["a"] = -100.f;
	dm_1["b"] = 1.f;
	dm_1["c"] = 11.f;
	//*/
	/*
	dm_1["a"] = -1.f;
	dm_1["b"] = -1.f;
	dm_1["c"] = -1.f;
	dm_1["d"] = -1.f;
	dm_1["e"] = -1.f;
	dm_1["f"] = -1.f;
	*/
	auto cb_1 = [&](const std::string& variable_name) {return cb(dm_1, variable_name);};

	std::map<std::string, float> dm_2;
	///*
	dm_2["a"] = 100.f;
	dm_2["b"] = -1.f;
	dm_2["c"] = -11.f;
	//*/
	/*
	dm_2["a"] = -1.f;
	dm_2["b"] = -1.f;
	dm_2["c"] = -1.f;
	dm_2["d"] = -1.f;
	dm_2["e"] = -1.f;
	dm_2["f"] = -1.f;
	*/
	auto cb_2 = [&](const std::string& variable_name) {return cb(dm_2, variable_name);};

	cpu_timer timer;
	auto compile_succ = 0, exec_succ = 0, match = 0;
	for (size_t i = 0; i < sizeof(inputs) / sizeof(ut_input_and_expectation<>); ++i)
	{
		printf("compile the question mark expression: %s\n", inputs[i].input);
		timer.restart();
		//auto exp = qme::question_exp_parser<int, qme::O2>::parse(inputs[i].input); //for integer, do not use optimization level 3
		//auto exp = qme::question_exp_parser<float, qme::O2>::parse(inputs[i].input); //for float, any optimization level is OK
		auto exp = qme::question_exp_parser<>::parse(inputs[i].input); //for float (default), the default optimization level is 3
		printf("spent %f seconds.\n", timer.elapsed());
		if (exp)
		{
			++compile_succ;
			try
			{
				puts("perform the question mark expression:");
				timer.restart();
				auto re = (*exp)(cb_1);
				printf("spent %f seconds.\n", timer.elapsed());
				++exec_succ;
				if (re == inputs[i].exp_1)
				{
					++match;
					std::cout << ' ' << re << std::endl;
				}
				else
					std::cout << " UT failed, expected result: \033[31m" << inputs[i].exp_1 << "\033[0m, actual result: \033[32m" << re << "\033[0m" << std::endl;

				puts("perform the question mark expression again:");
				timer.restart();
				re = (*exp)(cb_2);
				printf("spent %f seconds.\n", timer.elapsed());
				++exec_succ;
				if (re == inputs[i].exp_2)
				{
					++match;
					std::cout << ' ' << re << std::endl;
				}
				else
					std::cout << " UT failed, expected result: \033[31m" << inputs[i].exp_2 << "\033[0m, actual result: \033[32m" << re << "\033[0m" << std::endl;
			}
			catch (const std::exception& e) {printf("\033[31m%s\033[0m\n", e.what());}
			catch (const std::string& e) {printf("\033[31m%s\033[0m\n", e.data());}
			catch (const char* e) {printf("\033[31m%s\033[0m\n", e);}
			catch (...) {puts("\033[31munknown exception happened!\033[0m");}
		}
		putchar('\n');
	}
	std::cout << "summary:" << std::endl
		<< " total qme: " << sizeof(inputs) / sizeof(ut_input_and_expectation<>) << std::endl
		<< " successfully compiled: " << compile_succ << std::endl
		<< " successfully executed: " << exec_succ << std::endl
		<< " successfully matched: " << match << std::endl;

	return 0;
}
