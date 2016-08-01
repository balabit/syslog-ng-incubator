.PHONY: test-client-dest test-server

test-server:
	cd ../ws_api/ && make test-server

test-client:
	cd ../ws_api/ && make test-client

test-client-dest:
	syslog-ng -Fdve -f ./test_client_dest.conf

test-server-dest:
	syslog-ng -Fdve -f ./test_server_dest.conf

test-source:
	syslog-ng -Fdve -f ./test_source.conf
