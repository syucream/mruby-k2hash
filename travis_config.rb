MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug
  conf.enable_bintest
  conf.enable_test

  # to use k2hash
  conf.linker.flags_after_libraries << "-lstdc++ -ldl -lcrypto -lpthread"

  conf.gembox 'default'
  conf.gem './'
end
