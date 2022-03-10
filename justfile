alias b := build
alias c := clean
alias r := run

flags := "-Wall -Wpedantic -Wextra -Werror -fsanitize=undefined -g"

# list available options
default:
    just --list

# build the program
build OUT:
    cc {{flags}} {{OUT}}.c -o {{OUT}}.out

# remove build artifacts
clean:
    rm *.out

# build and run the program
run OUT: (build OUT)
    ./{{OUT}}.out
