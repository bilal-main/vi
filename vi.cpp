#include "libs/raylib.h"
#include "libs/raymath.h"
#include <vector>
#include <cmath>

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
namespace GameConstants {
    constexpr float PLAYER_BASE_SPEED   = 5.0f;
    constexpr float GRAVITY             = -15.0f;
    constexpr float JUMP_SPEED          = 8.0f;
    constexpr float MOUSE_SENSITIVITY   = 0.003f;

    constexpr float EYE_STANDING  = 1.7f;
    constexpr float EYE_CROUCHING = 1.2f;
    constexpr float EYE_CRAWLING  = 0.2f;

    // Collision box dimensions (full: width, height, depth)
    constexpr Vector3 PLAYER_SIZE_STANDING  = { 0.3f, 1.8f, 0.3f };
    constexpr Vector3 PLAYER_SIZE_CROUCHING = { 0.8f, 1.3f, 0.8f };
    constexpr Vector3 PLAYER_SIZE_CRAWLING  = { 1.8f, 0.3f, 0.4f };
}

// ----------------------------------------------------------------------------
// Enums and data structures
// ----------------------------------------------------------------------------
enum class Stance { STANDING, CROUCHING, CRAWLING };

struct CollidableObject {
    Vector3 position;   // center
    Vector3 size;       // full extents
    Color   color;
};

struct PlayerState {
    Vector3 position;   // feet (bottom center)
    Vector3 velocity;
    Stance  stance   = Stance::STANDING;
    bool    onGround = false;
    bool    debugMode= false;
    float   yaw      = 0.0f;   // radians
    float   pitch    = 0.0f;
};

// ----------------------------------------------------------------------------
// Simple math helpers
// ----------------------------------------------------------------------------
namespace MathUtil {
    inline float Vec3LengthSq(const Vector3& v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }
}

// ----------------------------------------------------------------------------
// Collision utilities
// ----------------------------------------------------------------------------
namespace CollisionUtils {

// Returns player AABB (feet at bottom)
inline BoundingBox GetPlayerAABB(const Vector3& feetPos, Stance stance) {
    Vector3 half;
    switch (stance) {
        case Stance::STANDING:  half = Vector3Scale(GameConstants::PLAYER_SIZE_STANDING,  0.5f); break;
        case Stance::CROUCHING: half = Vector3Scale(GameConstants::PLAYER_SIZE_CROUCHING, 0.5f); break;
        case Stance::CRAWLING:  half = Vector3Scale(GameConstants::PLAYER_SIZE_CRAWLING,  0.5f); break;
        default:                half = {0.15f, 0.9f, 0.15f}; break;
    }
    BoundingBox box;
    box.min = { feetPos.x - half.x, feetPos.y, feetPos.z - half.z };
    box.max = { feetPos.x + half.x, feetPos.y + half.y * 2.0f, feetPos.z + half.z };
    return box;
}

// Slide‑and‑drag resolution against a static box.
inline void ResolveCollisionWithObject(PlayerState& player, const CollidableObject& obj) {
    BoundingBox playerBox = GetPlayerAABB(player.position, player.stance);
    BoundingBox objBox = {
        Vector3Subtract(obj.position, Vector3Scale(obj.size, 0.5f)),
        Vector3Add(obj.position, Vector3Scale(obj.size, 0.5f))
    };
    if (!CheckCollisionBoxes(playerBox, objBox)) return;

    Vector3 pc = { (playerBox.min.x + playerBox.max.x) * 0.5f,
                   (playerBox.min.y + playerBox.max.y) * 0.5f,
                   (playerBox.min.z + playerBox.max.z) * 0.5f };
    Vector3 ph = { (playerBox.max.x - playerBox.min.x) * 0.5f,
                   (playerBox.max.y - playerBox.min.y) * 0.5f,
                   (playerBox.max.z - playerBox.min.z) * 0.5f };
    Vector3 oc = obj.position;
    Vector3 oh = Vector3Scale(obj.size, 0.5f);

    float ox = ph.x + oh.x - fabsf(pc.x - oc.x);
    float oy = ph.y + oh.y - fabsf(pc.y - oc.y);
    float oz = ph.z + oh.z - fabsf(pc.z - oc.z);

    if (ox < oy && ox < oz) {
        float sign = (pc.x < oc.x) ? -1.0f : 1.0f;
        player.position.x += sign * ox;
        player.velocity.x = 0.0f;
    } else if (oy < oz) {
        float sign = (pc.y < oc.y) ? -1.0f : 1.0f;
        player.position.y += sign * oy;
        player.velocity.y = 0.0f;
        if (sign > 0.0f) player.onGround = true;
    } else {
        float sign = (pc.z < oc.z) ? -1.0f : 1.0f;
        player.position.z += sign * oz;
        player.velocity.z = 0.0f;
    }
}

// Object creation from a Vector3 size.
inline CollidableObject CreateObject(Vector3 position, Vector3 size, Color color) {
    return { position, size, color };
}

// Overload with separate width, height, depth.
inline CollidableObject CreateObject(Vector3 position, float width, float height, float depth, Color color) {
    return { position, { width, height, depth }, color };
}

} // namespace CollisionUtils

// ----------------------------------------------------------------------------
// Player system
// ----------------------------------------------------------------------------
namespace PlayerSystem {

inline void InitPlayer(PlayerState& player) {
    player.position = { 0.0f, 0.0f, 0.0f };
    player.velocity = { 0.0f, 0.0f, 0.0f };
    player.stance   = Stance::STANDING;
    player.onGround = false;
    player.debugMode= false;
    player.yaw      = 0.0f;
    player.pitch    = 0.0f;
}

inline Camera3D CreateCamera() {
    Camera3D camera = { 0 };
    camera.position = { 0.0f, GameConstants::EYE_STANDING, 0.0f };
    camera.target   = { 0.0f, GameConstants::EYE_STANDING, 1.0f };
    camera.up       = { 0.0f, 1.0f, 0.0f };
    camera.fovy     = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

inline void UpdateCameraFromPlayer(Camera3D& camera, const PlayerState& player) {
    float eyeHeight = GameConstants::EYE_STANDING;
    switch (player.stance) {
        case Stance::STANDING:  eyeHeight = GameConstants::EYE_STANDING;  break;
        case Stance::CROUCHING: eyeHeight = GameConstants::EYE_CROUCHING; break;
        case Stance::CRAWLING:  eyeHeight = GameConstants::EYE_CRAWLING;  break;
    }
    camera.position = Vector3Add(player.position, { 0.0f, eyeHeight, 0.0f });

    Vector3 forward = {
        sinf(player.yaw) * cosf(player.pitch),
        sinf(player.pitch),
        cosf(player.yaw) * cosf(player.pitch)
    };
    camera.target = Vector3Add(camera.position, forward);
}

// Shared input processing (mouse, stances, debug toggle)
inline void HandleInput(PlayerState& player, Camera3D& camera) {
    Vector2 mouseDelta = GetMouseDelta();
    player.yaw   -= mouseDelta.x * GameConstants::MOUSE_SENSITIVITY;
    player.pitch -= mouseDelta.y * GameConstants::MOUSE_SENSITIVITY;

    constexpr float maxPitch = 1.5f;
    if (player.pitch > maxPitch)  player.pitch = maxPitch;
    if (player.pitch < -maxPitch) player.pitch = -maxPitch;

    // Stances
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_C)) {
        if (IsKeyDown(KEY_X))
            player.stance = Stance::CRAWLING;
        else
            player.stance = Stance::CROUCHING;
    }
    if (IsKeyPressed(KEY_C) && !IsKeyDown(KEY_LEFT_SHIFT))
        player.stance = Stance::STANDING;

    // Debug toggle
    static bool toggleGuard = false;
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_D) && IsKeyDown(KEY_B)) {
        if (!toggleGuard) {
            player.debugMode = !player.debugMode;
            toggleGuard = true;
            if (player.debugMode) {
                player.position = camera.position;
                player.velocity = {0,0,0};
            } else {
                player.position = camera.position;
                player.position.y = 0.0f;
                player.velocity = {0,0,0};
            }
        }
    } else {
        toggleGuard = false;
    }
}

// Flight movement (no gravity, no collisions)
inline void UpdateDebugMovement(PlayerState& player, Camera3D& camera, float delta) {
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 right   = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

    float inputX = 0.0f, inputZ = 0.0f;
    if (IsKeyDown(KEY_W)) inputZ += 1.0f;
    if (IsKeyDown(KEY_S)) inputZ -= 1.0f;
    if (IsKeyDown(KEY_D)) inputX += 1.0f;
    if (IsKeyDown(KEY_A)) inputX -= 1.0f;

    Vector3 moveDir = Vector3Add(Vector3Scale(forward, inputZ), Vector3Scale(right, inputX));
    if (MathUtil::Vec3LengthSq(moveDir) > 0.0f)
        moveDir = Vector3Normalize(moveDir);

    const float speed = GameConstants::PLAYER_BASE_SPEED * 5.0f;
    Vector3 displacement = Vector3Scale(moveDir, speed * delta);

    if (IsKeyDown(KEY_SPACE))         displacement.y += speed * delta;
    if (IsKeyDown(KEY_LEFT_CONTROL))  displacement.y -= speed * delta;

    player.position = Vector3Add(player.position, displacement);
    camera.position = player.position;

    Vector3 front = {
        sinf(player.yaw) * cosf(player.pitch),
        sinf(player.pitch),
        cosf(player.yaw) * cosf(player.pitch)
    };
    camera.target = Vector3Add(camera.position, front);
}

// Normal ground movement with gravity and collisions
inline void UpdateNormalMovement(PlayerState& player, Camera3D& camera, float delta,
                                 const std::vector<CollidableObject>& objects) {
    // Speed multiplier
    float speedMul = 1.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_W))
        speedMul = 3.0f;
    else if (player.stance == Stance::CROUCHING)
        speedMul = 0.4f;
    else if (player.stance == Stance::CRAWLING)
        speedMul = 0.2f;

    float speed = GameConstants::PLAYER_BASE_SPEED * speedMul;

    float inputX = 0.0f, inputZ = 0.0f;
    if (IsKeyDown(KEY_W)) inputZ += 1.0f;
    if (IsKeyDown(KEY_S)) inputZ -= 1.0f;
    if (IsKeyDown(KEY_D)) inputX -= 1.0f;
    if (IsKeyDown(KEY_A)) inputX += 1.0f;

    Vector3 forwardH = { sinf(player.yaw), 0.0f, cosf(player.yaw) };
    Vector3 rightH   = { cosf(player.yaw), 0.0f, -sinf(player.yaw) };

    Vector3 moveDir = Vector3Add(Vector3Scale(forwardH, inputZ), Vector3Scale(rightH, inputX));
    if (MathUtil::Vec3LengthSq(moveDir) > 0.0f)
        moveDir = Vector3Normalize(moveDir);

    player.velocity.x = moveDir.x * speed;
    player.velocity.z = moveDir.z * speed;

    // Gravity & integration
    player.velocity.y += GameConstants::GRAVITY * delta;
    player.position = Vector3Add(player.position, Vector3Scale(player.velocity, delta));

    // Object collisions
    for (const auto& obj : objects)
        CollisionUtils::ResolveCollisionWithObject(player, obj);

    // Floor clamp
    if (player.position.y < 0.0f) {
        player.position.y = 0.0f;
        player.velocity.y = 0.0f;
        player.onGround = true;
    } else {
        player.onGround = false;
    }

    // Jump
    if (IsKeyPressed(KEY_SPACE)  && player.stance != Stance::CRAWLING ||player.stance!=Stance::CROUCHING) {
        player.velocity.y = GameConstants::JUMP_SPEED;
        player.onGround = false;
    }

    UpdateCameraFromPlayer(camera, player);
}

// Dispatcher
inline void UpdatePlayer(PlayerState& player, Camera3D& camera, float delta,
                         const std::vector<CollidableObject>& objects) {
    HandleInput(player, camera);
    if (player.debugMode)
        UpdateDebugMovement(player, camera, delta);
    else
        UpdateNormalMovement(player, camera, delta, objects);
}

} // namespace PlayerSystem

// ----------------------------------------------------------------------------
// World
// ----------------------------------------------------------------------------
namespace World {

inline void InitWorld(std::vector<CollidableObject>& objects) {
    objects.clear();
    using namespace CollisionUtils;
    objects.push_back(CreateObject({ 3.0f, 0.5f, 2.0f }, { 1.0f, 1.0f, 1.0f }, RED));
    objects.push_back(CreateObject({ -2.0f, 1.0f, -4.0f }, { 2.0f, 2.0f, 2.0f }, BLUE));
    objects.push_back(CreateObject({ 0.0f, 0.25f, -6.0f }, { 4.0f, 0.5f, 1.0f }, GREEN));
    objects.push_back(CreateObject({ -5.0f, 0.75f, 5.0f }, { 1.5f, 1.5f, 1.5f }, YELLOW));
}

inline void DrawWorld(const std::vector<CollidableObject>& objects) {
    for (const auto& obj : objects) {
        DrawCubeV(obj.position, obj.size, obj.color);
        DrawCubeWiresV(obj.position, obj.size, BLACK);
    }
}

inline void DrawFloorGrid() {
    const float minX = -6.5f, maxX = 6.5f;
    const float minZ = -10.0f, maxZ = 10.0f;
    const Color gridColor = { 80, 80, 80, 255 };
    for (int i = 0; i <= 13; ++i) {
        float x = minX + i * 1.0f;
        DrawLine3D({ x, 0.0f, minZ }, { x, 0.0f, maxZ }, gridColor);
    }
    for (int j = 0; j <= 20; ++j) {
        float z = minZ + j * 1.0f;
        DrawLine3D({ minX, 0.0f, z }, { maxX, 0.0f, z }, gridColor);
    }
}

} // namespace World

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
void main_logic(){
     const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Walking Simulator");
    SetTargetFPS(60);
    DisableCursor();

    PlayerState player;
    PlayerSystem::InitPlayer(player);
    Camera3D camera = PlayerSystem::CreateCamera();

    std::vector<CollidableObject> objects;
    World::InitWorld(objects);

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        PlayerSystem::UpdatePlayer(player, camera, delta, objects);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
        World::DrawFloorGrid();
        World::DrawWorld(objects);
        EndMode3D();

        const char* stanceStr = "Standing";
        if (player.stance == Stance::CROUCHING) stanceStr = "Crouching";
        else if (player.stance == Stance::CRAWLING) stanceStr = "Crawling";
        if (player.debugMode) stanceStr = "DEBUG FLIGHT";

        DrawText(TextFormat("Stance: %s", stanceStr), 10, 10, 20, WHITE);
        DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
}
int main() {
   main_logic();
    return 0;
}