        j       5
        !:
        j       3
        -
        ,:
        dup
        -
        J       0 0 0 0 0
        ,:
        ~

        f       3.14
        j       42
        -
        f       38.86
        =
        &

        ; unary - on list
        J       0 1 2 3 4
        -:
        J       0 -1 -2 -3 -4
        ~
        &

        ; unary - on matrix
        J       0 -1 -2 -3 -4
        ,:
        dup
        ,
        -:
        J       0 1 2 3 4
        J       2 5
        #
        ~
        &

        ; unary - on mixed list
        f       3.14
        j       42
        ,
        -:
        f       -3.14
        j       -42
        ,
        ~
        &

        h
