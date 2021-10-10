out/main: out/main.o out/builtins.o
	gcc -o out/main out/main.o out/builtins.o
out/main.o: src/main.c src/builtins.h
	gcc -c -g src/main.c -o out/main.o
out/builtins.o: src/builtins.c src/builtins.h
	gcc -c -g src/builtins.c -o out/builtins.o
