BIN = c-
CC = g++

SRCS = scanner.l  parser.y main.cpp ast.cpp symbolTable.cpp semantics.cpp yyerror.cpp codeGen.cpp
HDRS = scanType.h ast.h symbolTable.h semantics.h yyerror.h codeGen.h
OBJS = lex.yy.o parser.tab.o main.o ast.o symbolTable.o semantics.o yyerror.o codeGen.o

$(BIN) : $(OBJS)
	$(CC) $(OBJS) -o $(BIN) -g

main.o : main.cpp
	$(CC) -c main.cpp -g

ast.o : ast.cpp ast.h
	$(CC) -c ast.cpp -g

symbolTable.o : symbolTable.cpp symbolTable.h
	$(CC) -c symbolTable.cpp -g

semantics.o : semantics.cpp semantics.h
	$(CC) -c semantics.cpp -g

yyerror.o : yyerror.cpp yyerror.h
	$(CC) -c yyerror.cpp -g

codeGen.o : codeGen.cpp codeGen.h
	$(CC) -c codeGen.cpp -g

lex.yy.c : scanner.l parser.tab.h scanType.h
	flex scanner.l

parser.tab.h parser.tab.c : parser.y
	bison -v -t -d parser.y

clean :
	rm -f *~ $(OBJS) $(BIN) lex.yy.c parser.tab.h parser.tab.c parser.output $(BIN).output *.tm

rtm :
	rm -f *.tm

tar : $(HDRS) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile
