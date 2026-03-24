# Parity
The Parity programming language
# Compile
compiler:
```
bash bd.sh
```
program for linux:
```
./parity input.par -o output.asm -64

nasm -f elf64 output.asm -o prog.o
ld prog.o -o prog

./prog
```


