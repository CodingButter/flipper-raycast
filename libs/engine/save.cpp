#include "save.h"

#include <furi.h>
#include <storage/storage.h>

namespace Engine { namespace Save {

bool write_file(const char* path, const void* data, size_t size) {
    Storage* storage = (Storage*)furi_record_open(RECORD_STORAGE);
    File*    file    = storage_file_alloc(storage);
    bool     ok      = false;

    if (storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        size_t written = storage_file_write(file, data, size);
        ok = (written == size);
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

size_t read_file(const char* path, void* buffer, size_t max_size) {
    Storage* storage    = (Storage*)furi_record_open(RECORD_STORAGE);
    File*    file       = storage_file_alloc(storage);
    size_t   bytes_read = 0;

    if (storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        bytes_read = storage_file_read(file, buffer, max_size);
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return bytes_read;
}

bool exists(const char* path) {
    Storage* storage = (Storage*)furi_record_open(RECORD_STORAGE);
    bool     e       = storage_file_exists(storage, path);
    furi_record_close(RECORD_STORAGE);
    return e;
}

}} // namespace Engine::Save
