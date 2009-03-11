# OCaml configuration
CFLAGS 	= -g -w s -thread

OBJS	= ns_info.cmo ns_server.cmo ns_conn.cmo ns_set.cmo ns_nsv.cmo

tests:	all

all:	$(OBJS)
	
%.cmi: %.mli
	ocamlc $(CFLAGS) -c $<
        
%.cmo: %.ml
	ocamlc $(CFLAGS) -c $<

clean:
	rm -rf *.cma *.cmo *.cmi *.o *.so *~ *.a

