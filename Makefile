MODULE_big = insert_frozen
OBJS = \
	$(WIN32RES) \
	insert_frozen.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
