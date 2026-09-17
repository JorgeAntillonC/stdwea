#ifndef DYNBUFFWEA_INCLUDE
#define DYNBUFFWEA_INCLUDE

#include "p:\stdwea\gdefwea.h"
#include "p:\stdwea\iowea.h"

typedef struct DYNBUFF DYNBUFF;
struct DYNBUFF {
	u4 occ;
	u4 cap;
	u4 tsize;
	u1* data;
};

DYNBUFF* DB_Create(u4 size, u4 typesize);

void DB_Free(DYNBUFF* db);

void DB_Grow(DYNBUFF* db);

void DB_Append(DYNBUFF* db, void* source, u4 sources);

void DB_Push(DYNBUFF* db, void* source);

u1 DB_Pop(DYNBUFF* db, void* dest);

static inline void* DB_SoftPop(DYNBUFF* db);

static inline void DB_Drop(DYNBUFF* db, u4 num);

static inline void* DB_Index(DYNBUFF* db, u4 i);

static inline void* DB_Peek(DYNBUFF* db, u4 i);

static inline u4 DB_ElementCount(DYNBUFF* db);

static inline u4 DB_Size(DYNBUFF* db);


void DB_Extend(DYNBUFF* dest, DYNBUFF* source);

void* DB_Reserve(DYNBUFF* db, u4 sources);

#endif// DYNBUFFWEA_INCLUDE

#ifndef  DYNBUFFWEA_DEF
#define  DYNBUFFWEA_DEF

DYNBUFF* DB_Create(u4 size, u4 typesize) {
	DYNBUFF* tmp = (DYNBUFF*)Malloc(sizeof(DYNBUFF));
	tmp->data = Malloc(size * typesize);
	tmp->cap = size * typesize;
	tmp->occ = 0;
	tmp->tsize = typesize;
	return tmp;
}

void DB_Free(DYNBUFF* db) {
	Free(db->data);
	Free(db);
}

void DB_Grow(DYNBUFF* db) {
	u4 ncap = db->cap << 1;
	if (ncap < db->cap) { FatalError(420, "Internal error: DB_GROW() underflow::buy a 64 bit pc bruh\n"); }
	u1* tmp = Malloc(ncap);
	memcpy(tmp, db->data, db->occ);
	Free(db->data);
	db->data = tmp;
	db->cap = ncap;
}

void DB_Append(DYNBUFF* db, void* source, u4 sources) {
	while (db->cap <= db->occ + sources) { DB_Grow(db); }
	memcpy(db->data + db->occ, source, sources);
	db->occ += sources;
}

void* DB_Reserve(DYNBUFF* db, u4 sources) {
	while (db->cap <= db->occ + sources) { DB_Grow(db); }
	u4 oldocc = db->occ;
	db->occ += sources;
	return oldocc + db->data;
}

void DB_Push(DYNBUFF* db, void* source) {
	while (db->cap <= db->occ + db->tsize) { DB_Grow(db); }
	memcpy(db->data + db->occ, source, db->tsize);
	db->occ += db->tsize;
}

u1 DB_Pop(DYNBUFF* db, void* dest) {
	if (db->occ < db->tsize) { return 0; }
	db->occ -= db->tsize;
	if (dest) { memcpy(dest, db->data + db->occ, db->tsize); }
	return 1;
}

static inline void DB_Drop(DYNBUFF* db, u4 num) {
	if (db->occ < db->tsize) { return; }
	db->occ -= num*db->tsize;
}

static inline void* DB_Index(DYNBUFF* db, u4 i) {
	u4 index = i * db->tsize;
	if (db->cap <= index) { return 0; }
	return (void*)(db->data + index);
}

//gives pointer to the last object pushed onto the stack and preserves it
static inline void* DB_Peek(DYNBUFF* db, u4 i) {
	u4 index = i * db->tsize;
	if (!index || db->occ < index) { return 0; }
	return (void*)(db->data + db->occ - index);
}

static inline void* DB_SoftPop(DYNBUFF* db) {
	if (db->occ < db->tsize) { return 0; }
	db->occ -= db->tsize;
	return (void*)(db->data + db->occ);
}

static inline u4 DB_ElementCount(DYNBUFF* db) {
	return db->occ / db->tsize;
}

static inline u4 DB_Size(DYNBUFF* db) {
	return db->cap / db->tsize;
}

void DB_Extend(DYNBUFF* dest, DYNBUFF* source) {
	while (dest->cap <= dest->occ + source->occ) { DB_Grow(dest); }
	memcpy(dest->data + dest->occ, source->data, source->occ);
	dest->occ += source->occ;
}

#endif//DYNBUFFWEA_DEF