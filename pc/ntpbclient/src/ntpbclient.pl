#!/usr/bin/perl
#
# $Id$
#
# Copyright (C) 2010 Mathias Lafeldt <misfire@debugon.org>
#
# This file is part of PS2rd, the PS2 remote debugger.
#
# PS2rd is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PS2rd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PS2rd.  If not, see <http://www.gnu.org/licenses/>.
#

use warnings;
use strict;
use IO::Socket;

my ($remote_host, $remote_port) = ("localhost", 4234);

my $NTPB_MAGIC = "\xff\x00NTPB";
my ($NTPB_DUMP, $NTPB_HALT, $NTPB_RESUME) = (0x0100, 0x0201, 0x0202);
my ($NTPB_SEND_DUMP, $NTPB_EOT, $NTPB_ACK) = (0x0300, 0xffff, 0x0001);

my $sock = IO::Socket::INET->new(
    PeerAddr => $remote_host,
    PeerPort => $remote_port,
    Proto => "tcp",
    Timeout => 3
) or die "Could not create socket: $!\n";

sub ntpb_send_cmd {
    my $cmd = shift;
    my $buf = shift || "";
    $sock->send($NTPB_MAGIC . pack("S2", length $buf, $cmd) . $buf) or return undef;
    my $ret = $sock->recv($buf, 65536);
    # TODO check magic
}

sub ntpb_recv_data {
    # TODO adapt for dump command
    my ($buf, $size);
    my $ret = $sock->recv($buf, 65536);
    unless (defined $ret) { return undef; }
    $sock->send($NTPB_MAGIC . pack("S3", $size, $NTPB_EOT, $NTPB_ACK));
}

my $cmd = shift;
unless (defined $cmd) { die "Command missing.\n" };

if ($cmd eq "halt") {
    ntpb_send_cmd($NTPB_HALT);
} elsif ($cmd eq "resume") {
    ntpb_send_cmd($NTPB_RESUME);
} else {
    die "Invalid command.\n";
}
ntpb_recv_data();

close($sock);
