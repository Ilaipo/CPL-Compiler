LIBS = $(addprefix lib/, triples symbols patches)
FRONTEND = $(addprefix src/, cpl)
BACKEND = $(addprefix src/, quad)
MAIN = $(addprefix src/, cpq)

FRONTEND_LEX = $(addsuffix .lex, $(FRONTEND))
FRONTEND_BISON = $(addsuffix .y, $(FRONTEND))
OBJS = $(addsuffix .o, $(addsuffix .tab, $(FRONTEND)) \
	   					$(BACKEND) $(MAIN) $(LIBS) )
cpq: $(OBJS)
	gcc -ansi -Wall $(OBJS) -ll -o cpq

$(addsuffix .tab.c, $(FRONTEND)): $(FRONTEND_LEX) $(FRONTEND_BISON)
	flex -o $(addsuffix .yy.c, $(FRONTEND)) $(FRONTEND_LEX) 
	bison -d $(FRONTEND_BISON) -o $(addsuffix .tab.c, $(FRONTEND))

%.o: %.c
	gcc -ansi -Wall -c $< -o $@

clean:
	rm -f cpq $(OBJS) $(addsuffix .yy.c, $(FRONTEND)) $(addsuffix .tab.*, $(FRONTEND))
