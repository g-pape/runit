DESTDIR=

PACKAGE=runit-2.3.1
DIRS=doc man etc package src

all: clean .doc .man $(PACKAGE).tar.gz

.doc:
	cd md && ./gen-html ../doc
	touch .doc

.man:
	cd md && ./gen-man ../man
	touch .man

$(PACKAGE).tar.gz:
	rm -rf TEMP
	mkdir -p TEMP/admin/$(PACKAGE)
	make -C src clean
	cp -a $(DIRS) TEMP/admin/$(PACKAGE)/
	ln -sf ../etc/debian TEMP/admin/$(PACKAGE)/doc/
	for i in TEMP/admin/$(PACKAGE)/etc/*; do \
	  test -d $$i && ln -s ../2 $$i/2; \
	done
	chmod -R g-ws TEMP/admin
	chmod +t TEMP/admin
	find TEMP -exec touch {} \;
	su -c '\
	  chown -R root:root TEMP/admin ; \
	  (cd TEMP && tar --exclude CVS -cpzf ../$(PACKAGE).tar.gz admin); \
	  rm -rf TEMP'

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;

cleaner: clean
	rm -f $(PACKAGE).tar.gz
	rm -f doc/*.html man/*.[0-9] .doc .man
