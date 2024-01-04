
cflag = -Wall -fexceptions
ifeq (${MAKECMDGOALS}, debug)
	cflag += -g -DDEBUG
else
	cflag += -O3 -DNDEBUG -s
endif

release debug :
	${CXX} ${cflag} -o test_question_exp test_question_exp.cpp

.PHONY : clean
clean:
	-rm -rf test_question_exp

