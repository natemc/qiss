        ; vec?atom
        j       10
        J       5 10 15 20
        ?
        j       1
        =

        ; vec?matrix
        J       3 5 -5
        ,:
        dup
        ,
        j       10
        !:
        ?
        J       3 5 10
        ,:
        dup
        ,
        ~
        &

        ; dict?vec
        `       c
        `       b
        `       a
        ,
        ,
        dup
        `
        ,
        |:
        swap
        J       1 2 3
        swap
        !
        J       3 2 1 0
        swap
        ?
        ~
        &

        ; matrix?vec
        J       7 8 9
        dup
        ,:
        J       4 5 6
        ,:
        J       1 2 3
        ,:
        ,
        ,
        ?
        j       2
        =
        &
        
        ; mixed vec?atom
        `       foo
        dup
        j       1
        ,
        ?
        j       1
        =
        &

        ; mixed vec?vec
        `       bar
        `       foo
        ,
        `       foo
        j       1
        ,
        ?
        j       2
        =
        &

        ; matrix?matrix
        J       1 2 3
        ,:
        J       7 8 9
        ,:
        ,
        J       7 8 9
        ,:
        J       4 5 6
        ,:
        J       1 2 3
        ,:
        ,
        ,
        ?
        J       2 0
        ~
        &

        h
