# MAVN → MIPS prevodilac

Prevodilac koji program napisan u jeziku **MAVN** (Mips Asembler Visokog Nivoa) prevodi u **MIPS 32-bit asemblerski kod**.
MAVN omogućava pisanje asemblerskog koda sa neograničenim brojem *registarskih promjenljivih*, a prevodilac sam
odlučuje koja promjenljiva će se naći u kom stvarnom registru procesora.

Projekat je urađen u okviru predmeta **Osnovi paralelnog programiranja i softverski alati (OPPISA)**.

## Faze prevođenja

1. **Leksička analiza**: konačni automat dijeli ulazni tekst na tokene (`LexicalAnalysis`, `FiniteStateMachine`).
2. **Sintaksna analiza**: provjera gramatike i pravljenje međureprezentacije: liste instrukcija, promjenljivih i labela (`SyntaxAnalysis`, `IR`).
3. **Analiza životnog vijeka promjenljivih**: računanje `in`/`out` skupova za svaku instrukciju (`LivenessAnalysis`).
4. **Graf smetnji**: određuje koje promjenljive ne smiju dijeliti isti registar (`InterferenceGraph`).
5. **Dodjela resursa**: bojenje grafa (simplify / select) sa 4 registra `$t0`–`$t3`, uz *spill* u memoriju kad registara nema dovoljno (`ResourceAllocation`).
6. **Generisanje koda**: ispis `.s` fajla sa `.globl`, `.data` i `.text` sekcijom (`main.cpp`).

## Podržane instrukcije

`add`, `addi`, `sub`, `mul`, `and`, `la`, `li`, `lw`, `sw`, `b`, `bltz`, `beq`, `nop`

Deklaracije:
- `_mem ime vrijednost;`: memorijska promjenljiva
- `_reg ime;`: registarska promjenljiva
- `_func ime;`: početak funkcije

## Primjer

Ulaz (`examples/simple.mavn`):
```
_mem m1 6;
_mem m2 5;

_reg r1;
_reg r2;
_reg r3;
_reg r4;
_reg r5;

_func main;
	la		r4, m1;
	lw		r1, 0(r4);
	la		r5, m2;
	lw		r2, 0(r5);
	add		r3, r1, r2;
```

Izlaz (`examples/simple.s`):
```
.globl main

.data
m1:	.word 6
m2:	.word 5

.text
main:
	la	$t0, m1
	lw	$t1, 0($t0)
	la	$t0, m2
	lw	$t0, 0($t0)
	add	$t0, $t1, $t0
```

Pet registarskih promjenljivih (`r1`–`r5`) se smjestilo u samo dva stvarna registra.
Složeniji primjer sa petljom je u `examples/multiply.mavn`.

## Pokretanje

1. Otvori `src/LexicalAnalysis.sln` u **Visual Studiju**.
2. Pokreni projekat (F5).
3. Program čita `examples/simple.mavn` i upisuje rezultat u `examples/simple.s`.
   Za drugi ulaz promijeni putanje `fileName` i `outputFileName` u `src/main.cpp`.

Dobijeni `.s` fajl može se pokrenuti u MIPS simulatoru (npr. **QtSPIM** ili **MARS**).

## Struktura

```
src/        izvorni kod prevodioca (C++)
examples/   primjeri ulaznih .mavn i izlaznih .s fajlova
doc/        dokumentacija projekta
```

Detaljna dokumentacija: [`doc/OPPISA_dokumentacija.pdf`](doc/OPPISA_dokumentacija.pdf)
