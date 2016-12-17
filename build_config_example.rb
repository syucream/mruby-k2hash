MRuby::Build.new do |conf|
  toolchain :gcc

  enable_debug

  conf.gembox 'default'

  # k2hash requires C++ so it enables cxxabi
  enable_cxx_abi
  conf.gem 'mrbgems/mruby-k2hash/'
end
