#ifndef PHASHTABLEWEA_INCLUDE
#define PHASHTABLEWEA_INCLUDE

#include "p:\stdwea\gdefwea.h"
#include "p:\stdwea\iowea.h"
#include "p:\stdwea\dynbuffwea.h"

typedef struct PHTENTRY PHTENTRY;
struct PHTENTRY {
	u8 hash;
	u4 off;
	u2 state;
	u2 padd0;
};

typedef struct PHASHTABLE PHASHTABLE;
struct PHASHTABLE {
	DYNBUFF* pool;
	DYNBUFF* entries;
	u8 (*Hash)(u1*, u4);
	u1 (*Cmp)(PHASHTABLE*, PHTENTRY*, u1*, u4);
	u4 cap;
	u4 occ;
};

PHASHTABLE* PHT_Create(u4 elementtypesize, u8(*HashFunc)(u1*, u4), u1(*CmpFunc)(PHASHTABLE*, PHTENTRY*, u1*, u4));

void PHT_Free(PHASHTABLE* table);

u8 PHT_Hash(void* key, u4 keysize);

PHTENTRY* PHT_GetEntry(PHASHTABLE* table, void* key, u4 keysize);

u4 PHT_Index(PHASHTABLE* table, void* key, u4 keysize);

void PHT_Grow(PHASHTABLE* table);

u4 PHT_Ensure(PHASHTABLE* table, void* key, u4 keysize, void* obj, u4 objsize);

u4 PHT_Insert(PHASHTABLE* table, void* key, u4 keysize, void* obj, u4 objsize);

PHTENTRY* PHT_Reserve(PHASHTABLE* table, void* key, u4 keysize, u4 objsize);

#endif// HASHTABLEWEA_INCLUDE

#ifndef  PHASHTABLEWEA_DEF
#define  PHASHTABLEWEA_DEF

PHASHTABLE* PHT_Create(u4 elementtypesize, u8(*HashFunc)(u1*, u4), u1(*CmpFunc)(PHASHTABLE*, PHTENTRY*, u1*, u4)) {
	PHASHTABLE* tmp = (PHASHTABLE*)Malloc(sizeof(PHASHTABLE));
	tmp->cap = 256;
	tmp->occ = 0;
	tmp->entries = DB_Create(tmp->cap, sizeof(PHTENTRY));
	memset(tmp->entries->data, 0, tmp->entries->cap);
	tmp->pool = DB_Create(tmp->cap, elementtypesize);
	memset(tmp->pool->data, 0, tmp->pool->cap);
	tmp->Hash = HashFunc;
	tmp->Cmp = CmpFunc;
	return tmp;
}

void PHT_Free(PHASHTABLE* table) {
	DB_Free(table->entries);
	DB_Free(table->pool);
	Free(table);
}

u8 PHT_Hash(void* key, u4 keysize) {
	u8 hashedvalue = 0xCABA1106A1042069;
	for (u4 i = keysize & 0xFFFFFFF8; i; i -= 8) {
		hashedvalue ^= *(u8*)key ^ (hashedvalue << (i & 0x3F));
		hashedvalue *= 0xCABA1106A1042069 ^ (hashedvalue >> 29);
		(u1*)key += 8;
	}
	for (u4 i = keysize & 0x07; i; i--) {
		hashedvalue ^= *(u1*)key ^ (hashedvalue >> i);
		hashedvalue *= 0xCABA1106A1042069 ^ (hashedvalue >> 19);
		((u1*)key)++;
	}
	hashedvalue ^= hashedvalue >> 17;
	hashedvalue *= 0xCABA1106A1042069;
	hashedvalue ^= hashedvalue >> 29;
	hashedvalue *= 0xCABA1106A1042069;
	return hashedvalue;
}

PHTENTRY* PHT_GetEntry(PHASHTABLE* table, void* key, u4 keysize) {
	u8 hashedvalue = table->Hash(key, keysize);
	u4 idx = hashedvalue & (table->cap - 1);
	PHTENTRY* entry = (PHTENTRY*)DB_Index(table->entries, idx);

	if (entry->state) {
		PHTENTRY* start = entry;
		do {
			if (entry->hash == hashedvalue && table->Cmp(table, entry, key, keysize)) return entry;
			idx = (idx + 1) & (table->cap - 1);
			entry = (PHTENTRY*)DB_Index(table->entries, idx);
		} while (entry != start && entry->state);
		if (entry == start) {
			FatalError(0, "PHT_GetEntry::something went wrong, buy more ram\n");
		}
	}
	entry->hash = hashedvalue;
	return entry;
}

u4 PHT_Index(PHASHTABLE* table, void* key, u4 keysize) {
	PHTENTRY* entry = PHT_GetEntry(table, key, keysize);
	if (entry->state) { return entry->off; }
	return (u4)(-1);
}

void PHT_Grow(PHASHTABLE* table) {
	DYNBUFF tmp = *table->entries;
	table->cap <<= 1;
	table->entries->cap <<= 1;
	table->entries->data = Malloc(table->entries->cap);
	memset(table->entries->data, 0, table->entries->cap);

	u8 symtablesize = DB_Size(&tmp);
	PHTENTRY* entry = (PHTENTRY*)DB_Index(&tmp, 0);
	for (u4 i = 0; i < symtablesize; i++) {
		if (entry->state) {
			u4 idx = entry->hash & (table->cap - 1);
			PHTENTRY* nxtentry = (PHTENTRY*)DB_Index(table->entries, idx);
			while (nxtentry->state) {
				idx = (idx + 1) & (table->cap - 1);
				nxtentry = (PHTENTRY*)DB_Index(table->entries, idx);
			}
			memcpy(nxtentry, entry, sizeof(PHTENTRY));
		}
		entry++;
	}
	Free(tmp.data);
}

u4 PHT_Ensure(PHASHTABLE* table, void* key, u4 keysize, void* obj, u4 objsize) {
	if (table->cap - (table->cap >> 2) < table->occ + 1) { PHT_Grow(table); }
	PHTENTRY* entry = PHT_GetEntry(table, key, keysize);
	if (entry->state) { return entry->off; }
	entry->state = 1;
	entry->off = DB_ElementCount(table->pool);
	DB_Append(table->pool, obj, objsize);
	table->entries->occ += sizeof(PHTENTRY);
	table->occ++;
	return entry->off;
}

u4 PHT_Insert(PHASHTABLE* table, void* key, u4 keysize, void* obj, u4 objsize) {
	if (table->cap - (table->cap >> 2) < table->occ + 1) { PHT_Grow(table); }
	PHTENTRY* entry = PHT_GetEntry(table, key, keysize);
	if (entry->state) {
		memcpy(DB_Index(table->pool, entry->off), obj, objsize);
		return entry->off;
	}
	entry->state = 1;
	entry->off = DB_ElementCount(table->pool);
	DB_Append(table->pool, obj, objsize);
	table->entries->occ += sizeof(PHTENTRY);
	table->occ++;
	return entry->off;
}

PHTENTRY* PHT_Reserve(PHASHTABLE* table, void* key, u4 keysize, u4 objsize) {
	if (table->cap - (table->cap >> 2) < table->occ + 1) { PHT_Grow(table); }
	PHTENTRY* entry = PHT_GetEntry(table, key, keysize);
	if (entry->state) return entry;
	entry->off = table->pool->occ;
	DB_Reserve(table->pool, objsize);
	table->entries->occ += sizeof(PHTENTRY);
	table->occ++;
	return entry;
}

#endif// PHASHTABLEWEA_DEF