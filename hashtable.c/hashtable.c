/**
 * Hashtable implementation
 * (c) 2011-2019 @marekweb
 *
 * Uses dynamic addressing with linear probing.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "hashtable.h"

/*
 * Interface section used for `makeheaders`.
 */
#if INTERFACE
struct hashtable_entry {
	char* key;
	void* value;
};

struct hashtable {
	unsigned int size;
	unsigned int capacity;
	hashtable_entry* body;
};
#endif

#define HASHTABLE_INITIAL_CAPACITY 4

/**
 * Compute the hash value for the given string.
 * Implements the djb k=33 hash function.
 */
unsigned long hashtable_hash(char* str)
{
	unsigned long hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;  /* hash * 33 + c */
	return hash;
}

/**
 * Find an available slot for the given key, using linear probing.
 */

int hashtable_strcmp_wrap(char* s1, char* s2){
	if(s1 == (char*)0xDEAD|| s2 == (char*)0xDEAD) return 1;
	return strcmp(s1,s2);
}

unsigned int hashtable_find_slot(hashtable* t, char* key)
{
	int index = hashtable_hash(key) % t->capacity;
	while (t->body[index].key != NULL && hashtable_strcmp_wrap(t->body[index].key,key) != 0) {
		index = (index + 1) % t->capacity;
	}
	return index;
}

/**
 * Return the item associated with the given key, or NULL if not found.
 */
void* hashtable_get(hashtable* t, char* key)
{
	int index = hashtable_find_slot(t, key);
	if (t->body[index].key != NULL && t->body[index].key != (char*)0xDEAD) {
		return t->body[index].value;
	} else {
		return NULL;
	}
}

/**
 * Assign a value to the given key in the table.
 */
void hashtable_set(hashtable* t, char* key, void* value)
{
	int index = hashtable_find_slot(t, key);
	if (t->body[index].key != NULL && t->body[index].key != (char*)0xDEAD) {
		/* Entry exists; update it. */
		t->body[index].value = value;
	} else {
		t->size++;
		/* Create a new  entry */
		if ((float)t->size / t->capacity > 0.8) {
			/* Resize the hash table */
			hashtable_resize(t, t->capacity * 2);
			index = hashtable_find_slot(t, key);
		}
		t->body[index].key = key;
		t->body[index].value = value;
	}
}

/**
 * Remove a key from the table
 */
void hashtable_remove(hashtable* t, char* key)
{
	int index = hashtable_find_slot(t, key);
	if (t->body[index].key != NULL) {
		t->body[index].key = (char*)0xDEAD;
		t->body[index].value = NULL;
		t->size--;
	}
}

/**
 * Create a new, empty hashtable
 */
hashtable* hashtable_create()
{
	hashtable* new_ht = malloc(sizeof(hashtable));
	new_ht->size = 0;
	new_ht->capacity = HASHTABLE_INITIAL_CAPACITY;
	new_ht->body = hashtable_body_allocate(new_ht->capacity);
	return new_ht;
}

/**
 * Allocate a new memory block with the given capacity.
 */
hashtable_entry* hashtable_body_allocate(unsigned int capacity)
{
	// calloc fills the allocated memory with zeroes
	return (hashtable_entry*)calloc(capacity, sizeof(hashtable_entry));
}

/**
 * Resize the allocated memory.
 */
void hashtable_resize(hashtable* t, unsigned int capacity)
{
	assert(capacity >= t->size);
	unsigned int old_capacity = t->capacity;
	hashtable_entry* old_body = t->body;
	t->body = hashtable_body_allocate(capacity);
	t->capacity = capacity;

	// Copy all the old values into the newly allocated body
	for (int i = 0; i < old_capacity; i++) {
		if (old_body[i].key != NULL && old_body[i].key != (char*)0xDEAD) {
			hashtable_set(t, old_body[i].key, old_body[i].value);
		}
	}
	free(old_body);
}

/**
 * Destroy the table and deallocate it from memory. This does not deallocate the contained items.
 */
void hashtable_destroy(hashtable* t)
{
	free(t->body);
	free(t);
}

uint64_t murmur(char* str,uint32_t keylen){
	uint64_t h = (525201411107845655ull);
	for (int i =0; i < keylen; i++,str++){
		h ^= *str;
		h *= 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}
