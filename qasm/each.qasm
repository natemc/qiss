        J       1 2 3
        dup
        J       3 3
        #
        swap
        op      #
        '
        invoke  2
        op      #
        '
        invoke  1
        J       1 2 3
        ~

        J       1 2 3
        j       7
        swap
        op      #
        \:
        invoke  2
        j       5
        op      #
        /:
        invoke  2
        j       7
        J       3 5
        #
        ~
        &

        j       1
        i       2
        B       101b
        ,:
        ,
        ,
        op      @
        '
        invoke  1
        I       1 -7 -8
        ~
        &

        h
