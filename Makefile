ifndef NAVISERVER
    NAVISERVER  = /usr/local/ns
endif

# OCaml configuration
OCAMLC 	 	= ocamlc
OCAMLMKLIB 	= ocamlmklib
OCAMLHOME       = $(shell $(OCAMLC) -where)
OCAMLCFLAGS 	= -g -w s -thread
OCAMLLIBS	= -L$(OCAMLHOME) -lcamlrun -ltermcap -lunix -lstr -lnaviserver
OCAMLLDFLAGS	= -cclib "-fPIC -shared $(OCAMLLIBS) $(LDFLAGS) $(LIBS) $(LDRPATH)"
# Required OCaml modules
OCAMLMODS	= naviserver.cma dynlink.cma str.cma unix.cma
OCAMLOBJS 	= nsocaml.cmo

# NaviServer configuration
MOD		= nsocaml.so
BUILD		+= naviserver.cma
CLEAN		+= clean-ocaml
CFLAGS	 	= -I$(OCAMLHOME)
OBJS     	= nsocaml.o
MODLIBS		= -L$(OCAMLHOME) -lcamlrun -ltermcap -lunix -lstr -lnaviserver

NSLIB		= naviserver

include  $(NAVISERVER)/include/Makefile.module

# Custom compilcation of Naviserver module
nsocaml.so: $(OCAMLOBJS) $(OBJS) install-ocaml
	$(OCAMLC) -linkall -custom $(OCAMLCFLAGS) $(OBJS) $(OCAMLMODS) $(OCAMLOBJS) -o $@ $(OCAMLLDFLAGS)

$(NSLIB).cma:	$(NSLIB).cmo $(NSLIB).o $(NSLIB).ml
	$(OCAMLMKLIB) -o $(NSLIB) $(NSLIB).cmo
	$(OCAMLMKLIB) -o $(NSLIB) $(NSLIB).o $(LDFLAGS) $(LIBS) $(LDRPATH)
	$(RANLIB) lib$(NSLIB).a

$(NSLIB).o:	$(NSLIB).c
	$(OCAMLC) -I $(NAVISERVER)/include -c $(NSLIB).c -o $@
	
%.cmi: %.mli
	$(OCAMLC) $(OCAMLCFLAGS) -c $<
        
%.cmo: %.ml
	$(OCAMLC) $(OCAMLCFLAGS) -c $<

clean-ocaml:
	rm -rf *.cma *.cmo *.cmi *.o *.so *~ *.a
	make -C test clean

world:	clean all install tests

tests:
	make -C test all

install-ocaml:
	if test -f dll$(NSLIB).so; then cp -f dll$(NSLIB).so $(OCAMLHOME)/stublibs; fi
	cp -f lib$(NSLIB).a $(OCAMLHOME)/lib$(NSLIB).a
	cp -f $(NSLIB).cma $(NSLIB).cmi $(OCAMLHOME)

