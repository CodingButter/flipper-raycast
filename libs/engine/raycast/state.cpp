#include "state.h"
#include "camera.h"
#include "world_api.h"
#include "../save.h"

#include <stdint.h>
#include <string.h>

namespace Engine { namespace Raycast {

namespace {
constexpr uint32_t MAGIC   = 0x52594354;  // 'RYCT'
constexpr uint32_t VERSION = 1;

// Header is laid out manually so we control padding (no `struct`-with-
// implicit-padding sneaking into the on-disk layout).
struct Header {
    uint32_t magic;
    uint32_t version;
    Vec2     cam_pos;
    float    cam_rotation;
    uint8_t  world_cols;
    uint8_t  world_rows;
    uint8_t  _pad[2];   // explicit padding
};
static_assert(sizeof(Header) == 24, "Header layout drift");

// One stack buffer big enough for both header + a generously-sized world.
constexpr size_t MAX_WORLD_BYTES = 1024;   // covers 32x32 worlds
}

bool save_state(const char* path, const Camera& camera) {
    const size_t world_size = (size_t)WORLD_COLS * (size_t)WORLD_ROWS;
    if (world_size > MAX_WORLD_BYTES) return false;

    uint8_t buf[sizeof(Header) + MAX_WORLD_BYTES];

    Header hdr{};
    hdr.magic        = MAGIC;
    hdr.version      = VERSION;
    hdr.cam_pos      = camera.position;
    hdr.cam_rotation = camera.rotation;
    hdr.world_cols   = (uint8_t)WORLD_COLS;
    hdr.world_rows   = (uint8_t)WORLD_ROWS;

    memcpy(buf,                  &hdr,  sizeof(Header));
    memcpy(buf + sizeof(Header), WORLD, world_size);

    return Save::write_file(path, buf, sizeof(Header) + world_size);
}

bool load_state(const char* path, Camera& camera) {
    const size_t world_size = (size_t)WORLD_COLS * (size_t)WORLD_ROWS;
    if (world_size > MAX_WORLD_BYTES) return false;

    uint8_t buf[sizeof(Header) + MAX_WORLD_BYTES];
    const size_t want = sizeof(Header) + world_size;
    if (Save::read_file(path, buf, want) != want) return false;

    Header hdr;
    memcpy(&hdr, buf, sizeof(Header));
    if (hdr.magic   != MAGIC)            return false;
    if (hdr.version != VERSION)          return false;
    if (hdr.world_cols != WORLD_COLS)    return false;
    if (hdr.world_rows != WORLD_ROWS)    return false;

    camera.position = hdr.cam_pos;
    camera.rotation = hdr.cam_rotation;
    memcpy(WORLD, buf + sizeof(Header), world_size);

    update_view_frustum(camera);    // resync derived fields
    return true;
}

}} // namespace Engine::Raycast

// ====================================================================
// Future: ECS state serialization
// --------------------------------------------------------------------
// To persist entities + components, the engine needs a stable way to
// re-resolve any pointer-valued component (e.g. AnimSprite::sheet) after
// load. Sketch:
//   1. Asset registry hands out integer IDs for SpriteSheets / Textures
//      at startup (or at registration time).
//   2. ECS save writes, for each live entity: mask + a contiguous blob
//      of every component the mask says is present, with pointers
//      replaced by their asset IDs.
//   3. ECS load: reset(), then for each entity in the file: create() +
//      write component slots + add(mask), with IDs resolved back to
//      pointers via the asset registry.
// Left as future work — current save only covers camera + world.
