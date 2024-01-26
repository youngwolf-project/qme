
cflag = -Wall -fexceptions -std=c++0x
ifeq (${MAKECMDGOALS}, debug)
	cflag += -g -DDEBUG
else
	cflag += -O3 -DNDEBUG -s
endif

target = test_question_exp
input = ${target}.cpp
dep = question_exp.h
release debug : ${target}
${target} : ${input} ${dep}
	${CXX} ${cflag} -o $@ $<

.PHONY : clean
clean:
	-rm -rf ${target}

