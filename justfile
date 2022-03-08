alias b := build
alias c := clean
alias r := run

flags := "-Wall -Wpedantic -Wextra -Werror -fsanitize=undefined -g"
out := "eff"

# build the program
build:
    clang {{flags}} eff.c -o {{out}}.out

# remove build artifacts
clean:
    rm *.out

# build and run the program
run: build
    ./{{out}}.out
