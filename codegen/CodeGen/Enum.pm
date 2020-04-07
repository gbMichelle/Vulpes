#
# Vulpes (c) 2020 gbMichelle
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

use strict;
use warnings;
use List::Util qw{ max };

sub preprocess_enum_option {
    $_->{uc_name} = uc $_->{name};
    # Replace all series of spaces with single underscores
    $_->{uc_name} =~ s/ +/_/;

    return $_;
}

sub preprocess_enum {
    $_->{options} = [map { preprocess_enum_option } @{$_->{options}}];

    # Make it so each value that isn't given is the previous one incremented.
    my $i = 0;
    foreach my $opt (@{$_->{options}}) {
        $opt->{value} //= $i;
        $i = $opt->{value} + 1;
    };

    return $_;
}

sub yaml_enum_to_cpp_definition {
    # Start definition.
    my $string = "enum class $_->{name}";

    # Optionally specify a base type
    if (exists $_->{type}) {
        $string .= " : $_->{type}";
    }

    # Start main part.
    $string .= " {\n";

    # Get the length for allignment.
    my $max_opt_len = max (map { length ($_->{uc_name}) } @{$_->{options}});

    # Turn each option into a line with allignment.
    my @options = map {
        sprintf "    %- ".$max_opt_len."s = %d,", $_->{uc_name}, $_->{value}
    } @{$_->{options}};

    # Close enum.
    $string .= join "\n", @options, "};\n";

    return $string;
}

sub yaml_enums_to_cpp_definitions {
    my ($name, $enums) = @_;
    my @enums = map { preprocess_enum } @{$enums};

    print map {yaml_enum_to_cpp_definition} @enums;
}

1;