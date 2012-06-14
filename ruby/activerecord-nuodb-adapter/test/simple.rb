#
# Copyright (c) 2012, NuoDB, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of NuoDB, Inc. nor the names of its contributors may
#       be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL NUODB, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

require 'rubygems'
require 'active_record'

class User < ActiveRecord::Base
  has_one :addr, :class_name => 'Addr'
  def to_s
    return "User(#{id}), Username: #{user_name}, Name: #{first_name} #{last_name}, #{admin ? "admin" : "member"}\n" +
      "  Address: #{addr}\n"
  end
end

class Addr < ActiveRecord::Base
  belongs_to :User
  def to_s
    return "Addr(#{id}:#{user_id}) Street: #{street} City: #{city} Zip: #{zip}"
  end
end

class NuoSimpleTest < Test::Unit::TestCase

  def setup()

    ActiveRecord::Base.establish_connection(
                                            :adapter => 'nuodb',
                                            :database => 'test',
                                            :username => 'cloud',
                                            :password => 'user'
                                            )

    ActiveRecord::Schema.drop_table(User.table_name) rescue nil

    ActiveRecord::Schema.drop_table(Addr.table_name) rescue nil

    ActiveRecord::Schema.define do
      create_table User.table_name do |t|
        t.string :first_name, :limit => 20
        t.string :last_name, :limit => 20
        t.string :email, :limit => 20
        t.string :user_name, :limit => 20
        t.boolean :admin
      end
      create_table Addr.table_name do |t|
        t.integer :user_id
        t.string :street, :limit => 20
        t.string :city, :limit => 20
        t.string :zip, :limit => 6
      end
    end

  end


  def test_create_user_records

    u = User.create do |u|
      u.first_name = "Fred"
      u.last_name = "Flintstone"
      u.email = "fredf@example.com"
      u.user_name = "fred"
      u.admin = true
    end

    assert_not_nil u
    assert_not_nil u.id

    fred_id = u.id

    u.create_addr do |a|
      a.street = "301 Cobblestone Way"
      a.city = "Bedrock"
      a.zip = "00001"
    end

    assert_not_nil u

    u = User.create do |u|
      u.first_name = "Barney"
      u.last_name = "Rubble"
      u.email = "barney@example.com"
      u.user_name = "barney"
      u.admin = false
    end

    barney_id = u.id

    u.create_addr do |a|
      a.street = "303 Cobblestone Way"
      a.city = "Bedrock"
      a.zip = "00001"
    end

    assert_not_nil u

    assert_equal 2, User.count

    User.find do |entry|
      case entry.id 
      when fred_id
        assert_equal 'Fred', entry.first_name
        assert_equal 'Flintstone', entry.last_name
        assert_equal '301 Cobblestone Way', entry.addr.street
        nil
      when barney_id
        assert_equal 'Barney', entry.first_name
        assert_equal 'Rubble', entry.last_name
        assert_equal '303 Cobblestone Way', entry.addr.street
        nil
      else
        raise 'unknown entry.id'
      end
    end

    User.all.each do |entry|
      entry.first_name = entry.first_name.upcase
      entry.last_name = entry.last_name.upcase
      entry.admin = !entry.admin
      entry.addr.street = entry.addr.street.upcase
      entry.addr.save 
      entry.save
    end

    assert_equal 2, User.count

    User.find do |entry|
      case entry.id 
      when fred_id
        assert_equal 'FRED', entry.first_name
        assert_equal '301 COBBLESTONE WAY', entry.addr.street
        nil
      when barney_id
        assert_equal 'BARNEY', entry.first_name
        assert_equal '303 COBBLESTONE WAY', entry.addr.street
        nil
      else
        raise 'unknown entry.id'
      end
    end

  end

end
