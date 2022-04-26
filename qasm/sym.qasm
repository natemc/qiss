        `       foobar     ; sym length could be restricted to 64K
        `       foo
        `       baz
        `       bar
        `       ack
        ,
        ,
        ,
        ,
        dup
        j       20
        ?
        =:
        !:
        dup
        <:
        swap
        @
        ~
        h
