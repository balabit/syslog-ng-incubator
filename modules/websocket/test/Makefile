.PHONY: test-client-dest test-server

test-server:
	cd ../ws_api/ && make test-server

test-client-dest:
	syslog-ng -Fdve -f ./test_client_dest.conf
