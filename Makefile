SOURCE_DIR=src
HEADER_DIR=include
RESOURCE_DIR=resources
INTERFACE_DIR=interface
GENERATED_DIR=generated
BUILD_DIR=out

SOURCES=$(wildcard $(SOURCE_DIR)/*.c)
OBJECTS=$(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

RESOURCE_LIST=mdr.gresource.xml
RESOURCES=$(wildcard $(RESOURCE_DIR)/*)
GENERATED_RESOURCES=$(GENERATED_DIR)/resources.c
OBJECTS+=$(patsubst $(GENERATED_DIR)/%.c,$(BUILD_DIR)/%.o,$(GENERATED_RESOURCES))

INTERFACES=$(wildcard $(INTERFACE_DIR)/*.xml)
GENERATED_INTERFACE_HEADERS=$(patsubst $(INTERFACE_DIR)/%.xml,$(GENERATED_DIR)/%.h,$(INTERFACES))
GENERATED_INTERFACE_SOURCES=$(patsubst $(INTERFACE_DIR)/%.xml,$(GENERATED_DIR)/%.c,$(INTERFACES))
OBJECTS+=$(patsubst $(GENERATED_DIR)/%.c,$(BUILD_DIR)/%.o,$(GENERATED_INTERFACE_SOURCES))

TARGET=mdr-manager

CFLAGS=$(shell pkg-config --cflags gio-2.0 gtk+-3.0) \
	   -Wall \
	   -Wpedantic \
		-g \
	   -I $(HEADER_DIR)
LDFLAGS=$(shell pkg-config --libs gio-2.0 gtk+-3.0)

GDBUS_CODEGEN=$(shell pkg-config --variable=gdbus_codegen gio-2.0)
GLIB_COMPILE_RESOURCES=$(shell pkg-config --variable=glib_compile_resources gio-2.0)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(GENERATED_DIR)
	rm -f $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADER_DIR)/%.h $(GENERATED_INTERFACE_HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -I $(HEADER_DIR) -I $(GENERATED_DIR) -o $@ $<

$(BUILD_DIR)/%.o: $(GENERATED_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -I $(HEADER_DIR) -I $(GENERATED_DIR) -o $@ $<

$(GENERATED_DIR)/%.h: $(INTERFACE_DIR)/%.xml | $(GENERATED_DIR)
	$(GDBUS_CODEGEN) $< --header --output $@

$(GENERATED_DIR)/%.c: $(INTERFACE_DIR)/%.xml $(GENERATED_DIR)/%.h | $(GENERATED_DIR)
	$(GDBUS_CODEGEN) $< --body --output $@

$(GENERATED_RESOURCES): $(RESOURCE_LIST) $(RESOURCES) | $(GENERATED_DIR)
	$(GLIB_COMPILE_RESOURCES) $< --target=$@ --sourcedir=resources --generate-source

.PHONY: all clean

.PRECIOUS: $(GENERATED_INTERFACE_HEADERS)

$(BUILD_DIR):
	mkdir -p $@

$(GENERATED_DIR):
	mkdir -p $@

