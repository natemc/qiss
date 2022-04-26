        ; key of an atom is iota
        j       0
        !:
        j       0
        dup
        #
        ~

        b       1b
        !:
        J       0
        ~
        &

        j       5
        !:
        J       0 1 2 3 4
        ~
        &

        ; key of a list is iota of count
        J       0 1 2 3 0 1 2 3
        !:
        J       0 1 2 3 4 5 6 7
        ~
        &

        ; key of a dict
        "       abcdefgh
        dup
        j       4
        !:
        j       8
        #
        swap
        !
        !:
        ~
        &

        h
