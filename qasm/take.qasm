        ; atom#atom
        j       4
        dup
        #
        J       4 4 4 4
        ~
        
        ; atom#vec
        J       0 3 6 9
        j       2
        #
        J       0 3
        ~
        &

        ; -atom#vec
        j       10
        !:
        j       -3
        #
        J      7 8 9
        ~
        &

        ; overtake
        j       5
        !:
        j       8
        #
        J       0 1 2 3 4 0 1 2
        ~
        &

        ; pair#atom
        j       9
        J       3 3
        #
        J       9 9 9
        ,:
        dup
        dup
        ,
        ,
        ~
        &

        ; pair#vec
        j       9
        !:
        J       3 3
        #
        J       6 7 8
        ,:
        J       3 4 5
        ,:
        J       0 1 2
        ,:
        ,
        ,
        ~
        &

        h
