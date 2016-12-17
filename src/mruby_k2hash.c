#include <stdlib.h>
#include <string.h>

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/variable.h"

#include "k2hash.h"

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

static void
_k2hash_close(mrb_state *mrb, void *p)
{
  k2h_close((k2h_h)p);
}

static const
struct mrb_data_type K2Hash_type = {
  "K2Hash", _k2hash_close,
};

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
    // TODO Check whether the file exists
    handler = k2h_open_rw(filename, 1,
        DEFAULT_MASK_BITCOUNT, DEFAULT_COLLISION_MASK_BITCOUNT, DEFAULT_MAX_ELEMENT_CNT, MIN_PAGE_SIZE);
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

  k2h_h handler = (k2h_h)DATA_PTR(self);
  if (!handler) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2hash is closed");
  }

  unsigned char* pval = NULL;
  size_t vallen = 0;

  bool success = k2h_get_value(handler, (unsigned char*)key, strlen(key), &pval, &vallen);
  mrb_value rv = success ? mrb_str_new(mrb, (char*)pval, vallen) : mrb_nil_value();

  free(pval);
  return rv;
}

static mrb_value
mrb_k2hash_set(mrb_state *mrb, mrb_value self)
{
  char *key, *value;
  mrb_get_args(mrb, "zz", &key, &value);

  k2h_h handler = (k2h_h)DATA_PTR(self);
  if (!handler) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2hash is closed");
  }

  bool success = k2h_set_value(handler, (unsigned char*)key, strlen(key), (unsigned char*)value, strlen(value));

  if (success) {
    return mrb_nil_value();
  } else {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2h_set_value is failed");
  }
}

static mrb_value
mrb_k2hash_close(mrb_state *mrb, mrb_value self)
{
  k2h_h handler = (k2h_h)DATA_PTR(self);
  if (!handler) {
    mrb_raise(mrb, E_K2HASH_ERROR, "k2hash is closed");
  }

  k2h_close(handler);
  DATA_PTR(self) = NULL;

  return mrb_nil_value();
}

void
mrb_mruby_k2hash_gem_init(mrb_state* mrb)
{
  struct RClass *rclass;

  rclass = mrb_define_class(mrb, K2HASH_CLASSNAME, mrb->object_class);

  mrb_define_method(mrb, rclass, "initialize", mrb_k2hash_open, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, rclass, "open", mrb_k2hash_open, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, rclass, "fetch", mrb_k2hash_get, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "store", mrb_k2hash_set, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, rclass, "[]", mrb_k2hash_get, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, rclass, "[]=", mrb_k2hash_set, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, rclass, "close", mrb_k2hash_close, MRB_ARGS_NONE());

  mrb_define_const(mrb, rclass, "READER", mrb_fixnum_value(FLAG_READER));
  mrb_define_const(mrb, rclass, "WRITER", mrb_fixnum_value(FLAG_WRITER));
  mrb_define_const(mrb, rclass, "WRCREAT", mrb_fixnum_value(FLAG_WRCREAT));
  mrb_define_const(mrb, rclass, "NEWDB", mrb_fixnum_value(FLAG_NEWDB));
}

void
mrb_mruby_k2hash_gem_final(mrb_state* mrb)
{
}

