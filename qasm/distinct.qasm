        B       1001b
        ?:
        B       10b
        ~

        j       10
        j       50
        ?
        dup
        ?:
        call    sort
        swap
        call    sort
        dup
        dup
        j       0N
        swap
        ,
        j       1
        _
        =
        ~:
        &:
        swap
        @
        ~
        &

        h

sort
        dup
        <:
        swap
        @
        return
