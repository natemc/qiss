        ; long+/floats
        J       1 2 4 8
        j       1
        %
        j       0
        op      +
        /
        invoke  2
        f       1.875
        =

        ; atom+/matrix
        j       6
        !:
        J       2 3
        #
        j       1
        op      +
        /
        invoke  2
        J       4 6 8
        ~
        &

        ; vec+/vec
        J       4 6 8
        J       0 100 200
        op      +
        /
        invoke  2
        J       18 118 218
        ~
        &

        ; vec|/matrix
        j       9
        !:
        J       3 3
        #
        J       5 3 1
        op      |
        /
        invoke  2
        J       6 7 8
        ~
        &

        ; ,/
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
        /
        invoke  2
        j       10
        !:
        ~
        &

        h
