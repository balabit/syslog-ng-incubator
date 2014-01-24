function test_init()
	print "Init called!"
	counter = 0
end

function test_queue(msg)
	counter = counter + 1
	print(msg  .. " " .. tostring(counter))
end

function test_queue_raw(msg)
	counter = counter + 1
	print(msg['MESSAGE'] .. " " .. tostring(counter))
end
