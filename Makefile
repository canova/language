GREEN=\033[0;32m
NC=\033[0m #No color

clean:
	cd src; make clean
	@echo "${GREEN}Clenup Complete!${NC}"

.PHONY: compiler
compile:
	cd src/; make parser
	@echo "${GREEN}Compilation Successful!${NC}"

.PHONY: compiler
parser:
	cd src/; make parser.cpp
	@echo "${GREEN}Parser Generated!${NC}"

.PHONY: test
test: compile
	cat example.txt | ./compiler
