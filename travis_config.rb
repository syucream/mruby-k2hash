MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug
  # k2hash requires C++ so it enables cxxabi
  enable_cxx_abi

  conf.enable_bintest
  conf.enable_test

  # quickfix to use mruby-io
  conf.cc.flags << '-fpermissive'
  # to use k2hash
  conf.linker.flags_after_libraries << "-ldl -lcrypto -lpthread -lfullock"

  conf.gembox 'default'
  conf.gem '../'
end
