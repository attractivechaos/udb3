#pragma once

#ifndef EE_DICT_H
#define EE_DICT_H

#include "ee_core.h"

static const u64 EE_ZERO_U64 = 0;
static const u64 EE_ONE_U64  = 1;
static const u64 EE_MAX_U64  = 0xffffffffffffffff;

static const f64 EE_ZERO_F64 = 0.0;
static const f64 EE_ONE_F64  = 1.0;

#ifndef EE_DICT_START_SIZE
#define EE_DICT_START_SIZE           (32)
#endif

#define EE_GROUP_SIZE                (EED_SIMD_BYTES)

#define EE_SLOT_EMPTY                (0x80)
#define EE_SLOT_DELETED              (0xFE)
#define EE_GROUP_MASK                (~(EE_GROUP_SIZE - 1))

#define EE_CONST_ZERO                (EE_RECAST_U8(EE_ZERO_U64))
#define EE_CONST_ONE                 (EE_RECAST_U8(EE_ONE_U64))
#define EE_CONST_MAX_U64             (EE_RECAST_U8(EE_MAX_U64))
#define EE_CONST_ZERO_F64            (EE_RECAST_U8(EE_ZERO_F64))
#define EE_CONST_ONE_F64             (EE_RECAST_U8(EE_ONE_F64))

// #define EE_DICT_TOMBS_REHASH
typedef u64  (*DictHash)(const u8* key, size_t len);
typedef i32  (*DictEq)(const u8* a, const u8* b, size_t len);
typedef void (*DictCpy)(u8* dest, const u8* src, size_t len);

#define ee_dict_new_conf_m(size, key_type, val_type, config)     ee_dict_new(size, sizeof(key_type), sizeof(val_type), config)
#define ee_dict_new_m(size, key_type, val_type, \
	allocator, hash_fn, eq_fn, key_cpy_fn, val_cpy_fn)           ee_dict_new(size, sizeof(key_type), sizeof(val_type), (DictConfig) { allocator, hash_fn, eq_fn, key_cpy_fn, val_cpy_fn })
#define ee_dict_def_m(size, key_type, val_type)                  ee_dict_new(size, sizeof(key_type), sizeof(val_type), (DictConfig){ 0 })

typedef struct AlignedBuffer
{
	size_t size;
	u8*    buffer;
	void*  base;
} AlignedBuffer;

EE_INLINE AlignedBuffer ee_aligned_alloc(size_t size, size_t align, Allocator* allocator)
{
	EE_ASSERT(ee_is_pow2(align), "Alignment should be power of two");

	AlignedBuffer out = { 0 };

	out.size = size;
	
	if (allocator == NULL)
		out.base = malloc(out.size + align - 1);
	else
		out.base = allocator->alloc_fn(allocator, size);

	EE_ASSERT(out.base != NULL, "Unable to allocate (%zu) bytes for aligned alloc", out.size + align - 1);

	uintptr_t base    = (uintptr_t)out.base;
	uintptr_t aligned = (base + (align - 1)) & ~(uintptr_t)(align - 1);

	out.buffer = (u8*)aligned;

	return out;
}

EE_INLINE void ee_aligned_free(AlignedBuffer* buffer, Allocator* allocator)
{
	EE_ASSERT(buffer->base != NULL, "Trying to free invalid aligned buffer, AlignedBuffer.base is NULL");
	EE_ASSERT(buffer->buffer != NULL, "Trying to free invalid aligned buffer, AlignedBuffer.buffer is NULL");

	if (allocator == NULL)
		free(buffer->base);
	else
		allocator->free_fn(allocator, buffer->base);

	memset(buffer, 0, sizeof(*buffer));
}

typedef struct DictConfig
{
	Allocator* allocator;
	DictHash   hash_fn;
	DictEq     eq_fn;
	DictCpy    key_cpy_fn;
	DictCpy    val_cpy_fn;
} DictConfig;

typedef struct Dict
{
	AlignedBuffer keys;
	AlignedBuffer vals;
	AlignedBuffer ctrl;

	size_t count;
	size_t cap;
	size_t mask;
	size_t th;

	size_t key_len;
	size_t val_len;

#ifdef EE_DICT_TOMBS_REHASH
	size_t tombs;
	size_t tombs_th;
#endif

	Allocator allocator;
	DictHash  hash_fn;
	DictEq    eq_fn;
	DictCpy   key_cpy_fn;
	DictCpy   val_cpy_fn;
} Dict;

typedef struct DictIter
{
	const Dict* dict;
	size_t it;
} DictIter;

EE_EXTERN_C_START

EE_INLINE u64 ee_dict_th(u64 cap)
{
	return (cap * 896) >> 10;
}

#ifdef EE_DICT_TOMBS_REHASH
EE_INLINE u64 ee_tombs_th(u64 cap)
{
	return cap >> 3;
}
#endif

EE_INLINE u64 ee_hash64(const u8* key) 
{
	u64 hash;

	memcpy(&hash, key, sizeof(u64));

	hash ^= hash >> 30;
	hash *= 0xbf58476d1ce4e5b9ULL;
	hash ^= hash >> 27;
	hash *= 0x94d049bb133111ebULL;
	hash ^= hash >> 31;

	return hash;
}

EE_INLINE u64 ee_hash(const u8* key, size_t len) 
{
	if (len == sizeof(u64))
	{
		return ee_hash64(key);
	}

	u64 hash = 0x9e3779b97f4a7c15ull;
	size_t i = 0;
	
	for (; i + sizeof(u64) <= len; i += sizeof(u64))
	{
		u64 key_u64 = 0;
		memcpy(&key_u64, &key[i], sizeof(u64));

		hash ^= key_u64 + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
		hash ^= hash >> 30;
		hash *= 0xbf58476d1ce4e5b9ULL;
		hash ^= hash >> 27;
	}

	if (len > i)
	{
		u64 key_rem = 0;
		memcpy(&key_rem, &key[i], len - i);

		hash ^= key_rem + 0x9e3779b97f4a7c15ull + (hash << 6) + (hash >> 2);
		hash ^= hash >> 30;
		hash *= 0xbf58476d1ce4e5b9ULL;
		hash ^= hash >> 27;
	}

	return hash;
}

EE_INLINE DictConfig ee_dict_config_new(Allocator* allocator, DictHash hash_fn, DictEq eq_fn, DictCpy key_cpy_fn, DictCpy val_cpy_fn)
{
	DictConfig out = { 0 };

	out.allocator  = allocator;
	out.hash_fn    = hash_fn;
	out.eq_fn      = eq_fn;
	out.key_cpy_fn = key_cpy_fn;
	out.val_cpy_fn = val_cpy_fn;

	return out;
}

EE_INLINE DictConfig ee_dict_config_def()
{
	DictConfig out = { 0 };

	return out;
}

EE_INLINE u8* ee_dict_key_at(const Dict* dict, size_t i)
{
	EE_ASSERT(dict != NULL, "Trying to acces NULL dict slot");
	EE_ASSERT(i < dict->cap, "Invalid key index (%zu) for dict with cap (%zu)", i, dict->cap);

	return &dict->keys.buffer[i * dict->key_len];
}

EE_INLINE u8* ee_dict_val_at(const Dict* dict, size_t i)
{
	EE_ASSERT(dict != NULL, "Trying to acces NULL dict slot");
	EE_ASSERT(i < dict->cap, "Invalid val index (%zu) for dict with cap (%zu)", i, dict->cap);

	return &dict->vals.buffer[i * dict->val_len];
}



EE_INLINE DictIter ee_dict_iter_new(const Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to create iterator over NULL Dict");

	DictIter out = { 0 };

	out.dict = dict;
	out.it = 0;

	return out;
}

EE_INLINE void ee_dict_iter_reset(DictIter* iter)
{
	EE_ASSERT(iter != NULL, "Trying to reset NULL DictIter");

	iter->it = 0;
}

EE_INLINE DictConfig ee_dict_get_config(const Dict* dict)
{
	DictConfig out = { 0 };

	out.allocator  = &dict->allocator;
	out.hash_fn    = dict->hash_fn;
	out.eq_fn      = dict->eq_fn;
	out.key_cpy_fn = dict->key_cpy_fn;
	out.val_cpy_fn = dict->val_cpy_fn;

	return out;
}

EE_INLINE i32 ee_dict_iter_sp_next(DictIter* iter, u8* key_out, u8* val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrls = (const u8*)iter->dict->ctrl.buffer;
	size_t i = iter->it;
	size_t high = ee_round_up_pow2(iter->it, EED_SIMD_BYTES);

	for (; i < high; ++i)
	{
		if (ctrls[i] & 0x80)
			continue;

		iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
		iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

		iter->it = i + 1;

		return EE_TRUE;
	}

	eed_simd_i p_empty = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i p_deleted = eed_set1_epi8(EE_SLOT_DELETED);

	for (; i < iter->dict->cap; i += EED_SIMD_BYTES)
	{
		eed_simd_i group = eed_load_si((const eed_simd_i*)&ctrls[i]);
		eed_simd_i match = eed_or_si(eed_cmpeq_epi8(group, p_empty), eed_cmpeq_epi8(group, p_deleted));

		i32 mask = (~eed_movemask_epi8(match)) & ((1u << EED_SIMD_BYTES) - 1);

		if (mask)
		{
			i32 first = ee_first_bit_u32(mask);
			size_t pos = (size_t)first + i;

			iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
			iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

			iter->it = pos + 1;

			return EE_TRUE;
		}
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_sp_next_ptr(DictIter* iter, u8** key_out, u8** val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrls = (const u8*)iter->dict->ctrl.buffer;
	size_t i = iter->it;
	size_t high = ee_round_up_pow2(iter->it, EED_SIMD_BYTES);

	for (; i < high; ++i)
	{
		if (ctrls[i] & 0x80)
			continue;

		*key_out = ee_dict_key_at(iter->dict, i);
		*val_out = ee_dict_val_at(iter->dict, i);

		iter->it = i + 1;

		return EE_TRUE;
	}

	eed_simd_i p_empty = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i p_deleted = eed_set1_epi8(EE_SLOT_DELETED);

	for (; i < iter->dict->cap; i += EED_SIMD_BYTES)
	{
		eed_simd_i group = eed_load_si((const eed_simd_i*)&ctrls[i]);
		eed_simd_i match = eed_or_si(eed_cmpeq_epi8(group, p_empty), eed_cmpeq_epi8(group, p_deleted));

		i32 mask = (~eed_movemask_epi8(match)) & ((1u << EED_SIMD_BYTES) - 1);

		if (mask)
		{
			i32 first = ee_first_bit_u32(mask);
			size_t pos = (size_t)first + i;

			*key_out = ee_dict_key_at(iter->dict, pos);
			*val_out = ee_dict_val_at(iter->dict, pos);

			iter->it = pos + 1;

			return EE_TRUE;
		}
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_next(DictIter* iter, u8* key_out, u8* val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrl = iter->dict->ctrl.buffer;

	for (size_t i = iter->it; i < iter->dict->cap; ++i)
	{
		if (ctrl[i] & 0x80)
			continue;

		iter->dict->key_cpy_fn(key_out, ee_dict_key_at(iter->dict, i), iter->dict->key_len);
		iter->dict->val_cpy_fn(val_out, ee_dict_val_at(iter->dict, i), iter->dict->val_len);

		iter->it = i + 1;

		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE i32 ee_dict_iter_next_ptr(DictIter* iter, u8** key_out, u8** val_out)
{
	EE_ASSERT(iter != NULL, "Trying to dereference NULL DictIter");
	EE_ASSERT(key_out != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val_out != NULL, "Trying to dereference NULL value");

	if (iter->it >= iter->dict->cap)
	{
		return EE_FALSE;
	}

	const u8* ctrl = iter->dict->ctrl.buffer;

	for (size_t i = iter->it; i < iter->dict->cap; ++i)
	{
		if (ctrl[i] & 0x80)
			continue;

		*key_out = ee_dict_key_at(iter->dict, i);
		*val_out = ee_dict_val_at(iter->dict, i);

		iter->it = i + 1;

		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE Dict ee_dict_new(size_t size, size_t key_len, size_t val_len, DictConfig config)
{
	EE_ASSERT(key_len > 0, "Invalid key_len (%zu)", key_len);
	EE_ASSERT(val_len > 0, "Invalid val_len (%zu)", val_len);

	Dict out = { 0 };

	if (size < EE_DICT_START_SIZE)
	{
		size = EE_DICT_START_SIZE;
	}

	if (config.allocator == NULL)
	{
		out.allocator.alloc_fn = ee_default_alloc;
		out.allocator.realloc_fn = ee_default_realloc;
		out.allocator.free_fn = ee_default_free;
		out.allocator.context = NULL;
	}
	else
	{
		memcpy(&out.allocator, config.allocator, sizeof(Allocator));
	}

	EE_ASSERT(out.allocator.alloc_fn != NULL, "Trying to set NULL alloc callback");
	EE_ASSERT(out.allocator.realloc_fn != NULL, "Trying to set NULL realloc callback");
	EE_ASSERT(out.allocator.free_fn != NULL, "Trying to set NULL free callback");

	size_t cap = ee_next_pow_2(size);

	out.key_len = key_len;
	out.val_len = val_len;

	out.keys = ee_aligned_alloc(size * out.key_len, EE_MAX_ALIGN, &out.allocator);
	out.vals = ee_aligned_alloc(size * out.val_len, EE_MAX_ALIGN, &out.allocator);
	out.ctrl = ee_aligned_alloc(size, EE_MAX_ALIGN, &out.allocator);

	out.count = 0;
	out.cap = cap;
	out.mask = out.cap - 1;
	out.th = ee_dict_th(out.cap);
	
	out.hash_fn    = config.hash_fn == NULL ? ee_hash : config.hash_fn;
	out.eq_fn      = config.eq_fn   == NULL ? ee_bin_u8_eq : config.eq_fn;
	out.key_cpy_fn = config.key_cpy_fn == NULL ? memcpy : config.key_cpy_fn;
	out.val_cpy_fn = config.val_cpy_fn == NULL ? memcpy : config.val_cpy_fn;

#ifdef EE_DICT_TOMBS_REHASH
	out.tombs = 0;
	out.tombs_th = ee_tombs_th(out.cap);
#endif

	EE_ASSERT(out.ctrl.buffer != NULL, "NULL control buffer");

	memset(out.ctrl.buffer, EE_SLOT_EMPTY, cap);

	return out;
}

EE_INLINE void ee_dict_free(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to free NULL Dict");

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	memset(dict, 0, sizeof(Dict));
}

EE_INLINE i32 ee_dict_insert(Dict* dict, const u8* key, const u8* val)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");
	EE_ASSERT(val != NULL, "Trying to dereference NULL value");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);
	eed_simd_i deleted128 = eed_set1_epi8(EE_SLOT_DELETED);

	size_t probe_step = 0;
	size_t first_deleted = (size_t)-1;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);

		i32 match_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, hash_sign128));
		
		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);
		
			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				dict->val_cpy_fn(ee_dict_val_at(dict, group_index + first), val, dict->val_len);
			
				return EE_TRUE;
			}
			
			match_mask &= match_mask - 1;
		}

		i32 deleted_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, deleted128));
		
		if (deleted_mask && first_deleted == (size_t)-1)
		{
			first_deleted = group_index + (size_t)ee_first_bit_u32(deleted_mask);
		}

		i32 empty_mask = eed_movemask_epi8(eed_cmpeq_epi8(group, empty128));
		
		if (empty_mask)
		{
			size_t place = (first_deleted != (size_t)-1) ? first_deleted : (group_index + (size_t)ee_first_bit_u32(empty_mask));
			
			dict->key_cpy_fn(ee_dict_key_at(dict, place), key, dict->key_len);
			dict->val_cpy_fn(ee_dict_val_at(dict, place), val, dict->val_len);
			
			dict->ctrl.buffer[place] = hash_sign;
			dict->count++;
			
			return EE_TRUE;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	if (first_deleted != (size_t)-1)
	{
		size_t place = first_deleted;

		dict->key_cpy_fn(ee_dict_key_at(dict, place), key, dict->key_len);
		dict->val_cpy_fn(ee_dict_val_at(dict, place), val, dict->val_len);

		dict->ctrl.buffer[place] = hash_sign;
		dict->count++;
		
		return EE_TRUE;
	}

	return EE_FALSE;
}

EE_INLINE void ee_dict_grow(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	DictConfig config = ee_dict_get_config(dict);
	Dict out = ee_dict_new(dict->cap * 2, dict->key_len, dict->val_len, config);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrl.buffer[i] & 0x80)
			continue;

		ee_dict_insert(&out, ee_dict_key_at(dict, i), ee_dict_val_at(dict, i));
	}

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	*dict = out;
}

EE_INLINE void ee_dict_rehash(Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to insert to NULL Dict");

	DictConfig config = ee_dict_get_config(dict);
	Dict out = ee_dict_new(dict->cap, dict->key_len, dict->val_len, config);

	for (size_t i = 0; i < dict->cap; ++i)
	{
		if (dict->ctrl.buffer[i] & 0x80)
			continue;

		ee_dict_insert(&out, ee_dict_key_at(dict, i), ee_dict_val_at(dict, i));
	}

	ee_aligned_free(&dict->keys, &dict->allocator);
	ee_aligned_free(&dict->vals, &dict->allocator);
	ee_aligned_free(&dict->ctrl, &dict->allocator);

	*dict = out;
}

EE_INLINE i32 ee_dict_set(Dict* dict, const u8* key, const u8* val)
{
	i32 result = ee_dict_insert(dict, key, val);

	if (!result)
	{
		ee_dict_grow(dict);
		result = ee_dict_insert(dict, key, val);

		EE_ASSERT(result == EE_TRUE, "Unable to insert after grow");
	}

	if (dict->count > dict->th)
	{
		ee_dict_grow(dict);
	}

	return result;
}

EE_INLINE i32 ee_dict_remove(Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign128);

		i32 match_mask = eed_movemask_epi8(match);

		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				dict->ctrl.buffer[group_index + first] = EE_SLOT_DELETED;
				dict->count--;

				#ifdef EE_DICT_TOMBS_REHASH
				dict->tombs++;

				if (dict->tombs >= dict->tombs_th)
				{
					ee_dict_rehash(dict);
				}
				#endif

				return EE_TRUE;
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty128);

		i32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return EE_FALSE;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return EE_FALSE;
}

EE_INLINE u8* ee_dict_at(const Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");
	EE_ASSERT(key != NULL, "Trying to dereference NULL key");

	u64 hash = dict->hash_fn(key, dict->key_len);
	u64 base_index = (hash >> 7) & dict->mask;
	u8  hash_sign = hash & 0x7F;

	eed_simd_i hash_sign128 = eed_set1_epi8(hash_sign);
	eed_simd_i empty128 = eed_set1_epi8(EE_SLOT_EMPTY);

	size_t probe_step = 0;

	while (probe_step < dict->cap)
	{
		size_t group_index = base_index & EE_GROUP_MASK;

		size_t next_probe_step = probe_step + 1;
		size_t next_base_index = (base_index + EE_GROUP_SIZE * next_probe_step) & dict->mask;
		size_t next_group_index = next_base_index & EE_GROUP_MASK;

		eed_prefetch((const char*)&dict->ctrl.buffer[next_group_index], EED_SIMD_PREFETCH_T0);
		eed_prefetch((const char*)ee_dict_key_at(dict, next_group_index), EED_SIMD_PREFETCH_T0);

		eed_simd_i group = eed_load_si((eed_simd_i*)&dict->ctrl.buffer[group_index]);
		eed_simd_i match = eed_cmpeq_epi8(group, hash_sign128);

		i32 match_mask = eed_movemask_epi8(match);
		
		while (match_mask)
		{
			i32 first = ee_first_bit_u32(match_mask);

			if (dict->eq_fn(ee_dict_key_at(dict, group_index + first), key, dict->key_len))
			{
				return ee_dict_val_at(dict, group_index + first);
			}

			match_mask &= match_mask - 1;
		}

		eed_simd_i empty = eed_cmpeq_epi8(group, empty128);

		i32 empty_mask = eed_movemask_epi8(empty);

		if (empty_mask)
		{
			return NULL;
		}

		probe_step = next_probe_step;
		base_index = next_base_index;
	}

	return NULL;
}

EE_INLINE int ee_dict_contains(const Dict* dict, const u8* key)
{
	EE_ASSERT(dict != NULL, "Trying to check NULL Dict");
	EE_ASSERT(key != NULL, "Trying to check NULL key");

	u8* val = ee_dict_at(dict, key);

	return val != NULL;
}

EE_INLINE size_t ee_dict_count(const Dict* dict)
{
	EE_ASSERT(dict != NULL, "Trying to dereference NULL Dict");

	return dict->count;
}

EE_EXTERN_C_END

#endif // EE_DICT_H