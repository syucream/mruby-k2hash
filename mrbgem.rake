MRuby::Gem::Specification.new('mruby-k2hash') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Ryo Okubo'
  spec.version = '0.1.1'

  k2hash_dir = "#{build_dir}/k2hash"
  fullock_dir = "#{build_dir}/k2hash/fullock"
  prefix_dir = "#{build_dir}/_prefix"

  FileUtils.mkdir_p build_dir
  FileUtils.mkdir_p prefix_dir

  if !File.exists?(k2hash_dir)
    Dir.chdir(build_dir) do
      `git clone --depth 1 https://github.com/yahoojapan/k2hash.git`
      Dir.chdir(k2hash_dir) do
        `git submodule update --init --recursive`
        Dir.chdir(fullock_dir) do
          `./autogen.sh && ./configure --prefix=#{prefix_dir} && make && make install`
        end
        `ln -s #{prefix_dir}/include/fullock lib/fullock` # to avoid installing fullock
        `./autogen.sh && ./configure --prefix=#{prefix_dir} && make`
      end
    end
  end

  spec.cc.include_paths << "#{k2hash_dir}/lib"
  spec.linker.flags_before_libraries << "#{k2hash_dir}/lib/.libs/libk2hash.a"
  spec.linker.flags_after_libraries << "-lstdc++ -ldl -lcrypto -lpthread -lfullock"
end
