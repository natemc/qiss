        ; dict indexing
        j       4
        call    dict
        dup
        dup
        dup
        dup
        #:
        j       4
        =
        `       a
        rot     2
        @
        j       10
        =
        `       b
        rot     3
        @
        j       20
        =
        `       c
        rot     4
        @
        j       30
        =
        `       d
        rot     5
        @
        j       40
        =
        &
        &
        &
        &

        ; atom * dict, dict + vector, dict - atom
        j       4
        call    dict
        j       5
        *
        dup
        .:
        J       50 100 150 200
        ~
        swap
        J       -25 -50 -75 -100
        +
        dup
        .:
        J       25 50 75 100
        ~
        swap
        j       25
        swap
        -
        .:
        J       0 25 50 75
        ~
        &
        &
        &

        ; dict + dict
        j       4
        call    dict
        J       1000 2000 3000 4000
        `       f
        `       e
        `       b
        `       d
        ,
        ,
        ,
        !
        swap
        +
        j       6
        call    dict
        J       0 2000 0 1000 2950 3940
        +
        ~
        &

        ; dict + dict w/matrix value
        j       9
        !:
        J       3 3
        #
        j       3
        `       b
        call    syms
        !
        j       3
        call    dict
        +
        J       20 21 22 33 34 35 6 7 8
        J       3 3
        #
        j       10
        ,
        j       4
        `       a
        call    syms
        !
        ~
        &

        ; dict,dict
        J       1 2 3
        j       3
        `       b
        call    syms
        !
        j       3
        call    dict
        ,
        J       0 19 28 37
        j       4
        call    dict
        -
        ~
        &

        ; dict,dict w/differing value types
        "       BCD
        j       3
        `       b
        call    syms
        !
        j       3
        call    dict
        ,
        "       BCD
        j       10
        ,
        j       4
        `       a
        call    syms
        !
        ~
        &
        h

dict    ; Make dict of size x, e.g., if x == 4, `a`b`c`d!10 20 30 40
        dup
        !:
        j       1
        +
        j       10
        *
        swap
        `       a
        call    syms
        !
        return

syms    ; Make list of syms of size x starting with y
        $:
        *:
        "       j
        *:
        $
        swap
        !:
        +
        "       c
        *:
        $
        `
        op      $
        /:
        invoke  2
        return
