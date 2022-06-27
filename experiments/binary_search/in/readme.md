# Mapeamento de registradores (C->MIPS)

## Valores principais
- |a|: R1
- ini: R2
- fim: R3
- pos: R4
- target: R5
- a[pos]: R7
- retorno: R10 

## Valores auxiliares
- R31: const int(1)
- R30: const int(2)
- R6: controle de saída do loop

## Memória
- \[0\]: |a|
- \[1...|a|\]: a
- \[50\]: target
- \[499\]: índice de retorno da busca