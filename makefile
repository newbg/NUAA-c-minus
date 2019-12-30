SHELL=/bin/bash
target=main

$(target): 
	flex cminus.l
	bison -d cminus.y
	gcc -c *.c -g
	gcc -o $(target) *.o -g

test: 	
	@echo ./test2/test$(file).cm
	@./main ./test2/test$(file).cm $(file)
	
.PHONY.clean:
clean:
	-rm *.o *.tab.* lex.yy.* $(target)
