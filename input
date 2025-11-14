ADDI $t0, $zero, 5
NOP
NOP
NOP
ADDI $t1, $zero, 10
NOP
NOP
NOP
ADD $t2, $t0, $t1
NOP
NOP
NOP
SUB $t3, $t2, $t0
NOP
NOP
NOP
MUL $t4, $t1, $t3
NOP
NOP
NOP
AND $t5, $t4, $t2
NOP
NOP
NOP
OR $t6, $t5, $t0
NOP
NOP
NOP
SLL $t7, $t6, 2
NOP
NOP
NOP
SRL $s0, $t7, 1
NOP
NOP
NOP
SW $t2, 0($zero)
NOP
NOP
NOP
LW $s1, 0($zero)
NOP
NOP
NOP
BEQ $t2, $s1, branch
NOP
NOP
NOP
NOP
NOP
NOP                    # Branch decision padding
branch:
ADDI $s2, $zero, 42
NOP
NOP
NOP
J done
NOP
NOP
NOP
NOP
NOP
NOP                    # Jump target padding
done:
ADDI $s3, $zero, 99
NOP
NOP
NOP
NOP                    # End program

