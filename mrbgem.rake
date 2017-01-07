MRuby::Gem::Specification.new('mruby-k2hash') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Ryo Okubo'
  spec.version = '0.1.1'

  k2hash_dir = "#{build_dir}/k2hash"

  if !File.exists?(k2hash_dir)
    Dir.chdir(build_dir) do
      `git submodule init && git submodule update`
      Dir.chdir(build_dir + '/fullock/') do
        `git submodule init && git submodule update`
      end
    end
  end

  if !File.exists?("#{k2hash_dir}/lib/.libs/libk2hash.a")
    Dir.chdir(build_dir) do
      `autoreconf -if`
      `./configure --with-k2hash-prefix=#{k2hash_dir}`
      `make k2hash`
    end
  end

  spec.cc.include_paths << "#{k2hash_dir}/lib"
  spec.linker.flags_before_libraries << "#{k2hash_dir}/lib/.libs/libk2hash.a"
  spec.linker.flags_after_libraries << "-ldl -lcrypto -lpthread -lfullock"
end
