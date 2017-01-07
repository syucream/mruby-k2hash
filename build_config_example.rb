MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug
  # k2hash requires C++ so it enables cxxabi
  enable_cxx_abi

  conf.enable_bintest
  conf.enable_test

  # to use k2hash
  conf.linker.flags_after_libraries << "-ldl -lcrypto -lpthread -lfullock"

  conf.gembox 'default'

  conf.gem :github => 'syucream/mruby-k2hash'
end
