NAME := echoServer
CXX := c++
#CXXFLAGS := -Wall -Wextra -Werror -std=c++98

SRC := $(wildcard *.cpp)
OBJ := $(SRC:%.cpp=%.o)

$(NAME): $(OBJ)
	$(CXX) -o $@ $^

.PHONY: re clean

re: clean $(NAME)

clean:
	rm -f $(OBJ) $(NAME)