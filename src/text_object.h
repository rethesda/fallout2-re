#ifndef TEXT_OBJECT_H
#define TEXT_OBJECT_H

#include <stdbool.h>

#include "geometry.h"
#include "game/object_types.h"

// The maximum number of text objects that can exist at the same time.
#define TEXT_OBJECTS_MAX_COUNT (20)

typedef enum TextObjectFlags {
    TEXT_OBJECT_MARKED_FOR_REMOVAL = 0x01,
    TEXT_OBJECT_UNBOUNDED = 0x02,
} TextObjectFlags;

typedef struct TextObject {
    int flags;
    Object* owner;
    unsigned int time;
    int linesCount;
    int sx;
    int sy;
    int tile;
    int x;
    int y;
    int width;
    int height;
    unsigned char* data;
} TextObject;

extern int gTextObjectsCount;
extern unsigned int gTextObjectsBaseDelay;
extern unsigned int gTextObjectsLineDelay;

extern TextObject* gTextObjects[TEXT_OBJECTS_MAX_COUNT];
extern int gTextObjectsWindowWidth;
extern int gTextObjectsWindowHeight;
extern unsigned char* gTextObjectsWindowBuffer;
extern bool gTextObjectsEnabled;
extern bool gTextObjectsInitialized;

int textObjectsInit(unsigned char* windowBuffer, int width, int height);
int textObjectsReset();
void textObjectsFree();
void textObjectsDisable();
void textObjectsEnable();
int text_object_is_enabled();
void textObjectsSetBaseDelay(double value);
unsigned int text_object_get_base_delay();
void textObjectsSetLineDelay(double value);
unsigned int text_object_get_line_delay();
int textObjectAdd(Object* object, char* string, int font, int color, int a5, Rect* rect);
void textObjectsRenderInRect(Rect* rect);
int textObjectsGetCount();
void textObjectsTicker();
void textObjectFindPlacement(TextObject* textObject);
void textObjectsRemoveByOwner(Object* object);

#endif /* TEXT_OBJECT_H */
