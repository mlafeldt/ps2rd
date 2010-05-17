.SILENT:

TOOLS = ntpbclient

all:
	@for i in $(TOOLS); do \
		echo "* [PC] Building tool $$i ..."; \
		$(MAKE) -C $$i; \
	done

clean:
	@for i in $(TOOLS); do \
		echo "* [IOP] Cleaning $$i ..."; \
		$(MAKE) -C $$i clean; \
	done
