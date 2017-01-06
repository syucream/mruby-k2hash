#include <stdlib.h>
#include <string.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/proc.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "k2hash.h"
#include "k2hutil.h"

#define PAIR_ARGC 2
#define K2HASH_CLASSNAME "K2Hash"
#define K2HASH_HANDLER_EXCEPTION "K2HashHandlerError"
#define E_K2HASH_ERROR (mrb_class_get_under(mrb, mrb_class_get(mrb, K2HASH_CLASSNAME), K2HASH_HANDLER_EXCEPTION))

// Original definition is k2hash/lib/k2hshm.h
#define DEFAULT_MASK_BITCOUNT 8
#define DEFAULT_COLLISION_MASK_BITCOUNT 4
#define DEFAULT_MAX_ELEMENT_CNT 32
#define MIN_PAGE_SIZE 128

enum OpenFlag {
  FLAG_READER,
  FLAG_WRITER,
  FLAG_WRCREAT,
  FLAG_NEWDB
};

/*
 * Utils
 */
#define K2HASH_ITER_BEGIN(mrb, h, k, klen, v, vlen)                                           \
  for (k2h_find_h fh = k2h_find_first(h); K2H_INVALID_HANDLE != fh; fh = k2h_find_next(fh)) { \
    bool failed = false;                                                                      \
    if (k2h_find_get_key(fh, &k, &klen) && k2h_find_get_value(fh, &v, &vlen)) {

#define K2HASH_ITER_END(mrb, k, v)                                                            \
    } else {                                                                                  \
      failed = true;                                                                          \
    }                                                                                         \
    K2H_Free(k);                                                                              \
    K2H_Free(v);                                                                              \
                                                                                              \
    if (failed) {                                                                             \
      mrb_raise(mrb, E_K2HASH_ERROR, "k2h_find iterations are failed");                       \
    }                                                                                         \
  }

static inline k2h_h
_k2hash_get_handler(mrb_state* mrb, mrb_value self)
{
  k2h_h handler = (k2h_h)DATA_PTR(self);

  if (!handler) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2hash is closed");
  }

  return handler;
}

static void
_k2hash_clear(mrb_state* mrb, k2h_h handler)
{
  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    failed = !(k2h_remove_all(handler, ck, klen));
  }
  K2HASH_ITER_END(mrb, ck, cv);
}

static void
_k2hash_close(mrb_state *mrb, void *p)
{
  k2h_close((k2h_h)p);
}

static const
struct mrb_data_type K2Hash_type = {
  "K2Hash", _k2hash_close,
};


/*
 * Core methods
 */
static mrb_value
mrb_k2hash_open(mrb_state *mrb, mrb_value self)
{
  char* filename;
  mrb_int flags, mode;

  mrb_get_args(mrb, "zii", &filename, &mode, &flags);

  k2h_h handler = 0;
  switch(flags) {
  case FLAG_READER:
    handler = k2h_open_ro(filename, 1,
        DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
    break;
  case FLAG_WRITER:
    struct stat st;
    if (stat(filename, &st) == 0) {
      handler = k2h_open_rw(filename, 1,
          DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
    }
    break;
  case FLAG_WRCREAT:
    handler = k2h_open_rw(filename, 1,
        DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
    break;
  case FLAG_NEWDB:
    handler = k2h_open_rw(filename, 1,
        DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
    _k2hash_clear(mrb, handler);
  default:
    break;
  }

  if (!handler) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_open failed");
  }

  DATA_TYPE(self) = &K2Hash_type;
  DATA_PTR(self) = (void*)handler;

  return self;
}

static mrb_value
mrb_k2hash_get(mrb_state *mrb, mrb_value self)
{
  char* key;
  mrb_get_args(mrb, "z", &key);

  k2h_h handler = _k2hash_get_handler(mrb, self);

  unsigned char* pval = NULL;
  size_t vallen = 0;
  bool success = k2h_get_value(handler, (unsigned char*)key, strlen(key), &pval, &vallen);

  mrb_value rv = success ? mrb_str_new(mrb, (char*)pval, vallen) : mrb_nil_value();
  K2H_Free(pval);

  return rv;
}

static mrb_value
mrb_k2hash_set(mrb_state *mrb, mrb_value self)
{
  char *key, *value;
  mrb_get_args(mrb, "zz", &key, &value);

  k2h_h handler = _k2hash_get_handler(mrb, self);

  bool success = k2h_set_value(handler, (unsigned char*)key, strlen(key), (unsigned char*)value, strlen(value));
  if (!success) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_set_value is failed");
  }

  return mrb_nil_value();
}

static mrb_value
mrb_k2hash_delete(mrb_state *mrb, mrb_value self)
{
  char *key;
  mrb_get_args(mrb, "z", &key);

  k2h_h handler = _k2hash_get_handler(mrb, self);

  bool success = k2h_remove_all(handler, (unsigned char*)key, strlen(key));
  if (!success) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_remove_all is failed");
  }

  return mrb_nil_value();
}

static mrb_value
mrb_k2hash_each(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_value argv[PAIR_ARGC] = {key, val};
    mrb_yield_argv(mrb, block, PAIR_ARGC, argv);
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return self;
}

static mrb_value
mrb_k2hash_each_key(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_yield(mrb, block, key);
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return self;
}

static mrb_value
mrb_k2hash_each_value(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value value = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_yield(mrb, block, value);
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return self;
}

static mrb_value
mrb_k2hash_empty_q(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = _k2hash_get_handler(mrb, self);
  bool is_empty = k2h_find_first(handler) == K2H_INVALID_HANDLE;
  return is_empty ? mrb_true_value() : mrb_false_value();
}

static mrb_value
mrb_k2hash_has_key_q(mrb_state *mrb, mrb_value self)
{
  bool has_key = !mrb_nil_p(mrb_k2hash_get(mrb, self));
  return has_key ? mrb_true_value() : mrb_false_value();
}

static mrb_value
mrb_k2hash_has_value_q(mrb_state *mrb, mrb_value self)
{
  mrb_value target;
  mrb_get_args(mrb, "S", &target);

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  mrb_value found = mrb_false_value();
  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    if (mrb_str_equal(mrb, val, target)) {
      found = mrb_true_value();
    }
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return found;
}

static mrb_value
mrb_k2hash_clear(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = _k2hash_get_handler(mrb, self);

  _k2hash_clear(mrb, handler);

  return self;
}

static mrb_value
mrb_k2hash_close(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = _k2hash_get_handler(mrb, self);

  k2h_close(handler);
  DATA_PTR(self) = NULL;

  return mrb_nil_value();
}

static mrb_value
mrb_k2hash_closed_q(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = (k2h_h)DATA_PTR(self);
  return handler == NULL ? mrb_true_value() : mrb_false_value();
}

static mrb_value
mrb_k2hash_delete_if(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_value argv[PAIR_ARGC] = {key, val};
    mrb_value rc = mrb_yield_argv(mrb, block, PAIR_ARGC, argv);

    if (mrb_bool(rc)) {
      failed = !(k2h_remove_all(handler, ck, klen));
    }
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return self;
}

static mrb_value
mrb_k2hash_invert(mrb_state *mrb, mrb_value self)
{
  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  mrb_value hash = mrb_hash_new(mrb);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_hash_set(mrb, hash, val, key);
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return hash;
}

static mrb_value
mrb_k2hash_values_at(mrb_state *mrb, mrb_value self)
{
  mrb_value *argv;
  mrb_int argc;
  mrb_get_args(mrb, "*", &argv, &argc);

  k2h_h handler = _k2hash_get_handler(mrb, self);
  mrb_value array = mrb_ary_new(mrb);

  for (int i = 0; i<argc; i++) {
    mrb_value str = argv[i];
    if (!mrb_string_p(str)) {
      continue;
    }

    const char* pkey = mrb_string_value_ptr(mrb, str);
    size_t keylen = mrb_string_value_len(mrb, str);
    unsigned char* pval = NULL;
    size_t vallen = 0;
    bool found = k2h_get_value(handler, (unsigned char*)pkey, keylen, &pval, &vallen);

    if (found) {
      mrb_value v = mrb_str_new(mrb, (char*)pval, vallen);
      mrb_ary_push(mrb, array, v);
    }

    K2H_Free(pval);
  }

  return array;
}

static mrb_value
mrb_k2hash_shift(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = _k2hash_get_handler(mrb, self);
  k2h_find_h fh = k2h_find_first(handler);

  if (fh == K2H_INVALID_HANDLE) {
    return mrb_nil_value();
  }

  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  bool failed = false;

  mrb_value array = mrb_ary_new(mrb);
  if (k2h_find_get_key(fh, &ck, &klen) && k2h_find_get_value(fh, &cv, &vlen)) {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_ary_push(mrb, array, key);
    mrb_ary_push(mrb, array, val);
    failed = !(k2h_remove_all(handler, ck, klen));
  } else {
    failed = true;
  }
  K2H_Free(ck);
  K2H_Free(cv);

  if (failed) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_find failed");
    return mrb_nil_value();
  } else {
    return array;
  }
}

static mrb_value
mrb_k2hash_count(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = _k2hash_get_handler(mrb, self);

  mrb_int count = 0;
  for (k2h_find_h fh = k2h_find_first(handler); K2H_INVALID_HANDLE != fh; fh = k2h_find_next(fh)) {
    ++count;
  }

  return mrb_fixnum_value(count);
}

static mrb_value
mrb_k2hash_to_hash(mrb_state *mrb, mrb_value self)
{
  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  mrb_value hash = mrb_hash_new(mrb);
  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_hash_set(mrb, hash, key, val);
  }
  K2HASH_ITER_END(mrb, ck, cv);

  return hash;
}

/*
 * Enumerable methods
 */
static mrb_value
_mrb_k2hash_keys_map(mrb_state *mrb, mrb_value v)
{
  mrb_value key, val;
  mrb_get_args(mrb, "SS", &key, &val);
  return key;
}
static mrb_value
mrb_k2hash_keys(mrb_state *mrb, mrb_value self)
{
  RProc* proc = mrb_proc_new_cfunc(mrb, _mrb_k2hash_keys_map);
  mrb_value block = mrb_obj_value(proc);
  return mrb_funcall_with_block(mrb, self, mrb_intern_lit(mrb, "map"), 0, NULL, block);
}

static mrb_value
_mrb_k2hash_values_map(mrb_state *mrb, mrb_value v)
{
  mrb_value key, val;
  mrb_get_args(mrb, "SS", &key, &val);
  return val;
}
static mrb_value
mrb_k2hash_values(mrb_state *mrb, mrb_value self)
{
  RProc* proc = mrb_proc_new_cfunc(mrb, _mrb_k2hash_values_map);
  mrb_value block = mrb_obj_value(proc);
  return mrb_funcall_with_block(mrb, self, mrb_intern_lit(mrb, "map"), 0, NULL, block);
}

static mrb_value
mrb_k2hash_reject(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  mrb_value hash = mrb_funcall(mrb, self, "to_hash", 0);
  mrb_sym reject_sym = mrb_intern_lit(mrb, "reject");
  mrb_value rejected = mrb_funcall_with_block(mrb, hash, reject_sym, 0, NULL, block);

  return rejected;
}

/*
 * K2HASH specifics
 */
static mrb_value
mrb_k2hash_get_subkeys(mrb_state *mrb, mrb_value self)
{
  char* key;
  int keylen = 0;
  mrb_get_args(mrb, "s", &key, &keylen);

  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HKEYPCK* keypack = NULL;
  int count = 0;
  bool success = k2h_get_subkeys(handler, (unsigned char*)key, keylen, &keypack, &count);
  if (!success) {
    count = 0; // To ensure returning empty array
  }

  mrb_value array = mrb_ary_new_capa(mrb, count);
  for (int i = 0; i < count; i++) {
    int ai = mrb_gc_arena_save(mrb);
    const char* csk = (const char*)keypack[i].pkey;
    size_t csklen = keypack[i].length;
    mrb_value sk = mrb_str_new(mrb, csk, csklen);
    mrb_ary_push(mrb, array, sk);
    mrb_gc_arena_restore(mrb, ai);
  }

  k2h_free_keypack(keypack, count);

  return array;
}

static mrb_value
mrb_k2hash_set_subkeys(mrb_state *mrb, mrb_value self)
{
  char* key;
  int keylen = 0;
  mrb_value subkeys;
  mrb_get_args(mrb, "sA", &key, &keylen, &subkeys);

  k2h_h handler = _k2hash_get_handler(mrb, self);

  mrb_int count = RARRAY_LEN(subkeys);
  K2HKEYPCK* keypack = (K2HKEYPCK*)mrb_malloc(mrb, count * sizeof(K2HKEYPCK));
  for (int i = 0; i < count; i++) {
    mrb_value sk = RARRAY_PTR(subkeys)[i];
    // Should it check weather sk is a string?
    keypack[i].pkey = (unsigned char*)mrb_string_value_ptr(mrb, sk);
    keypack[i].length = mrb_string_value_len(mrb, sk);
  }

  bool success = k2h_set_subkeys(handler, (unsigned char*)key, keylen, keypack, count);
  mrb_free(mrb, keypack);

  if (!success) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_set_str_subkeys is failed");
  }
  return self;
}

/*
 * Definitions
 */
void
mrb_mruby_k2hash_gem_init(mrb_state* mrb)
{
  struct RClass *rclass;

  rclass = mrb_define_class(mrb, K2HASH_CLASSNAME, mrb->object_class);

  mrb_define_method(mrb, rclass, "initialize", mrb_k2hash_open, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, rclass, "[]", mrb_k2hash_get, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "[]=", mrb_k2hash_set, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, rclass, "clear", mrb_k2hash_clear, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "close", mrb_k2hash_close, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "closed?", mrb_k2hash_closed_q, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "count", mrb_k2hash_count, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "delete", mrb_k2hash_delete, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "delete_if", mrb_k2hash_delete_if, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each", mrb_k2hash_each, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_key", mrb_k2hash_each_key, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_pair", mrb_k2hash_each, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_value", mrb_k2hash_each_value, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "empty?", mrb_k2hash_empty_q, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "fetch", mrb_k2hash_get, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "has_key?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "has_value?", mrb_k2hash_has_value_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "include?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "invert", mrb_k2hash_invert, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "key?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "member?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "open", mrb_k2hash_open, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, rclass, "reject", mrb_k2hash_reject, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "reject!", mrb_k2hash_delete_if, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "shift", mrb_k2hash_shift, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "store", mrb_k2hash_set, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, rclass, "to_hash", mrb_k2hash_to_hash, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "value?", mrb_k2hash_has_value_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "values_at", mrb_k2hash_values_at, MRB_ARGS_ANY());

  // Enumerable
  mrb_include_module(mrb, rclass, mrb_module_get(mrb, "Enumerable"));
  mrb_define_method(mrb, rclass, "keys", mrb_k2hash_keys, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "values", mrb_k2hash_values, MRB_ARGS_NONE());

  // Subkey
  mrb_define_method(mrb, rclass, "fetch_subkeys", mrb_k2hash_get_subkeys, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "store_subkeys", mrb_k2hash_set_subkeys, MRB_ARGS_REQ(1));

  // Constants
  mrb_define_const(mrb, rclass, "READER", mrb_fixnum_value(FLAG_READER));
  mrb_define_const(mrb, rclass, "WRITER", mrb_fixnum_value(FLAG_WRITER));
  mrb_define_const(mrb, rclass, "WRCREAT", mrb_fixnum_value(FLAG_WRCREAT));
  mrb_define_const(mrb, rclass, "NEWDB", mrb_fixnum_value(FLAG_NEWDB));

  // Exceptions
  mrb_define_class(mrb, K2HASH_HANDLER_EXCEPTION, rclass);
}

void
mrb_mruby_k2hash_gem_final(mrb_state* mrb)
{
}

