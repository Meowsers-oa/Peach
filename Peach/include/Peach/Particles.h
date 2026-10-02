//
// Created by Štěpán Toman on 02.10.2026.
//

#ifndef PEACH_PARTICLES_H
#define PEACH_PARTICLES_H

#include <Peach/Common.h>

// amount is the fixed pool capacity. Speed is pixels/second; lifetime defaults to one second.
mParticleEmitter mParticleEmitterCreate(float posX, float posY, uint amount, float speed, int startSize, int endSize, mColor startColor, mColor endColor);
void mParticlesStartSpawning(mParticleEmitter* emitter);
void mParticlesStopSpawning(mParticleEmitter* emitter);
void mParticlesToggleSpawning(mParticleEmitter* emitter);
void mParticlesSetSpawning(mParticleEmitter* emitter, M_BOOL active);

// Call once per frame. Stopping emission lets existing particles finish.
void mParticleEmitterUpdate(mContext* ctx, mParticleEmitter* emitter);
int mDrawParticles(mContext* ctx, const mParticleEmitter* emitter);
// Unregistered emitters only; registered emitters are released by mRemoveResource/mEnd.
void mParticleEmitterDestroy(mParticleEmitter* emitter);


#endif //PEACH_PARTICLES_H
