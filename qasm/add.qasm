; This is a comment.  I chose ; because it's not an operator in k,
; so it's an unlikely candidate for an opcode.
; Blank lines are ignored.

; Labels must start in the first column.  An instruction may have multiple labels.
start                     ; A line may have just a label,
L0      j       0         ; a label and an instruction,
        j       1         ; or just an instruction
        +                 ; (in which case the line must start with whitespace).
        j       1
        =

        ; vec+atom
        j       1
        j       3
        !:
        +
        J       1 2 3
        ~
        &

        ; vec+vec
        J       1 2 3
        J       10 20 30
        +
        J       11 22 33
        ~
        &
        
        j       3
        B       1001001b
        *
        &:
        j       3
        =
        call    neg       ; We restrict instruction addresses to 4 bytes.
        J       0 0 0 -1 -1 -1 0 0 0
        ~
        &

        ; Add atom to matrix
        j       25
        !:
        J       5 5
        #
        j       5
        +
        J       25 26 27 28 29
        ,:
        J       20 21 22 23 24
        ,:
        J       15 16 17 18 19
        ,:
        J       10 11 12 13 14
        ,:
        J       5 6 7 8 9
        ,:
        ,
        ,
        ,
        ,
        ~
        &
        
        ; Add vec to matrix
        j       25
        !:
        J       5 5
        #
        j       5
        !:
        +
        J       24 25 26 27 28
        ,:
        J       18 19 20 21 22
        ,:
        J       12 13 14 15 16
        ,:
        J       6 7 8 9 10
        ,:
        J       0 1 2 3 4
        ,:
        ,
        ,
        ,
        ,
        ~
        &
        
        ; Add two matrices
        j       5
        !:
        j       1
        +
        ,:
        dup
        j       10
        *
        ,
        dup
        +
        J       2 4 6 8 10
        ,:
        J       20 40 60 80 100
        ,:
        ,
        ~
        &

        h
neg     -:
        return

; If an opcode takes multiple arguments
;   (e.g., someday the call instruction may take two arguments:
;      the target address, and
;      the number of slots to bump the stack pointer),
; then those arguments will be separated by a comma:
;       call    label, 3
; This will not interfere with the use of , as an opcode.
