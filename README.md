# Dayton Deatherage

# fp_overflow_checker

This program checks for floating-point overflow in loops using two user-supplied floating-point arguments.

## Compilation

Compile the code using:
g++ -o fp_overflow_checker fp_overflow_checker.cpp


## Usage

Run the program with:
./fp_overflow_checker loop_bound loop_counter

- `loop_bound`: Positive floating-point number for the loop's upper limit.
- `loop_counter`: Positive floating-point number for the loop increment.

**Example:**

./fp_overflow_checker 1000.0 0.1