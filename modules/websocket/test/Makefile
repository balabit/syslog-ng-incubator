.PHONY: test-client-dest test-server

test-server:
	cd ../sample_server/ && make test-server

test-client-dest:
	syslog-ng -Fdve -f ./test_client_dest.conf
