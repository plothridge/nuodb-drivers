require 'rubygems'
require 'active_record'

puts "Connecting to database..."

ActiveRecord::Base.establish_connection(
  :adapter => 'nuodb',
  :database => 'test',
  :username => 'cloud',
  :password => 'user'
)

puts "Create tables..."

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

puts "Create user records..."

@u = User.create do |u|
  u.first_name = "Fred"
  u.last_name = "Flintstone"
  u.email = "fredf@example.com"
  u.user_name = "fred"
  u.admin = true
end

@u.create_addr do |a|
  a.street = "301 Cobblestone Way"
  a.city = "Bedrock"
  a.zip = "00001"
end

puts "Created #{@u}"

@u = User.create do |u|
  u.first_name = "Barney"
  u.last_name = "Rubble"
  u.email = "barney@example.com"
  u.user_name = "barney"
  u.admin = false
end

@u.create_addr do |a|
  a.street = "303 Cobblestone Way"
  a.city = "Bedrock"
  a.zip = "00001"
end

puts "Created #{@u}"

puts "Print user records..."

puts "Found #{User.count} records:"
User.find do |entry|
  puts entry
end

puts "Modify user records..."

User.all.each do |entry|
  entry.first_name = entry.first_name.upcase
  entry.last_name = entry.last_name.upcase
  entry.admin = !entry.admin
  entry.addr.street = entry.addr.street.upcase
  entry.addr.save 
  entry.save
end

puts "Print user records..."

puts "Found #{User.count} records:"
User.find_each do |entry|
  puts entry
end

