# qme
Question mark expression just as C/C++'s
-
Overview
-
Compile once and execute any times with different values of the variables in the question mark expression.

Quick start
-
Execute make or make debug, if compilation error occurs, try to add -std=c++11 option.</br>
Then execute ./test_question_exp

Example:
-
```
auto cb = [](const std::map<std::string, float>& dm, const std::string& variable_name) {
	auto iter = dm.find(variable_name);
	if (iter == std::end(dm))
		throw("undefined symbol " + variable_name);
	return iter->second;
};

std::map<std::string, float> dm_1;
dm_1["a"] = -100.f;
dm_1["b"] = 1.f;
dm_1["c"] = 11.f;
auto cb_1 = [&](const std::string& variable_name) {return cb(dm_1, variable_name);};

auto exp = qme::compiler<>::compile<>("a > 0 ? b > 0 ? b : 100 : c + 1");
if (exp)
{
	puts("perform the question mark expression:");
	try {printf("%f\n", (*exp)(cb_1));}
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
