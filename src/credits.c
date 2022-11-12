#include "credits.h"

#include <string.h>

#include "game/art.h"
#include "color.h"
#include "core.h"
#include "cycle.h"
#include "debug.h"
#include "draw.h"
#include "game_mouse.h"
#include "memory.h"
#include "message.h"
#include "palette.h"
#include "sound.h"
#include "text_font.h"
#include "window_manager.h"

// 0x56D740
File* gCreditsFile;

// 0x56D744
int gCreditsWindowNameColor;

// 0x56D748
int gCreditsWindowTitleFont;

// 0x56D74C
int gCreditsWindowNameFont;

// 0x56D750
int gCreditsWindowTitleColor;

// 0x42C860
void creditsOpen(const char* filePath, int backgroundFid, bool useReversedStyle)
{
    int oldFont = fontGetCurrent();

    loadColorTable("color.pal");

    if (useReversedStyle) {
        gCreditsWindowTitleColor = colorTable[18917];
        gCreditsWindowNameFont = 103;
        gCreditsWindowTitleFont = 104;
        gCreditsWindowNameColor = colorTable[13673];
    } else {
        gCreditsWindowTitleColor = colorTable[13673];
        gCreditsWindowNameFont = 104;
        gCreditsWindowTitleFont = 103;
        gCreditsWindowNameColor = colorTable[18917];
    }

    soundContinueAll();

    char localizedPath[MAX_PATH];
    if (_message_make_path(localizedPath, filePath)) {
        gCreditsFile = fileOpen(localizedPath, "rt");
        if (gCreditsFile != NULL) {
            soundContinueAll();

            colorCycleDisable();
            gameMouseSetCursor(MOUSE_CURSOR_NONE);

            bool cursorWasHidden = mouse_hidden();
            if (cursorWasHidden) {
                mouse_show();
            }

            int creditsWindowX = 0;
            int creditsWindowY = 0;
            int window = windowCreate(creditsWindowX, creditsWindowY, CREDITS_WINDOW_WIDTH, CREDITS_WINDOW_HEIGHT, colorTable[0], 20);
            soundContinueAll();
            if (window != -1) {
                unsigned char* windowBuffer = windowGetBuffer(window);
                if (windowBuffer != NULL) {
                    unsigned char* backgroundBuffer = (unsigned char*)internal_malloc(CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT);
                    if (backgroundBuffer) {
                        soundContinueAll();

                        memset(backgroundBuffer, colorTable[0], CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT);

                        if (backgroundFid != -1) {
                            CacheEntry* backgroundFrmHandle;
                            Art* frm = art_ptr_lock(backgroundFid, &backgroundFrmHandle);
                            if (frm != NULL) {
                                int width = art_frame_width(frm, 0, 0);
                                int height = art_frame_length(frm, 0, 0);
                                unsigned char* backgroundFrmData = art_frame_data(frm, 0, 0);
                                blitBufferToBuffer(backgroundFrmData,
                                    width,
                                    height,
                                    width,
                                    backgroundBuffer + CREDITS_WINDOW_WIDTH * ((CREDITS_WINDOW_HEIGHT - height) / 2) + (CREDITS_WINDOW_WIDTH - width) / 2,
                                    CREDITS_WINDOW_WIDTH);
                                art_ptr_unlock(backgroundFrmHandle);
                            }
                        }

                        unsigned char* intermediateBuffer = (unsigned char*)internal_malloc(CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT);
                        if (intermediateBuffer != NULL) {
                            memset(intermediateBuffer, 0, CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT);

                            fontSetCurrent(gCreditsWindowTitleFont);
                            int titleFontLineHeight = fontGetLineHeight();

                            fontSetCurrent(gCreditsWindowNameFont);
                            int nameFontLineHeight = fontGetLineHeight();

                            int lineHeight = nameFontLineHeight + (titleFontLineHeight >= nameFontLineHeight ? titleFontLineHeight - nameFontLineHeight : 0);
                            int stringBufferSize = CREDITS_WINDOW_WIDTH * lineHeight;
                            unsigned char* stringBuffer = (unsigned char*)internal_malloc(stringBufferSize);
                            if (stringBuffer != NULL) {
                                blitBufferToBuffer(backgroundBuffer,
                                    CREDITS_WINDOW_WIDTH,
                                    CREDITS_WINDOW_HEIGHT,
                                    CREDITS_WINDOW_WIDTH,
                                    windowBuffer,
                                    CREDITS_WINDOW_WIDTH);

                                win_draw(window);

                                paletteFadeTo(cmap);

                                unsigned char* v40 = intermediateBuffer + CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT - CREDITS_WINDOW_WIDTH;
                                char str[260];
                                int font;
                                int color;
                                unsigned int tick = 0;
                                bool stop = false;
                                while (creditsFileParseNextLine(str, &font, &color)) {
                                    fontSetCurrent(font);

                                    int v19 = fontGetStringWidth(str);
                                    if (v19 >= CREDITS_WINDOW_WIDTH) {
                                        continue;
                                    }

                                    memset(stringBuffer, 0, stringBufferSize);
                                    fontDrawText(stringBuffer, str, CREDITS_WINDOW_WIDTH, CREDITS_WINDOW_WIDTH, color);

                                    unsigned char* dest = intermediateBuffer + CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT - CREDITS_WINDOW_WIDTH + (CREDITS_WINDOW_WIDTH - v19) / 2;
                                    unsigned char* src = stringBuffer;
                                    for (int index = 0; index < lineHeight; index++) {
                                        if (_get_input() != -1) {
                                            stop = true;
                                            break;
                                        }

                                        memmove(intermediateBuffer, intermediateBuffer + CREDITS_WINDOW_WIDTH, CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT - CREDITS_WINDOW_WIDTH);
                                        memcpy(dest, src, v19);

                                        blitBufferToBuffer(backgroundBuffer,
                                            CREDITS_WINDOW_WIDTH,
                                            CREDITS_WINDOW_HEIGHT,
                                            CREDITS_WINDOW_WIDTH,
                                            windowBuffer,
                                            CREDITS_WINDOW_WIDTH);

                                        blitBufferToBufferTrans(intermediateBuffer,
                                            CREDITS_WINDOW_WIDTH,
                                            CREDITS_WINDOW_HEIGHT,
                                            CREDITS_WINDOW_WIDTH,
                                            windowBuffer,
                                            CREDITS_WINDOW_WIDTH);

                                        while (getTicksSince(tick) < CREDITS_WINDOW_SCROLLING_DELAY) {
                                        }

                                        tick = _get_time();

                                        win_draw(window);

                                        src += CREDITS_WINDOW_WIDTH;
                                    }

                                    if (stop) {
                                        break;
                                    }
                                }

                                if (!stop) {
                                    for (int index = 0; index < CREDITS_WINDOW_HEIGHT; index++) {
                                        if (_get_input() != -1) {
                                            break;
                                        }

                                        memmove(intermediateBuffer, intermediateBuffer + CREDITS_WINDOW_WIDTH, CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT - CREDITS_WINDOW_WIDTH);
                                        memset(intermediateBuffer + CREDITS_WINDOW_WIDTH * CREDITS_WINDOW_HEIGHT - CREDITS_WINDOW_WIDTH, 0, CREDITS_WINDOW_WIDTH);

                                        blitBufferToBuffer(backgroundBuffer,
                                            CREDITS_WINDOW_WIDTH,
                                            CREDITS_WINDOW_HEIGHT,
                                            CREDITS_WINDOW_WIDTH,
                                            windowBuffer,
                                            CREDITS_WINDOW_WIDTH);

                                        blitBufferToBufferTrans(intermediateBuffer,
                                            CREDITS_WINDOW_WIDTH,
                                            CREDITS_WINDOW_HEIGHT,
                                            CREDITS_WINDOW_WIDTH,
                                            windowBuffer,
                                            CREDITS_WINDOW_WIDTH);

                                        while (getTicksSince(tick) < CREDITS_WINDOW_SCROLLING_DELAY) {
                                        }

                                        tick = _get_time();

                                        win_draw(window);
                                    }
                                }

                                internal_free(stringBuffer);
                            }
                            internal_free(intermediateBuffer);
                        }
                        internal_free(backgroundBuffer);
                    }
                }

                soundContinueAll();
                paletteFadeTo(gPaletteBlack);
                soundContinueAll();
                windowDestroy(window);
            }

            if (cursorWasHidden) {
                mouse_hide();
            }

            gameMouseSetCursor(MOUSE_CURSOR_ARROW);
            colorCycleEnable();
            fileClose(gCreditsFile);
        }
    }

    fontSetCurrent(oldFont);
}

// 0x42CE6C
bool creditsFileParseNextLine(char* dest, int* font, int* color)
{
    char string[256];
    while (fileReadString(string, 256, gCreditsFile)) {
        char* pch;
        if (string[0] == ';') {
            continue;
        } else if (string[0] == '@') {
            *font = gCreditsWindowTitleFont;
            *color = gCreditsWindowTitleColor;
            pch = string + 1;
        } else if (string[0] == '#') {
            *font = gCreditsWindowNameFont;
            *color = colorTable[17969];
            pch = string + 1;
        } else {
            *font = gCreditsWindowNameFont;
            *color = gCreditsWindowNameColor;
            pch = string;
        }

        strcpy(dest, pch);

        return true;
    }

    return false;
}
