        j       5          ; rhs (y)
        !:
        j       0          ; lhs (x)
        j       0          ; initial loop counter (i)
loop    dup                ; (#y)-i
        dup     3
        #:
        -
        zb      done
        dup                ; y[i]
        dup     3
        @
        rot     2          ; acc + y[i]
        +
        swap               ; ++i
        j       1
        +
        jump    loop
done    swap
        clean   2
        j       10
        =
        h
