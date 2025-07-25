
all : build-tld build-sar
install : install-tld install-sar
clean : clean-tld clean-sar

build-tld :
	$(MAKE) -C tld

install-tld :
	$(MAKE) -C tld install

clean-tld :
	$(MAKE) -C tld clean

build-sar :
	$(MAKE) -C sar

install-sar :
	$(MAKE) -C sar install

clean-sar :
	$(MAKE) -C sar clean
.PHONY : all install clean build-% install-% clean-%
