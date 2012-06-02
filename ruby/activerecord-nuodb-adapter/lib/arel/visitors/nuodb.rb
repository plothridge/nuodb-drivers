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

require 'arel'

module Arel

  module Visitors
    class NuoDB < Arel::Visitors::ToSql

      private

      def visit_Arel_Nodes_Offset(o)
        "WHERE [__rnt].[__rn] > (#{visit o.expr})"
      end

      def visit_Arel_Nodes_Limit(o)
        "TOP (#{visit o.expr})"
      end

      def visit_Arel_Nodes_Lock(o)
        visit o.expr
      end
      
      def visit_Arel_Nodes_Ordering(o)
        if o.respond_to?(:direction)
          "#{visit o.expr} #{o.ascending? ? 'ASC' : 'DESC'}"
        else
          visit o.expr
        end
      end
      
      def visit_Arel_Nodes_Bin(o)
        "#{visit o.expr} #{@connection.cs_equality_operator}"
      end

    end
  end

end

Arel::Visitors::VISITORS['nuodb'] = Arel::Visitors::NuoDB
