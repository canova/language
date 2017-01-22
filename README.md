# Turkish Programming Language
A Programming Language in Turkish to Encourage More Turkish Speaking People to Programming

This is still under active developement and highly unstable.

### Prerequisites
- Flex and Bison.
- Clang and LLVM 3.8 (You can get the linux packages here: http://apt.llvm.org/ or you can get with Homebrew on macOS) (You also need to create a symlink for llvm-config-3.8 to llvm-config)
- Blocks Runtime Library if you are on Linux (You can get with apt: `sudo apt-get install libblocksruntime-dev`)

### Build
- Enter the project directory with terminal.
- Run `make compile` command. This will give you a binary program with name "compiler". Or if you want to test it directly you can run `make test` command. This will get the file named example.program from your project's root directory and compile it with generated compiler.
