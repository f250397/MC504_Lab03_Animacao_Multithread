c_flag = -Wno-implicit-function-declaration

main.exe: main.c
	gcc -g -o main.exe main.c -lm

main.o: main.c
	gcc $(c_flag) -g -c main.c -lm

clean:
	rm -f main.o funcoes.o global_var.o main.exe