# mruby-k2hash

[![Build Status](https://travis-ci.org/syucream/mruby-k2hash.svg?branch=master)](https://travis-ci.org/syucream/mruby-k2hash)

mruby binding of [yahoojapan/k2hash](https://github.com/yahoojapan/k2hash)

# Quickstart

1. build mruby with mruby-k2hash

```
# mruby root dir
$ MRUBY_CONFIG=./path/to/build_config.rb ./minirake
```

2. Run mirb and test K2Hash class

```
$ ./build/host/bin/mirb
mirb - Embeddable Interactive Ruby Shell

> 
>
> k2hash = K2Hash.new('/tmp/tmp.k2hash', 0666, K2Hash::WRCREAT)
 => #<K2Hash:0x127ada0>
> k2hash.store('key', 'value')
 => nil
> k2hash.fetch('key')
 => "value"
> k2hash.store('key', 'new value')
 => nil
> k2hash.fetch('key')
 => "new value"
> k2hash['key2'] = 'value2'
 => "value2"
> k2hash['key2']
 => "value2"
> k2hash.close
 => nil
```

# License

MITL
