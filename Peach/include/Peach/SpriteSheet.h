//
// Created by Štěpán Toman on 29.09.2026.
//

#ifndef PEACH_SPRITESHEET_H
#define PEACH_SPRITESHEET_H

#include <Peach/Common.h>

// Frames run left to right, then top to bottom. spritesAmount 0 uses the whole grid.
// Returns an empty sheet on failure. Register with mAddSpriteSheet for automatic cleanup.
mSpriteSheet mCreateSpriteSheet(mSpriteSheetInfo info, const char* filePath);

// For unregistered sheets; use mRemoveResource for registered sheets.
void mSpriteSheetDestroy(mContext* ctx, mSpriteSheet* sheet);

#endif //PEACH_SPRITESHEET_H
