all: strlen-demo-without-dropit strlen-demo switch-demo-without-dropit switch-demo

strlen-demo: strlen-demo.c 
	gcc -g strlen-demo.c -o strlen-demo -O3 -lpthread -march=native 

strlen-demo-without-dropit: strlen-demo.c 
	gcc -g strlen-demo.c -o strlen-demo-without-dropit -DDISABLE_DROPIT -O3 -lpthread -march=native 

switch-demo: switch-demo.c
	gcc -g switch-demo.c -o switch-demo -O3 -lpthread -march=native 

switch-demo-without-dropit: switch-demo.c
	gcc -g switch-demo.c -o switch-demo-without-dropit -DDISABLE_DROPIT -O3 -lpthread -march=native 
	

clean:
	rm -f strlen-demo strlen-demo-without-dropit switch-demo switch-demo-without-dropit core
	
