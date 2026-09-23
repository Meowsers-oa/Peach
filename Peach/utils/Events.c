//
// Created by Štěpán Toman on 23.09.2026.
//

#include "Events.h"
#include <stdlib.h>

#define MAX_LISTENERS 256

typedef struct {
    uint32_t type;
    mEventCallback callback;
    void* userData;
} ListenerSlot;

static ListenerSlot g_listeners[MAX_LISTENERS];
static size_t g_listenerCount = 0;

void mListenToEvent(uint32_t type, mEventCallback callback, void* userData) {
    if (g_listenerCount >= MAX_LISTENERS) return;

    g_listeners[g_listenerCount++] = (ListenerSlot){
        .type = type,
        .callback = callback,
        .userData = userData
    };
}

void mDispatchEvent(mEvent event) {
    for (size_t i = 0; i < g_listenerCount; i++) {
        if (g_listeners[i].type == event.type) {
            g_listeners[i].callback(&event, g_listeners[i].userData);
            if (event.handled) break;
        }
    }
}