# Build wrapper: regenerate the asset atlas from BMPs, then invoke ufbt.
#
# Why a Makefile and not fap_extbuild in application.fam: ufbt's
# fap_extbuild registers a Command target but doesn't add the output
# to the source list, and the variant-dir path resolution doesn't
# match the source glob — so the .cpp ends up as an orphaned target
# SCons won't compile. Wrapping at the make level dodges all of that
# and is what your editor's "Build" command should call.

UFBT       ?= ufbt
PYTHON     ?= python3
BUNDLE_PY  := scripts/bundler/bundle.py
MANIFEST   := scripts/bundler/manifest.json
ATLAS_SRC  := game/assets/assets.cpp
ATLAS_HDR  := game/assets/assets.h

# Rebuild atlas only when its inputs change. This makes incremental
# builds cheap — touching a .cpp doesn't re-run the bundler.
ATLAS_INPUTS := $(MANIFEST) $(BUNDLE_PY) scripts/bundler/atlas.py $(wildcard images/*.bmp)

.PHONY: all clean flash bundle

all: $(ATLAS_SRC) $(ATLAS_HDR)
	$(UFBT)

$(ATLAS_SRC) $(ATLAS_HDR): $(ATLAS_INPUTS)
	$(PYTHON) $(BUNDLE_PY)

bundle: $(ATLAS_SRC) $(ATLAS_HDR)

flash: all
	$(UFBT) flash

clean:
	$(UFBT) -c
	rm -f $(ATLAS_SRC) $(ATLAS_HDR)
