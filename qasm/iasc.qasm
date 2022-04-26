        ; iasc vec
        J       4 0 2 1 2 1 2 3 2 4 1 0 2 4 1 2 0 1 1 2
        call    sort
        op      <
        ':
        invoke  1
        op      |
        /
        invoke  1
        ~:

        ; idesc vec
        J       4 0 2 1 2 1 2 3 2 4 1 0 2 4 1 2 0 1 1 2
        dup
        >:
        swap
        @
        ?:
        J       4 3 2 1 0
        ~
        &

        ; iasc dict
        J       0 0 3 1 2 0 0 2 2 3
        "       abcdefghij
        !
        call    sort
        J       0 0 0 0 1 2 2 2 3 3
        ~
        &

        ; iasc sym
        "       adtlnfbrryjnvabnrajbkrpywncrksxtgbtiqtzj
        J       20 2
        #
        `
        op      $
        /:
        invoke  2
        call    sort
        "       adbnbrcrgbjbjnkrksnfpyqtrarytitlvawnxtzj
        J       20 2
        #
        `
        op      $
        /:
        invoke  2
        ~
        &

        h

sort    dup
        <:
        swap
        @
        return
