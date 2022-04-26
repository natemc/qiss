        ; atom,atom of same type
        j       1
        j       0
        ,
        J       0 1
        ~

        ; vec,atom of same type
        j       1
        J       10 20 30
        ,
        J       10 20 30 1
        ~
        &

        ; vec,atom of differing types
        `       foo
        J       1 2 3
        ,
        dup
        j       0
        swap
        @
        j       1
        =
        swap
        j       3
        swap
        @
        `       foo
        swap
        =
        &
        &
        
        ; atom,vec of same type
        J       10 15 20
        j       5
        ,
        J       5 10 15 20
        ~
        &

        ; atom,vec of differing types
        J       1 2 3
        `       foo
        ,
        dup
        j       0
        swap
        @
        `       foo
        =
        swap
        j       3
        swap
        @
        j       3
        swap
        =
        &
        &
        
        ; vec,vec of differing types
        J       10 15 20
        B       101b
        ,
        dup
        j       1
        swap
        @
        b       0b
        =
        swap
        j       4
        swap
        @
        j       15
        =
        &
        &

        ; matrix,matrix
        J       25 30 35
        ,:
        J       10 15 20
        ,:
        ,
        j       5
        j       2
        j       6
        !:
        +
        *
        J       2 3
        #
        ~
        &

        h
