#include "nevs.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "int/intlib.h"
#include "int/memdbg.h"

static_assert(sizeof(Nevs) == 60, "wrong size");

// 0x6391C8
Nevs* gNevs;

// 0x6391CC
int gNevsHits;

// nevs_alloc
// 0x488340
Nevs* _nevs_alloc()
{
    if (gNevs == NULL) {
        debugPrint("nevs_alloc(): nevs_initonce() not called!");
        exit(99);
    }

    for (int index = 0; index < NEVS_COUNT; index++) {
        Nevs* nevs = &(gNevs[index]);
        if (!nevs->used) {
            // NOTE: Uninline.
            _nevs_reset(nevs);
            return nevs;
        }
    }

    return NULL;
}

// NOTE: Inlined.
//
// 0x488394
void _nevs_reset(Nevs* nevs)
{
    nevs->used = false;
    memset(nevs, 0, sizeof(*nevs));
}

// 0x4883AC
void _nevs_close()
{
    if (gNevs != NULL) {
        myfree(gNevs, __FILE__, __LINE__); // "..\\int\\NEVS.C", 97
        gNevs = NULL;
    }
}

// 0x4883D4
void _nevs_removeprogramreferences(Program* program)
{
    if (gNevs != NULL) {
        for (int i = 0; i < NEVS_COUNT; i++) {
            Nevs* nevs = &(gNevs[i]);
            if (nevs->used && nevs->program == program) {
                // NOTE: Uninline.
                _nevs_reset(nevs);
            }
        }
    }
}

// nevs_initonce
// 0x488418
void _nevs_initonce()
{
    interpretRegisterProgramDeleteCallback(_nevs_removeprogramreferences);

    if (gNevs == NULL) {
        gNevs = (Nevs*)mycalloc(sizeof(Nevs), NEVS_COUNT, __FILE__, __LINE__); // "..\\int\\NEVS.C", 131
        if (gNevs == NULL) {
            debugPrint("nevs_initonce(): out of memory");
            exit(99);
        }
    }
}

// nevs_find
// 0x48846C
Nevs* _nevs_find(const char* name)
{
    if (gNevs == NULL) {
        debugPrint("nevs_find(): nevs_initonce() not called!");
        exit(99);
    }

    for (int index = 0; index < NEVS_COUNT; index++) {
        Nevs* nevs = &(gNevs[index]);
        if (nevs->used && stricmp(nevs->name, name) == 0) {
            return nevs;
        }
    }

    return NULL;
}

// 0x4884C8
int _nevs_addevent(const char* name, Program* program, int proc, int type)
{
    Nevs* nevs = _nevs_find(name);
    if (nevs == NULL) {
        nevs = _nevs_alloc();
    }

    if (nevs == NULL) {
        return 1;
    }

    nevs->used = true;
    strcpy(nevs->name, name);
    nevs->program = program;
    nevs->proc = proc;
    nevs->type = type;
    nevs->field_38 = NULL;

    return 0;
}

// nevs_clearevent
// 0x48859C
int _nevs_clearevent(const char* a1)
{
    debugPrint("nevs_clearevent( '%s');\n", a1);

    Nevs* nevs = _nevs_find(a1);
    if (nevs != NULL) {
        // NOTE: Uninline.
        _nevs_reset(nevs);
        return 0;
    }

    return 1;
}

// nevs_signal
// 0x48862C
int _nevs_signal(const char* name)
{
    debugPrint("nevs_signal( '%s');\n", name);

    Nevs* nevs = _nevs_find(name);
    if (nevs == NULL) {
        return 1;
    }

    debugPrint("nep: %p,  used = %u, prog = %p, proc = %d", nevs, nevs->used, nevs->program, nevs->proc);

    if (nevs->used
        && ((nevs->program != NULL && nevs->proc != 0) || nevs->field_38 != NULL)
        && !nevs->busy) {
        nevs->hits++;
        gNevsHits++;
        return 0;
    }

    return 1;
}

// nevs_update
// 0x4886AC
void _nevs_update()
{
    if (gNevsHits == 0) {
        return;
    }

    debugPrint("nevs_update(): we have anyhits = %u\n", gNevsHits);

    gNevsHits = 0;

    for (int index = 0; index < NEVS_COUNT; index++) {
        Nevs* nevs = &(gNevs[index]);
        if (nevs->used
            && ((nevs->program != NULL && nevs->proc != 0) || nevs->field_38 != NULL)
            && !nevs->busy) {
            if (nevs->hits > 0) {
                nevs->busy = true;

                nevs->hits -= 1;
                gNevsHits += nevs->hits;

                if (nevs->field_38 == NULL) {
                    executeProc(nevs->program, nevs->proc);
                } else {
                    nevs->field_38();
                }

                nevs->busy = false;

                if (nevs->type == NEVS_TYPE_EVENT) {
                    // NOTE: Uninline.
                    _nevs_reset(nevs);
                }
            }
        }
    }
}
