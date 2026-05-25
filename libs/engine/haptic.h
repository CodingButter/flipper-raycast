// Haptic — vibration motor control with simple sequenced patterns.
//
// A "pattern" is a queue of (duration_ms, motor_on/off) segments. The
// state machine is advanced by Engine::Haptic::tick(dt) which is wired
// into FlipperApp::run(), so games don't have to think about timing.
//
// Time granularity is bounded by the tick rate — patterns shorter than
// one tick (~33 ms at 30 Hz) round up. For game feel that's fine; the
// motor itself has a few-ms response time anyway.
//
// Calling blast() / pulse() interrupts whatever's currently playing.
#pragma once

namespace Engine { namespace Haptic {

// Single sustained vibration for `duration_ms`. Good for "thump"
// events: explosions, big-weapon shots, taking damage.
void blast(int duration_ms);

// `count` short vibrations separated by `gap_ms` silence. Good for
// "tatta-tatta" feel — rapid weapons, alerts.
void pulse(int count, int on_ms, int gap_ms);

// Engine tick — advances the state machine. FlipperApp::run() calls it;
// don't call it yourself.
void tick(float dt);

// Force the motor off and clear any queued pattern. Call on app shutdown
// so the motor doesn't keep running after we exit.
void stop();

}} // namespace Engine::Haptic
