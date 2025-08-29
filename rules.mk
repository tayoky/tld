all : $(BUILDDIR)/$(NAME)
$(BUILDDIR)/%.o : $(SRCDIR)/%.c 
	@echo '[compiling $^]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ -c $^ $(CFLAGS)
clean :
	rm -fr build

$(BUILDDIR)/$(NAME) : $(OBJ)
	@echo '[linking into $@]'
	@mkdir -p $(shell dirname $@)
	@$(CC) -o $@ $^ $(CFLAGS) -L../libttc/build -lttc

install : all
	@echo '[installing $(NAME)]'
	@mkdir -p $(PREFIX)/bin
	@cp $(BUILDDIR)/$(NAME) $(PREFIX)/bin/$(NAME)
uninstall :
	rm -rf $(PREFIX)/bin/$(NAME)

config.mk :
	$(error run ./configure before running make)

.PHONY : all $(BUILDIR)/$(NAME) install uninstall clean
