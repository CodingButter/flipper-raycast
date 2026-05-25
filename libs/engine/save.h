// Engine Save — thin wrappers around Flipper Storage for write-once /
// read-once binary blobs. Higher-level "what does it mean to save the
// game" lives in per-subsystem state.h files (e.g. raycast/state.h),
// which know how to lay their own data out.
//
// Paths are absolute (e.g. /ext/apps_data/raycast/save.bin). The Flipper
// installer creates /ext/apps_data/<app_id> automatically when the .fap
// is installed via ufbt.
#pragma once

#include <stddef.h>

namespace Engine { namespace Save {

// Write the buffer to file at `path`, overwriting any existing file.
// Returns true if all `size` bytes were written.
bool write_file(const char* path, const void* data, size_t size);

// Read up to `max_size` bytes from `path` into `buffer`. Returns bytes
// actually read, or 0 if file is missing / unreadable.
size_t read_file(const char* path, void* buffer, size_t max_size);

// Cheap existence check — useful for "is there a save file?" without
// allocating a buffer.
bool exists(const char* path);

}} // namespace Engine::Save
