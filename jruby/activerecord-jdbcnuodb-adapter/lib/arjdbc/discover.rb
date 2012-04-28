module ::ArJdbc
  extension :NuoDB do |name|
    if name =~ /nuodb/i
      require 'arjdbc/nuodb'
      true
    end
  end
end
