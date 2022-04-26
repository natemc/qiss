        j       5
        call    fac
        j       120
        =
        h

fac     dup         ; [x]     -> [x x]      {$[0=x;1;x*fac x-1]}
        j       0   ; [x x]   -> [0 x x]
        =           ; [0 x x] -> [0=x x]
        zb      L0  ; [0=x x] -> [x]        branches if x=0 is false!
        pop         ; [x]     -> []
        j       1   ; []      -> [1]
        return
L0      dup         ; [x x]
        j       1   ; [1 x x]
        swap        ; [x 1 x]
        -           ; [x-1 x]
        call fac    ; [fac[x-1] x]
        *           ; [x*fac x-1]
        return
