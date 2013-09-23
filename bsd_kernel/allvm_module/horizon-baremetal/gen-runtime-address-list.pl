#
# Horizon bytecode compiler
# Copyright (C) 2010 James Molloy
#
# Horizon is open source software, released under the terms of the Non-Profit
# Open Software License 3.0. You should have received a copy of the
# licensing information along with the source code distribution. If you
# have not received a copy of the license, please refer to the Horizon
# project website.
#
# Please note that if you modify this file, the license requires you to
# ADD your name to the list of contributors. This boilerplate is not the
# license itself; please refer to the copy of the license you have received
# for complete terms.
#

#
# gen-runtime-address-list.pl: Greps the runtime/ directory for functions, and adds
#                              them to a list.
#

use strict;
use warnings;

die("No output file specified.") unless defined $ARGV[0];
open my $outfd, ">", $ARGV[0] or die("Output file $ARGV[0] not available for writing.");

my @cclist = <horizon/Runtime/*.cc>;
my @arr;

foreach my $cc (@cclist) {
    open my $fd, "<", $cc or die("File $cc not available for reading.");

    while(<$fd>) {
        if(m/^[ \t]*extern[ \t]+"C"[ \t]+[^ ]+[ \t]+(horizon_.*?)[ (]/) {
            push @arr, $1;
        }
    }
}

print $outfd "/* Horizon bytecode compiler\n";
print $outfd " * Copyright (C) 2010 James Molloy\n";
print $outfd " *\n";
print $outfd " * Horizon is open source software, released under the terms of the Non-Profit\n";
print $outfd " * Open Software License 3.0. You should have received a copy of the\n";
print $outfd " * licensing information along with the source code distribution. If you\n";
print $outfd " * have not received a copy of the license, please refer to the Horizon\n";
print $outfd " * project website.\n";
print $outfd " *\n";
print $outfd " * Please note that if you modify this file, the license requires you to\n";
print $outfd " * ADD your name to the list of contributors. This boilerplate is not the\n";
print $outfd " * license itself; please refer to the copy of the license you have received\n";
print $outfd " * for complete terms.\n";
print $outfd " */\n";
print $outfd "\n";

print $outfd "/* AUTOGENERATED BY gen-runtime-address-list.pl. DO NOT EDIT! */\n";
print $outfd "/* This file is intended to be #included from dynamicLookup.cc . Expect errors if you #include it elsewhere. */\n";
print $outfd "\n";

print $outfd "#ifdef GEN_RUNTIME_ADDRESS_LIST\n";
print $outfd "# error Generated header file #included multiple times. Something is wrong.\n";
print $outfd "#endif\n";
print $outfd "#define GEN_RUNTIME_ADDRESS_LIST\n";
print $outfd "\n";

print $outfd "#include <map>\n";
print $outfd "#include <string>\n";
print $outfd "\n";

print $outfd "static std::map<std::string, void*> s_runtime_addresses;\n";
print $outfd "\n";

foreach my $x (@arr) {
    print $outfd "extern \"C\" void $x(void);\n";
}
print $outfd "\n";

print $outfd "/** Global initialisation function, to be called once. */\n";
print $outfd "void InitialiseRuntimeAddresses() {\n";

foreach my $x (@arr) {
    print $outfd "\ts_runtime_addresses[std::string(\"$x\")] \t= (void*)&$x;\n";
}

print $outfd "}\n";

exit 0;