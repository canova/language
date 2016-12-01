clean:
	$(RM) -rf parser
	cd src; make clean

.PHONY: compiler
compile:
	cd src/; make parser
	@echo "COMPILATION SUCCESSFUL!"

.PHONY: compiler
parser:
	cd src/; make parser.cpp
	@echo "Parser Generated!"

.PHONY: test
test:
	cat example.txt | ./src/parser