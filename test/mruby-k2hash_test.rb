class MrubyK2hashTest < MTest::Unit::TestCase
  K2HASH_FILENAME = '/tmp/mtest.k2hash'

  def test_clear
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.store('key1', 'value1')
    k2hash.store('key2', 'value2')
    k2hash.store('key3', 'value3')

    k2hash.clear

    assert_equal k2hash.size, 0
  end

  def test_closed
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.close

    assert_true k2hash.closed?
  end

  def test_delete
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.clear
    k2hash.store('key1', 'value1')
    k2hash.delete('key1')

    assert_nil k2hash.fetch('key1')
  end

  def test_delete_if
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

  def test_fetch_store
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.clear
    k2hash.store('key1', 'value1')

    assert_equal k2hash.fetch('key1'), 'value1'
    assert_nil k2hash.fetch('key100')

    k2hash['key2'] = 'value2'

    assert_equal k2hash['key2'], 'value2'
    assert_nil k2hash['key200']
  end

  def test_each
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

  def test_each_key
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.clear
    k2hash.store('key1', 'value1')
    k2hash.store('key2', 'value2')
    k2hash.store('key3', 'value3')

    k2hash.each_key do |k|
      assert_true k == 'key1' || k == 'key2' || k == 'key3'
    end
  end

  def test_each_value
    k2hash = K2Hash.new(K2HASH_FILENAME, 0666, K2Hash::NEWDB)
    k2hash.clear
    k2hash.store('key1', 'value1')
    k2hash.store('key2', 'value2')
    k2hash.store('key3', 'value3')

    k2hash.each_value do |v|
      assert_true v == 'value1' || v == 'value2' || v == 'value3'
    end
  end

  def test_has_key
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

  def test_keys
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

  def test_values
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


  #
  # Implemented by Enumerable
  #

  def test_to_a
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

  def test_to_hash
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

  def test_select
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

end

MTest::Unit.new.run
