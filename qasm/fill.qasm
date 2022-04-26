        ; fill atom
        j       0N
        j       42
        ^
        j       42
        =
        
        ; fill float with integer
        J       0 0 0 3 5
        j       0
        %
        j       100
        ^
        F       100 100 100 0 0f
        ~
        &
        h
