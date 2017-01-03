MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug
  # k2hash requires C++ so it enables cxxabi
  enable_cxx_abi

  conf.gembox 'default'

  conf.gem :github => 'syucream/mruby-k2hash'
end
