BUILD_DIR := build
BUILD_TYPE := Release
NUM_JOBS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

.DEFAULT_GOAL := run
.PHONY: configure build rebuild clean install run




configure:
	@echo "→ Configuring cc ($(BUILD_TYPE))..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_POLICY_VERSION_MINIMUM=4.2.0 \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON


build: configure
	@echo "→ Building cc..."
	@cmake --build $(BUILD_DIR) -j$(NUM_JOBS)
	@echo "✓ Build complete"

rebuild: clean build

debug:
	@$(MAKE) run BUILD_TYPE=Debug

release:
	@$(MAKE) run BUILD_TYPE=Release


clean:
	@echo "→ Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "✓ Clean complete"

swap:
	@echo "→ Swapping shaders..."
	@rm -rf $(BUILD_DIR)/bin/resources/
	@cp -r studio/resources $(BUILD_DIR)/bin/resources/


run: build swap
	@echo "→ Running cc studio..."
	@cd $(BUILD_DIR)/bin && ./studio


all: build


