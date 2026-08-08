#include "lib.h"

int fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

void print_num_labeled(int i, int val) {
    print_s("fib(");
    print_d(i);
    print_s(") = ");
    print_d(val);
    print_s("\n");
}

int main() {
    print_s("=== Fibonacci Sequence on RISC-V ===\n");
    int i;
    for (i = 0; i < 15; i++) {
        print_num_labeled(i, fib(i));
    }
    print_s("====================================\n");
    exit_proc();
}
