GREEN=\033[0;32m
NC=\033[0m #No color

clean:
	cd src; make clean
	@echo "${GREEN}Cleanup Complete!${NC}"

.PHONY: parser
parser:
	cd src/; make parser.cpp
	@echo "${GREEN}Parser Generated!${NC}"

.PHONY: compile
compile:
	cd src/; make parser
	@echo "${GREEN}Compilation Successful!${NC}"


.PHONY: test
test: compile
	cat example.program | ./compiler
