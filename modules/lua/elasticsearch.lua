--[[
   elasticsearch.lua -- A simple ElasticSearch destination for syslog-ng

   Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
   Copyright (c) 2014 Viktor Tusa <tusa@balabit.hu>
   Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License version 2 as published
   by the Free Software Foundation, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   As an additional exemption you are allowed to compile & link against the
   OpenSSL libraries as published by the OpenSSL project. See the file
   COPYING for details.
--]]

--
-- Use this table to configure the destination
--
es_batch_size = 100
es_host = "localhost"
es_port = "9200"
es_index = "syslog-ng"
es_type = "message"

function elastic_request(config, ep, req)
   req = req .. "\n"
   local request_len = tostring(#req)

   local result, respcode, respheaders, respstatus = http.request {
      url = "http://" .. es_host .. ":" .. es_port .. ep .. "/_bulk",
      source = ltn12.source.string(req),
      headers = {
         ["Content-Type"] = "application/json",
         ["Content-Length"] = request_len,
         ["Connection"] = "close"
      },
      method = "POST",
      sink = ltn12.sink.table(respbody)
   }
end

function elastic_queue(msg)
   request = request .. '\n{"create":null}\n' .. msg
   msgcount = msgcount + 1

  if (msgcount % es_batch_size) == 0 then
      request_len = tostring(#request)
      -- print ("Sending msgs, bytes=" .. request_len .. "; count="..tostring(msgcount))

      elastic_request(config, "/" .. es_index .. "/" .. es_type,
                      request)
      request = ""
  end

end

function elastic_init()
   http = require 'socket.http'
   msgcount = 1
   request = ""
   http.TIMEOUT = 0
end
