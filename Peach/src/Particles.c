#include <Peach/Particles.h>
#include <Peach/Renderer.h>
#include <Peach/Utils.h>
#include <math.h>
#include <stdlib.h>

static M_BOOL validEmitter(const mParticleEmitter* emitter) {
    return emitter != NULL && emitter->particles != NULL && emitter->amount > 0 &&
        isfinite(emitter->x) && isfinite(emitter->y) && isfinite(emitter->speed) && emitter->speed >= 0 &&
        isfinite(emitter->lifetime) && emitter->lifetime > 0 && emitter->startSize >= 0 && emitter->endSize >= 0;
}

mParticleEmitter mParticleEmitterCreate(float posX, float posY, uint amount, float speed, int startSize,
    int endSize, mColor startColor, mColor endColor) {
    mParticleEmitter emitter = {.x = posX, .y = posY, .speed = speed, .amount = amount,
        .startSize = startSize, .endSize = endSize, .startColor = startColor, .endColor = endColor, .lifetime = 1};
    if (!isfinite(posX) || !isfinite(posY) || !isfinite(speed) || speed < 0 || startSize < 0 || endSize < 0 ||
        amount == 0 || SIZE_MAX / amount < sizeof(mParticle)) return (mParticleEmitter){0};
    emitter.particles = calloc(amount, sizeof(mParticle));
    if (emitter.particles == NULL) return (mParticleEmitter){0};
    return emitter;
}

void mParticlesStartSpawning(mParticleEmitter* emitter) {
    mParticlesSetSpawning(emitter, M_TRUE);
}

void mParticlesStopSpawning(mParticleEmitter* emitter) {
    mParticlesSetSpawning(emitter, M_FALSE);
}

void mParticlesToggleSpawning(mParticleEmitter* emitter) {
    if (emitter != NULL) mParticlesSetSpawning(emitter, !emitter->active);
}

void mParticlesSetSpawning(mParticleEmitter* emitter, M_BOOL active) {
    if (emitter == NULL) return;
    emitter->active = active != M_FALSE;
    if (!emitter->active) emitter->spawnTime = 0;
}

static void advanceParticle(mParticleEmitter* emitter, mParticle* particle, double deltaTime) {
    particle->t += deltaTime;
    if (particle->t >= particle->lifetime) {
        particle->alive = M_FALSE;
        return;
    }
    particle->x += particle->velocityX * deltaTime;
    particle->y += particle->velocityY * deltaTime;
    float t = (float)(particle->t / particle->lifetime);
    particle->size = mLerp(emitter->startSize, emitter->endSize, t);
    particle->color = mColorBlend(&emitter->startColor, &emitter->endColor, t);
}

void mParticleEmitterUpdate(mContext* ctx, mParticleEmitter* emitter) {
    if (ctx == NULL || !validEmitter(emitter)) return;
    double deltaTime = ctx->time.deltaTime;
    if (!isfinite(deltaTime) || deltaTime <= 0) return;
    emitter->count = 0;
    for (uint i = 0; i < emitter->amount; i++) {
        mParticle* particle = &emitter->particles[i];
        if (particle->alive) advanceParticle(emitter, particle, deltaTime);
        if (particle->alive) emitter->count++;
    }
    if (!emitter->active) return;

    double interval = (double)emitter->lifetime / emitter->amount;
    double elapsed = emitter->spawnTime + deltaTime;
    double due = floor(elapsed / interval);
    uint spawnCount = due >= emitter->amount ? emitter->amount : (uint)due;
    emitter->spawnTime = isfinite(due) ? fmax(0, elapsed - due * interval) : fmod(elapsed, interval);
    for (uint i = 0; i < emitter->amount && spawnCount > 0; i++) {
        mParticle* particle = &emitter->particles[i];
        if (particle->alive) continue;
        float angle = (float)((rand_ui64() >> 40) / 16777216.0 * 6.283185307179586);
        *particle = (mParticle){.x = emitter->x, .y = emitter->y, .lifetime = emitter->lifetime,
            .velocityX = cosf(angle) * emitter->speed, .velocityY = sinf(angle) * emitter->speed,
            .size = emitter->startSize, .color = emitter->startColor, .alive = M_TRUE};
        advanceParticle(emitter, particle, emitter->spawnTime + (spawnCount - 1) * interval);
        if (particle->alive) emitter->count++;
        spawnCount--;
    }
}

int mDrawParticles(mContext* ctx, const mParticleEmitter* emitter) {
    if (ctx == NULL || ctx->renderer == NULL || !validEmitter(emitter)) return M_FAILURE;
    const unsigned int indices[] = {0, 1, 2, 2, 3, 0};
    for (uint i = 0; i < emitter->amount; i++) {
        const mParticle* particle = &emitter->particles[i];
        if (!particle->alive || particle->size <= 0) continue;
        float x = floorf(particle->x - particle->size * .5f);
        float y = floorf(particle->y - particle->size * .5f);
        float size = fmaxf(1, roundf(particle->size));
        mVertex vertices[] = {
            {.x = x, .y = y, .color = particle->color},
            {.x = x + size, .y = y, .color = particle->color},
            {.x = x + size, .y = y + size, .color = particle->color},
            {.x = x, .y = y + size, .color = particle->color}
        };
        if (mAddVerticesIndexed(ctx, vertices, 4, indices, 6, NULL, NULL) == M_FAILURE) return M_FAILURE;
    }
    return M_SUCCESS;
}

void mParticleEmitterDestroy(mParticleEmitter* emitter) {
    if (emitter == NULL) return;
    free(emitter->particles);
    *emitter = (mParticleEmitter){0};
}
