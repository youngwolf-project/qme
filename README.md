# qme
Question mark expression just as C/C++'s
-
Overview
-
Compile once and execute any times with different values of the variables in the question mark expression.

Quick start
-
Execute ./build.sh, if compilation error occurs, try to add -std=c++11 option.</br>
Then execute ./test_question_exp

Example:
-
```
auto exp = qme::question_exp_parser<>::parse("a > 0 ? b > 0 ? b : 100 : c + 1");
if (exp)
{
	std::map<std::string, float> data_map;
	data_map["a"] = -100.f;
	data_map["b"] = 1.f;
	data_map["c"] = 11.f;
	puts("perform the question mark expression:");
	try {printf("%f\n", (*exp)(data_map));}
	catch (const std::exception& e) {puts(e.what());}
	catch (const std::string& e) {puts(e.data());}
	catch (const char* e) {puts(e);}
	catch (...) {puts("unknown exception happened!");}
}
```
Compiler requirement:
-
Visual C++ 11.0, GCC 4.7 or Clang 3.1 at least, with c++11 features;</br>

email: mail2tao@163.com
-
Community on QQ: 198941541
-
