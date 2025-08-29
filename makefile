export DESTDIR

all : build-tld build-sar build-tdbg
install : install-tld install-sar install-tdbg
clean : clean-tld clean-sar clean-tdbg clean-libttc

build-libttc :
	@$(MAKE) -C libttc

clean-libttc :
	@$(MAKE) -C libttc clean

build-tld : build-libttc
	@$(MAKE) -C tld

install-tld : build-libttc
	@$(MAKE) -C tld install

clean-tld :
	@$(MAKE) -C tld clean

build-sar : build-libttc
	@$(MAKE) -C sar

install-sar : build-libttc
	@$(MAKE) -C sar install

clean-sar :
	@$(MAKE) -C sar clean

build-tdbg : build-libttc
	@$(MAKE) -C tdbg

install-tdbg : build-libttc
	@$(MAKE) -C tdbg install

clean-tdbg :
	@$(MAKE) -C tdbg clean

.PHONY : all install clean build-% install-% clean-%
