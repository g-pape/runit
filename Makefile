DESTDIR=

PACKAGE=runit-2.3.1
DIRS=doc man etc package src
RUNIT_BINARIES=chpst runit runit-init runsv runsvchdir runsvdir sv svlogd

PANDOC = pandoc

MANPAGES = man/chpst.8 man/runit-init.8 man/runit.8 man/runsv.8 \
		   man/runsvchdir.8 man/runsvdir.8 man/sv.8 man/svlogd.8
HTML_MAN = doc/chpst.8.html doc/runit-init.8.html doc/runit.8.html \
		   doc/runsv.8.html doc/runsvchdir.8.html doc/runsvdir.8.html \
		   doc/sv.8.html doc/svlogd.8.html
HTML_DOCS = doc/benefits.html doc/dependencies.html doc/faq.html \
			doc/index.html doc/install.html doc/replaceinit.html \
			doc/runlevels.html doc/runscripts.html doc/upgrade.html \
			doc/useinit.html

GEN_MAN = $(PANDOC) -s -t man $< | sed 's/"" "" ""$$//g' > $@
GEN_HTML = $(PANDOC) -s -t html --template md/template.html $< > $@
GEN_MAN_HTML = $(PANDOC) -s -t html --template md/template-man.html \
			   --shift-heading-level-by 2 $< > $@

all: clean doc man $(RUNIT_BINARIES)

man: $(MANPAGES)

doc: $(HTML_DOCS) $(HTML_MAN)

$(RUNIT_BINARIES):
	$(MAKE) -C src $@

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



man/chpst.8: md/chpst.8.md
	$(GEN_MAN)
man/runit-init.8: md/runit-init.8.md
	$(GEN_MAN)
man/runit.8: md/runit.8.md
	$(GEN_MAN)
man/runsv.8: md/runsv.8.md
	$(GEN_MAN)
man/runsvchdir.8: md/runsvchdir.8.md
	$(GEN_MAN)
man/runsvdir.8: md/runsvdir.8.md
	$(GEN_MAN)
man/sv.8: md/sv.8.md
	$(GEN_MAN)
man/svlogd.8: md/svlogd.8.md
	$(GEN_MAN)

doc/chpst.8.html: md/chpst.8.md
	$(GEN_MAN_HTML)
doc/runit-init.8.html: md/runit-init.8.md
	$(GEN_MAN_HTML)
doc/runit.8.html: md/runit.8.md
	$(GEN_MAN_HTML)
doc/runsv.8.html: md/runsv.8.md
	$(GEN_MAN_HTML)
doc/runsvchdir.8.html: md/runsvchdir.8.md
	$(GEN_MAN_HTML)
doc/runsvdir.8.html: md/runsvdir.8.md
	$(GEN_MAN_HTML)
doc/sv.8.html: md/sv.8.md
	$(GEN_MAN_HTML)
doc/svlogd.8.html: md/svlogd.8.md
	$(GEN_MAN_HTML)

doc/benefits.html: md/benefits.md
	$(GEN_HTML)
doc/dependencies.html: md/dependencies.md
	$(GEN_HTML)
doc/faq.html: md/faq.md
	$(GEN_HTML)
doc/index.html: md/index.md
	$(GEN_HTML)
doc/install.html: md/install.md
	$(GEN_HTML)
doc/replaceinit.html: md/replaceinit.md
	$(GEN_HTML)
doc/runlevels.html: md/runlevels.md
	$(GEN_HTML)
doc/runscripts.html: md/runscripts.md
	$(GEN_HTML)
doc/upgrade.html: md/upgrade.md
	$(GEN_HTML)
doc/useinit.html: md/useinit.md
	$(GEN_HTML)
