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

begin
  require 'rubygems'
  gem 'nuodb'
  gem 'dbi'
rescue LoadError
end

require 'dbi'
require "nuodb"

module DBI
  module DBD
    #
    # DBD::NuoDB - Database Driver for the NuoDB database system.
    #
    # Requires DBI and the 'nuodb' gem or package to work.
    #
    module NuoDB
      VERSION = "1.0.0"
      DESCRIPTION = "NuoDB DBI DBD, Leverages 'nuodb' low-level driver"

      #
      # See DBI::TypeUtil#convert for more information.
      #
      def self.driver_name
        "NuoDB"
      end

      DBI::TypeUtil.register_conversion(driver_name) do |obj|
        # TODO date and time formatting may not be correct
        newobj = case obj
                 when ::DateTime
                   "'#{obj.strftime("%Y-%m-%d %H:%M:%S")}'"
                 when ::Time
                   "'#{obj.strftime("%H:%M:%S")}'"
                 when ::Date
                   "'#{obj.strftime("%Y-%m-%d")}'"
                 when ::NilClass
                   "NULL"
                 else
                   obj
                 end

        [newobj, false]
      end
    end
  end
end

require 'dbd/nuodb/driver'
require 'dbd/nuodb/database'
require 'dbd/nuodb/statement'
