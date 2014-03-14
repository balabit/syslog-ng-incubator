function monitor()
  local result = {}
  local f = assert(io.popen("ps -A -o pid,%cpu,rss,vsize,cmd", 'r'))
  local lines = {}
  for line in f:lines() do
     table.insert(lines, line)
  end 
  f:close()
  local i = 2 
  while i <= #lines do
    local sstart, send = lines[i]:find("[^%s]+")
    result[tostring(i)..".pid"] = lines[i]:sub(sstart, send)
    sstart, send = lines[i]:find("[^%s]+", send + 1)
    result[tostring(i)..".cpu"] = lines[i]:sub(sstart, send)
    sstart, send = lines[i]:find("[^%s]+", send + 1)
    result[tostring(i)..".rss"] = lines[i]:sub(sstart, send)
    sstart, send = lines[i]:find("[^%s]+", send + 1)
    result[tostring(i)..".vsize"] = lines[i]:sub(sstart, send)
    sstart, send = lines[i]:find("[^%s]+", send + 1)
    result[tostring(i)..".cmd"] = lines[i]:sub(sstart)
    i = i + 1 
  end 
  return result
end
