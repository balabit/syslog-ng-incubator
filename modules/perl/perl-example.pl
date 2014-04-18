use Data::Dumper;
use strict;
use warnings;

BEGIN {
	print "perl example loaded\n";
}

END {
	print "perl example ending\n";
}

sub init {
	print "This is the init function.\n";
}

sub queue {
	my ($c) = @_;
	print "QUEUE: " . Dumper($c) . "\n";
}

sub deinit {
	print "This is the deinit function.\n";
}
