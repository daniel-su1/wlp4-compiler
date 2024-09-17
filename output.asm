.import print
.import init
.import new
.import delete
; begin prologue
lis $4
.word 4
lis $11
.word 1 ; set $11 to 1 (NULL)
sw $1, -4($30) ; push parameter variable a
sub $30, $30, $4 ; update stack pointer
sw $2, -4($30) ; push parameter variable b
sub $30, $30, $4 ; update stack pointer
sub $29, $30, $4 ;  set frame pointer ( after pushing parameters , before pushing non-parameters 
lis $3
.word 1
sw $3, -4($30) ; push NULL VALUE 1 to variable Big
sub $30, $30, $4 ; update stack pointer
lis $3
.word 241
sw $3, -4($30) ; push variable num
sub $30, $30, $4 ; update stack pointer
lis $3
.word 16777216
sw $3, -4($30) ; push variable small
sub $30, $30, $4 ; update stack pointer
lis $3
.word 536870912
sw $3, -4($30) ; push variable big
sub $30, $30, $4 ; update stack pointer
add $2, $2, $0 ; use second parameter for init
; Save $29 and $31
sw $29, -4($30)
sub $30, $30, $4
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word init
jalr $5
; Restore $29 and $31
lw $31, 0($30)
add $30, $30, $4
lw $29, 0($30)
add $30, $30, $4
; end prologue
lw $3, 8($29) ; load a
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -12($29) ; load big
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
mult $3, $4
mflo $3
add $3, $5, $3
sw $3, 0($29) ; store to Big
lw $3, 0($29) ; load Big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, 0($29) ; load Big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -8($29) ; load small
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
mult $3, $4
mflo $3
sub $3, $5, $3
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
sltu $6, $5, $3
bne $6, $0, label0
lw $3, -4($29) ; load num
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lis $3
.word 240000
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
add $3, $5, $3
sw $3, -4($29) ; store to num
beq $0, $0, label1
label0:
label1:
lw $3, -12($29) ; load big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -12($29) ; load big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -8($29) ; load small
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
sub $3, $5, $3
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
slt $6, $5, $3
bne $6, $0, label2
lw $3, -4($29) ; load num
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lis $3
.word 1000
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
add $3, $5, $3
sw $3, -4($29) ; store to num
beq $0, $0, label3
label2:
label3:
lw $3, 0($29) ; load Big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, 0($29) ; load Big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -8($29) ; load small
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
mult $3, $4
mflo $3
add $3, $5, $3
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
sltu $6, $3, $5
bne $6, $0, label4
lw $3, -4($29) ; load num
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lis $3
.word 240000000
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
add $3, $5, $3
sw $3, -4($29) ; store to num
beq $0, $0, label5
label4:
label5:
lw $3, -12($29) ; load big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -12($29) ; load big
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lw $3, -8($29) ; load small
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
add $3, $5, $3
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
slt $6, $3, $5
bne $6, $0, label6
lw $3, -4($29) ; load num
sw $3, -4($30) ; push left operand
sub $30, $30, $4
lis $3
.word 1000000
lw $5, 0($30) ; pop left operand into $5
add $30, $30, $4
add $3, $5, $3
sw $3, -4($29) ; store to num
beq $0, $0, label7
label6:
label7:
lw $3, -4($29) ; load num
; begin epilogue
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
jr $31

