FROM gcc:13.1
WORKDIR /usr/src/myapp
RUN apt-get install libncurses5-dev libncursesw5-dev
CMD mkdir -p build; cd build; g++ -O3 -std=c++23 -Wall -Wextra -Werror -Wpedantic -o myapp ../main.cpp -lncurses && ./myapp
