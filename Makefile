EXECUTABLE=financnidlo
SRC_DIR=src/

build:
	g++ -std=c++17 -o $(EXECUTABLE) -iquote src/ -Wall -O3 src/main.cpp
clean:
	rm -f $(EXECUTABLE)

buildDebug: src/main.cpp
	g++ -std=c++17 -g -o $(EXECUTABLE) -Wall -Wextra -pedantic -O0 -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer src/main.cpp

buildTest:
	g++ -std=c++17 -g -o $(EXECUTABLE) -iquote src/ -Wall -lgtest -lgtest_main tests/main.cpp

test: buildTest
	./$(EXECUTABLE)

run: build
	./$(EXECUTABLE)

runDebug: buildDebug
	./$(EXECUTABLE)
