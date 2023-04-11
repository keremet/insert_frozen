MODULE_big = insert_frozen
OBJS = \
	$(WIN32RES) \
	insert_frozen.o

EXTENSION = insert_frozen
DATA = insert_frozen--0.1.sql
PGFILEDESC = "insert_frozen - create log records"

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
