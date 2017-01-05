K2HASH_FILENAME = '/tmp/mtest.k2hash'
K2HASH_OTHER_FILENAME = '/tmp/mtest_other.k2hash'

assert 'K2Hash#open' do
  # TODO assert error cases

  writer = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::WRITER)
  writer.store('key1', 'value1')
  writer.close

  reader = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::READER)
  assert_equal reader.fetch('key1'), 'value1'
  reader.close

  wrcreat = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::WRCREAT)
  assert_false wrcreat.empty?
  wrcreat.close

  newdb = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  assert_true newdb.empty?
  newdb.close

  # NOTE dummy assertion to pass mrbtest
  assert_true true
end

assert 'K2Hash#clear' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  k2hash.clear

  assert_equal k2hash.size, 0
end

assert 'K2Hash#closed' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.close

  assert_true k2hash.closed?
end

assert 'K2Hash#delete' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.delete('key1')

  assert_nil k2hash.fetch('key1')
end

assert 'K2Hash#delete_if' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear

  k2hash.store('key1', 'value1')
  k2hash.delete_if do |key, value|
    key == 'key1'
  end
  assert_nil k2hash.fetch('key1')

  k2hash.store('key1', 'value1')
  k2hash.reject! do |key, value|
    key == 'key1'
  end
  assert_nil k2hash.fetch('key1')
end

assert 'K2Hash#fetch_store' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')

  assert_equal k2hash.fetch('key1'), 'value1'
  assert_nil k2hash.fetch('key100')

  k2hash['key2'] = 'value2'

  assert_equal k2hash['key2'], 'value2'
  assert_nil k2hash['key200']
end

assert 'K2Hash#each' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  k2hash.each do |k, v|
    assert_true k == 'key1' || k == 'key2' || k == 'key3'
    assert_true v == 'value1' || v == 'value2' || v == 'value3'
  end

  k2hash.each_pair do |k, v|
    assert_true k == 'key1' || k == 'key2' || k == 'key3'
    assert_true v == 'value1' || v == 'value2' || v == 'value3'
  end
end

assert 'K2Hash#each_key' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  k2hash.each_key do |k|
    assert_true k == 'key1' || k == 'key2' || k == 'key3'
  end
end

assert 'K2Hash#each_value' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  k2hash.each_value do |v|
    assert_true v == 'value1' || v == 'value2' || v == 'value3'
  end
end

assert 'K2Hash#has_key' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')

  assert_true k2hash.has_key?('key1')
  assert_true k2hash.key?('key1')
  assert_true k2hash.include?('key1')
  assert_true k2hash.member?('key1')

  k2hash.delete('key1')

  assert_false k2hash.has_key?('key1')
  assert_false k2hash.key?('key1')
  assert_false k2hash.include?('key1')
  assert_false k2hash.member?('key1')
end

assert 'K2Hash#has_value' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')

  assert_true k2hash.has_value?('value1')
  assert_true k2hash.value?('value1')

  k2hash.delete('key1')

  assert_false k2hash.has_value?('value1')
  assert_false k2hash.value?('value1')
end

assert 'K2Hash#keys' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  keys = k2hash.keys
  assert_true keys.include? 'key1'
  assert_true keys.include? 'key2'
  assert_true keys.include? 'key3'
end

assert 'K2Hash#values' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  values = k2hash.values
  assert_true values.include? 'value1'
  assert_true values.include? 'value2'
  assert_true values.include? 'value3'
end

assert 'K2Hash#invert' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  hash = k2hash.invert
  assert_true hash.is_a?(Hash)

  assert_true hash['value1'] == 'key1'
  assert_true hash['value2'] == 'key2'
  assert_true hash['value3'] == 'key3'
end

assert 'K2Hash#values_at' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  values = k2hash.values_at('key1', 'key2')
  assert_true values.include? 'value1'
  assert_true values.include? 'value2'
  assert_false values.include? 'value3'
end

assert 'K2Hash#shift' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')

  pair1 = k2hash.shift
  assert_equal pair1, ['key1', 'value1']
  assert_true k2hash.empty?
end

#
# Implemented by Enumerable
#

assert 'K2Hash#to_a' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  array = k2hash.to_a
  assert_true array.include? ['key1', 'value1']
  assert_true array.include? ['key2', 'value2']
  assert_true array.include? ['key3', 'value3']
end

assert 'K2Hash#to_hash' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  hash = k2hash.to_hash
  assert_true hash.is_a?(Hash)

  assert_true hash['key1'] == 'value1'
  assert_true hash['key2'] == 'value2'
  assert_true hash['key3'] == 'value3'
end

assert 'K2Hash#select' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  selected = k2hash.select do |key, value|
    key == 'key1'
  end

  assert_equal selected, [['key1', 'value1']]
end

assert 'K2Hash#reject' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear

  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  rejected = k2hash.reject do |key, value|
    key == 'key1'
  end

  assert_true rejected.is_a?(Hash)
  assert_false rejected.has_key?('key1')
  assert_true rejected.has_key?('key2')
  assert_true rejected.has_key?('key3')
end

assert 'K2Hash#replace' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  other = K2Hash.new(K2HASH_OTHER_FILENAME, 0666, K2Hash::NEWDB)
  other.clear
  other.store('other_key1', 'other_value1')
  other.store('other_key2', 'other_value2')
  other.store('other_key3', 'other_value3')

  k2hash.replace(other)

  assert_false k2hash.has_key?('key1')
  assert_false k2hash.has_key?('key2')
  assert_false k2hash.has_key?('key3')
  assert_true k2hash.has_key?('other_key1')
  assert_true k2hash.has_key?('other_key2')
  assert_true k2hash.has_key?('other_key3')
end

assert 'K2Hash#update' do
  k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
  k2hash.clear
  k2hash.store('key1', 'value1')
  k2hash.store('key2', 'value2')
  k2hash.store('key3', 'value3')

  other = K2Hash.new(K2HASH_OTHER_FILENAME, 0666, K2Hash::NEWDB)
  other.clear
  other.store('key1', 'other_value1')
  other.store('other_key2', 'other_value2')
  other.store('other_key3', 'other_value3')

  k2hash.update(other)

  assert_true k2hash['key1'] = 'other_value1'
  assert_true k2hash['key2'] = 'value2'
  assert_true k2hash['key3'] = 'value3'
  assert_true k2hash['other_key2'] = 'other_value2'
  assert_true k2hash['other_key3'] = 'other_value3'
end
