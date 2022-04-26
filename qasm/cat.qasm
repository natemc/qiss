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
        dup
        dup
        j       0
        swap
        @
        j       1
        =
        j       1
        rot     2
        @
        j       2
        =
        j       2
        rot     3
        @
        j       3
        =
        j       3
        rot     4
        @
        `       foo
        =
        &
        &
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
        dup
        dup
        j       0
        swap
        @
        `       foo
        =
        j       1
        rot     2
        @
        j       1
        =
        j       2
        rot     3
        @
        j       2
        =
        j       3
        rot     4
        @
        j       3
        =
        &
        &
        &
        &

        ; vec,vec of differing types
        J       10 15 20
        B       101b
        ,
        dup
        dup
        dup
        dup
        dup
        j       0
        swap
        @
        j       1
        rot     2
        @
        ~:
        j       2
        rot     3
        @
        j       3
        rot     4
        @
        j       10
        =
        j       4
        rot     5
        @
        j       15
        =
        j       5
        rot     6
        @
        j       20
        =
        &
        &
        &
        &
        &
        &


        ; matrix,matrix
        J       25 30 35
        ,:
        J       10 15 20
        ,:
        ,
        dup
        j       0
        swap
        @
        J       10 15 20
        ~
        j       1
        rot     2
        @
        J       25 30 35
        ~
        &
        &
        
        ; table,table
        B       101b
        ,:
        J       10 20 30
        ,:
        ,
        `       b
        `       a
        ,
        !
        +:
        B       101b
        ,:
        J       1 2 3
        ,:
        ,
        `       b
        `       a
        ,
        !
        +:
        ,
        dup
        `       a
        swap
        @
        J       1 2 3 10 20 30
        ~
        `       b
        rot     2
        @
        B       101101b
        ~
        &
        &

        h
