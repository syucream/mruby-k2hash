# mruby-k2hash

[![Build Status](https://travis-ci.org/syucream/mruby-k2hash.svg?branch=master)](https://travis-ci.org/syucream/mruby-k2hash)

mruby binding of [yahoojapan/k2hash](https://github.com/yahoojapan/k2hash)

# Quickstart

1. build mruby with mruby-k2hash. Or use [the Docker image](https://hub.docker.com/r/syucream/mruby-k2hash/).

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

# Implemented methods

* [DBM](http://ruby-doc.org/stdlib-2.3.3/libdoc/dbm/rdoc/DBM.html) like methods:

  - `H2Hash` has `each` so you can also use [Enumerable methods](https://ruby-doc.org/core-2.3.3/Enumerable.html).


| method      | implemented?       |
|:------------|--------------------|
| []          | :heavy_check_mark: |
| []=         | :heavy_check_mark: |
| clear       | :heavy_check_mark: |
| close       | :heavy_check_mark: |
| closed?     | :heavy_check_mark: |
| delete      | :heavy_check_mark: |
| delete_if   | :heavy_check_mark: |
| reject!     | :heavy_check_mark: |
| each        | :heavy_check_mark: |
| each_pair   | :heavy_check_mark: |
| each_key    | :heavy_check_mark: |
| each_value  | :heavy_check_mark: |
| empty?      | :heavy_check_mark: |
| fetch       | :heavy_check_mark: |
| has_key?    | :heavy_check_mark: |
| include?    | :heavy_check_mark: |
| key?        | :heavy_check_mark: |
| member?     | :heavy_check_mark: |
| has_value?  | :heavy_check_mark: |
| value?      | :heavy_check_mark: |
| invert      | :heavy_check_mark: |
| key         | :heavy_check_mark: |
| keys        | :heavy_check_mark: |
| length      | :heavy_check_mark: |
| size        | :heavy_check_mark: |
| reject      | :heavy_check_mark: |
| replace     | :heavy_check_mark: |
| select      | :heavy_check_mark: |
| shift       | :heavy_check_mark: |
| store       | :heavy_check_mark: |
| to_a        | :heavy_check_mark: |
| to_hash     | :heavy_check_mark: |
| update      | :heavy_check_mark: |
| values      | :heavy_check_mark: |
| values_at   | :heavy_check_mark: |

* subkey

  - `get_subkeys(key) -> Array` returns subkey array corresponding to the key str
  - `set_subkeys(key, subkeys) -> self` set the subkey array to the key str

# TODO

* Support k2hash features. Especially...:

  - open mode
  - transaction
  - queue
  - attributes

# License

MITL
