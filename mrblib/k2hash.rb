class K2Hash

  def replace(other)
    raise ArgumentError, "The argument is not K2Hash object" unless other.is_a? K2Hash

    self.clear
    other.each_pair do |key, value|
      self[key] = value
    end

    self
  end

  def update(other)
    raise ArgumentError, "The argument is not K2Hash object" unless other.is_a? K2Hash

    other.each_pair do |key, value|
      self[key] = value
    end

    self
  end

  alias length count
  alias size count
end
