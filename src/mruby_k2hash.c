#include <stdlib.h>
#include <string.h>

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/proc.h"
#include "mruby/variable.h"

#include "k2hash.h"
#include "k2hutil.h"

#define PAIR_ARGC 2
#define K2HASH_CLASSNAME "K2Hash"
#define E_K2HASH_ERROR (mrb_class_get_under(mrb, mrb_class_get(mrb, K2HASH_CLASSNAME), "K2HashHandlerError"))

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
#define K2HASH_ITER_BEGIN(mrb, h, k, klen, v, vlen)                                          \
  for (k2h_find_h fh = k2h_find_first(h); K2H_INVALID_HANDLE != fh; fh = k2h_find_next(fh)) { \
    bool failed = false;                                                                      \
    if (k2h_find_get_key(fh, &k, &klen) && k2h_find_get_value(fh, &v, &vlen)) {

#define K2HASH_ITER_END(mrb, k, v)                                                           \
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

#define _k2hash_each_pair(mrb, block, h, rv)                                                  \
  for (k2h_find_h fh = k2h_find_first(h); K2H_INVALID_HANDLE != fh; fh = k2h_find_next(fh)) { \
    bool failed = _yield_with_pair(mrb, block, fh, &rv);                                      \
    if (failed) {                                                                             \
      mrb_raise(mrb, E_K2HASH_ERROR, "k2h_find_get key/value is failed");                     \
    }                                                                                         \
  }                                                                                           \

#define _k2hash_each(target, mrb, block, h, rv)                                               \
  for (k2h_find_h fh = k2h_find_first(h); K2H_INVALID_HANDLE != fh; fh = k2h_find_next(fh)) { \
    bool failed = _yield_with_##target(mrb, block, fh, &rv);                                  \
    if (failed) {                                                                             \
      mrb_raise(mrb, E_K2HASH_ERROR, "k2h_find_get ##target is failed");                      \
    }                                                                                         \
  }                                                                                           \

static inline bool
_yield_with_pair(mrb_state* mrb, mrb_value block, k2h_find_h fh, mrb_value* rv)
{
  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  bool failed = false;

  if (k2h_find_get_key(fh, &ck, &klen) && k2h_find_get_value(fh, &cv, &vlen)) {
    mrb_value key = mrb_str_new(mrb, (char*)ck, klen);
    mrb_value val = mrb_str_new(mrb, (char*)cv, vlen);
    mrb_value argv[PAIR_ARGC] = {key, val};
    *rv = mrb_yield_argv(mrb, block, PAIR_ARGC, argv);
  } else {
    failed = true;
  }
  K2H_Free(ck);
  K2H_Free(cv);

  return failed;
}

static inline bool
_yield_with_key(mrb_state* mrb, mrb_value block, k2h_find_h fh, mrb_value* rv)
{
  unsigned char *carg = NULL;
  size_t arglen = 0;
  bool failed = false;

  if (k2h_find_get_key(fh, &carg, &arglen)) {
    mrb_value arg = mrb_str_new(mrb, (char*)carg, arglen);
    *rv = mrb_yield(mrb, block, arg);
  } else {
    failed = true;
  }
  K2H_Free(carg);

  return failed;
}

static inline bool
_yield_with_value(mrb_state* mrb, mrb_value block, k2h_find_h fh, mrb_value* rv)
{
  unsigned char *carg = NULL;
  size_t arglen = 0;
  bool failed = false;

  if (k2h_find_get_value(fh, &carg, &arglen)) {
    mrb_value arg = mrb_str_new(mrb, (char*)carg, arglen);
    *rv = mrb_yield(mrb, block, arg);
  } else {
    failed = true;
  }
  K2H_Free(carg);

  return failed;
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
    // TODO Clear old key/value's
    handler = k2h_open_rw(filename, 1,
        DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
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

  k2h_h handler = _k2hash_get_handler(mrb, self);
  mrb_value rv = mrb_nil_value();
  _k2hash_each_pair(mrb, block, handler, rv);

  return rv;
}

static mrb_value
mrb_k2hash_each_key(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  k2h_h handler = _k2hash_get_handler(mrb, self);
  mrb_value rv = mrb_nil_value();
  _k2hash_each(key, mrb, block, handler, rv);

  return rv;
}

static mrb_value
mrb_k2hash_each_value(mrb_state *mrb, mrb_value self)
{
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  k2h_h handler = _k2hash_get_handler(mrb, self);
  mrb_value rv = mrb_nil_value();
  _k2hash_each(value, mrb, block, handler, rv);

  return rv;
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
mrb_k2hash_clear(mrb_state *mrb, mrb_value self)
{
  unsigned char *ck	= NULL, *cv = NULL;
  size_t klen = 0, vlen = 0;
  k2h_h handler = _k2hash_get_handler(mrb, self);

  K2HASH_ITER_BEGIN(mrb, handler, ck, klen, cv, vlen);
  {
    failed = !(k2h_remove_all(handler, ck, klen));
  }
  K2HASH_ITER_END(mrb, ck, cv);

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
_mrb_k2hash_values_map(mrb_state *mrb, mrb_value v)
{
  mrb_value key, val;
  mrb_get_args(mrb, "SS", &key, &val);
  return val;
}

#define DEFINE_MAPPER(target)                                                             \
  static mrb_value                                                                        \
  mrb_k2hash_##target(mrb_state *mrb, mrb_value self)                                     \
  {                                                                                       \
    RProc* proc = mrb_proc_new_cfunc(mrb, _mrb_k2hash_##target##_map);                    \
    mrb_value block = mrb_obj_value(proc);                                                \
    return mrb_funcall_with_block(mrb, self, mrb_intern_lit(mrb, "map"), 0, NULL, block); \
  }

DEFINE_MAPPER(keys)
DEFINE_MAPPER(values)

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
  mrb_define_method(mrb, rclass, "delete", mrb_k2hash_delete, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "delete_if", mrb_k2hash_delete_if, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each", mrb_k2hash_each, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_key", mrb_k2hash_each_key, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_pair", mrb_k2hash_each, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "each_value", mrb_k2hash_each_value, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "empty?", mrb_k2hash_empty_q, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "fetch", mrb_k2hash_get, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "has_key?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "include?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "invert", mrb_k2hash_invert, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "key?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "member?", mrb_k2hash_has_key_q, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "open", mrb_k2hash_open, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, rclass, "reject!", mrb_k2hash_delete_if, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "store", mrb_k2hash_set, MRB_ARGS_REQ(2));

  mrb_include_module(mrb, rclass, mrb_module_get(mrb, "Enumerable"));
  mrb_define_method(mrb, rclass, "keys", mrb_k2hash_keys, MRB_ARGS_NONE());
  mrb_define_method(mrb, rclass, "values", mrb_k2hash_values, MRB_ARGS_NONE());
  mrb_define_alias(mrb, rclass, "length", "count");
  mrb_define_alias(mrb, rclass, "size", "count");
  mrb_define_alias(mrb, rclass, "to_hash", "to_h");

  mrb_define_const(mrb, rclass, "READER", mrb_fixnum_value(FLAG_READER));
  mrb_define_const(mrb, rclass, "WRITER", mrb_fixnum_value(FLAG_WRITER));
  mrb_define_const(mrb, rclass, "WRCREAT", mrb_fixnum_value(FLAG_WRCREAT));
  mrb_define_const(mrb, rclass, "NEWDB", mrb_fixnum_value(FLAG_NEWDB));
}

void
mrb_mruby_k2hash_gem_final(mrb_state* mrb)
{
}

