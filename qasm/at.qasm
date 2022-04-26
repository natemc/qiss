        ; vec@point
        j       2
        J       0 3 6 9
        @
        j       6
        =

        ; vec@matrix
        j       5
        !:
        j       2
        *
        ,:
        dup
        ,
        j       10
        !:
        j       1
        +
        j       5
        *
        @
        J       5 15 25 35 45
        ,:
        dup
        ,        
        ~
        &

        ; dict@point
        `       foo
        J       1 2 3
        `       foo
        `       bar
        `       baz
        ,
        ,
        !
        @
        j       3
        =
        &
        
        ; vec@dict
        J       0 1 2
        j       10
        #
        J       0 3 6
        _
        `       c
        `       b
        `       a
        ,
        ,
        !
        "       xyz
        @
        "       xyzx
        ,:
        "       xyz
        ,:
        dup
        ,
        ,
        `       c
        `       b
        `       a
        ,
        ,
        !
        ~
        &

        ; out of bounds => null
        J       0 3 6
        j       5
        !:
        j       1
        +
        @
        J       1 4 0N
        ~
        &
        
        J       0 3 6
        ,:
        dup
        ,
        j       5
        !:
        j       1
        +
        @
        J       1 4 0N
        ,:
        dup
        ,
        ~
        &

        h
