        B       101b
        ,:
        J       1 2 3
        ,:
        ,
        `       b
        `       j
        ,
        !
        +:
        dup
        call    cols
        `       b
        `       j
        ,
        ~
        swap
        `       j
        swap
        @
        J       1 2 3
        ~
        &
        h

cols    +:
        !:
        return
