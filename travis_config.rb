MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug

  # quickfix to use mruby-io
  conf.cc.flags << '-fpermissive'

  conf.gembox 'default'

  # k2hash requires C++ so it enables cxxabi
  enable_cxx_abi
  conf.gem '../'

  conf.gem :github => 'iij/mruby-mtest'
end
