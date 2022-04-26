        ; long+\floats
        J       1 2 4 8
        j       1
        %
        j       0
        op      +
        \
        invoke  2
        F       1 1.5 1.75 1.875
        ~

        ; atom+\matrix
        j       6
        !:
        J       2 3
        #
        j       1
        op      +
        \
        invoke  2
        J       4 6 8
        ,:
        J       1 2 3
        ,:
        ,
        ~
        &

        ; vec+\vec
        J       4 6 8
        J       0 100 200
        op      +
        \
        invoke  2
        J       18 118 218
        ,:
        J       10 110 210
        ,:
        J       4 104 204
        ,:
        ,
        ,
        ~
        &

        ; vec|\matrix
        j       9
        !:
        J       3 3
        #
        J       5 3 1
        op      |
        \
        invoke  2
        J       6 7 8
        ,:
        J       5 4 5
        ,:
        J       5 3 2
        ,:
        ,
        ,
        ~
        &

        ; ,\
        J       7 8 9
        ,:
        J       4 5 6
        ,:
        J       1 2 3
        ,:
        ,
        ,
        j       0
        op      ,
        \
        invoke  2
        J       4 7 10
        op      !
        '
        invoke  1
        ~
        &

        "       abcdef
        op      ,
        \
        invoke  1
        "       abcdef
        j       2
        j       5
        !:
        +
        op      #
        \:
        invoke  2
        ~
        &

        h
