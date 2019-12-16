#!/usr/bin/perl
#
# Vulpes (c) 2019 gbMichelle
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

use strict;
use warnings;
use Data::Dumper;
use File::Basename qw( dirname basename );
use File::Spec::Functions qw( catfile );
use List::AllUtils qw( max );
use YAML::XS qw( LoadFile );

use lib dirname(__FILE__); # Include own directory
use CodeGen::TextHelpers qw( trim is_number lpad rpad pad_strings pad_struct_strings as_decimal );
use CodeGen::Signature qw( preprocess_signature yaml_sig_to_c_sig yaml_sig_to_c_initializer );

sub process_enum_member { # Returns a listref containing all the parts needed
                          # for an enum member.
    # Split by + sign while keeping + signs.
    my @enum_values = (split /(\+)/, ($_->{value}) // "");
    # upper case the values if they are references
    @enum_values = map { (is_number $_) ? $_ : uc $_ } @enum_values;

    my @enum = ( (uc $_->{name}), @enum_values );
    # Trim the whitespace that we got from our ugly regex split.
    return [map { trim $_ } @enum];
}

my $types = LoadFile catfile(dirname(__FILE__), "basic_types.yaml");
$types = $types->{types};

sub process_struct_member { # Returns a listref containing all the parts needed
                            # for a struct member.

    # Figure out the size that this element would be in memory.
    my $size = $_->{size} // 1;
    $size = ($types->{$_->{type}} // 1) * $size;

    my $text = [""];
    # Padding should under no circumstance be used for anything.
    # Use the macro to make it inaccessible.
    if ($_->{type} eq "pad") {
        $text = [ "PAD($_->{size})" ];
    # Unknowns should under no circumstance be used for anything.
    # Define as padding.
    } elsif ($_->{name} eq "unknown") {
        $text = [ "UNKNOWN", "($_->{type})" ];
    # Handle arrays.
    } elsif (exists $_->{size}) {
        $text = [ $_->{type}, "$_->{name}\[$_->{size}\]" ]
    # Default.
    } else {
        $text = [ $_->{type}, $_->{name} ];
    }
    return {
        text => $text,
        size => $size
    }
}

##########################
# Main starts here!
##########################

my $filename = 'vulpes/memory/signatures.yaml';

my $file = LoadFile($filename);

unless ( $file ) {
    die "Couldn't load file.";
}
=pod
unless(open OUTPUT, "object.hh"){
    die "Unable to open output file.";
}
=cut

my $output_source_name = $filename;
$output_source_name =~ s/\.\w+$//;
my $output_header_name = "$output_source_name.hpp";
$output_source_name .= ".cpp";
open(OUTPUT_SRC, '>'.$output_source_name);
open(OUTPUT_HEAD, '>'.$output_header_name);

my $name = basename($filename);

my $license_header = qq{/*
 * Vulpes (c) 2019 gbMichelle
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 *
 * Warning, this file was autogenerated. To avoid getting your work overwritten, edit $name instead.
 */

};

print OUTPUT_SRC $license_header, qq{#include <$output_header_name>

};
print OUTPUT_HEAD $license_header, qq{#pragma once

#include <cstdint>
#include <vector>

};


if (exists $file->{constants}) {

    # Check if all constants have the required fields.
    foreach (@{$file->{constants}}) {
        die ("Couldn't find \"name\" in \n"  . Dumper $_) unless exists $_->{name};
        die ("Couldn't find \"value\" in \n" . Dumper $_) unless exists $_->{value};
        die ("Couldn't find \"type\" in \n"  . Dumper $_) unless exists $_->{type};
    }
    # Map all pieces for easy printing.
    my @text_parts = map { [ $_->{type}, @{process_enum_member $_} ] } @{$file->{constants}};

    # Pad strings for allignment
    pad_strings @text_parts;

    # Write them all out.
    foreach (@text_parts) {
        my $size = scalar @{$_};
        print trim (join " ", "const", @{$_}[0..1], "=", @{$_}[2..$size-1]), ";\n";
    }

    print "\n\n";
}

if (exists $file->{enums}) {

    foreach (@{$file->{enums}}){
        # Create a scoped enum.
        print "enum class ", $_->{name};
        # Set the parent class if needed.
        if (exists $_->{parent}) {
            print " : ", $_->{parent};
        }
        # Open brackets.
        print " {\n";

        # Map all pieces for easy printing.
        my @text_parts = map { process_enum_member } @{$_->{fields}};

        # Pad strings for allignment
        pad_strings @text_parts;

        # Write our fields to the file.
        foreach (@text_parts) {
            my $size = scalar @{$_};
            # If size is over 1 print with equals sign, otherwise do not.
            if ($size > 1){
                print "    ", trim (join " ", @{$_}[0], "=", @{$_}[1..$size-1]), ",\n";
            }else{
                print "    ", (trim @{$_}), ",\n"
            }
        }

        # Close brackets.
        print "};\n\n";
    }

    print "\n";
}

if (exists $file->{structs}) {
    print "#pragma pack(push, 1)\n\n";

    foreach (@{$file->{structs}}){
        # Create a struct.
        print "struct ", $_->{name};
        # Set the parent class if needed.
        if (exists $_->{parent}) {
            print " : public ", $_->{parent};
        }
        # Open brackets.
        print " {\n";

        # Map all pieces for easy printing.
        my @text_parts = map { process_struct_member } @{$_->{fields}};

        pad_struct_strings @text_parts;

        # Merge all separate line pieces into single strings.
        my @lines = map {
            {
                text => (join "", (trim (join " ", @{$_->{text}})), ";"),
                size => $_->{size}
            }
        } @text_parts;

        # Get the longest line length for comment allignment.
        my $line_length = max map { length $_->{text} } @lines;
        # Write our fields to the file.
        my $current_offset = 0;
        foreach (@lines) {
            my $string = rpad $_->{text}, $line_length;
            my $offset_string = (sprintf '0x%X', $current_offset);

            print "    $string // $offset_string\n";

            $current_offset += $_->{size};
        }

        if (exists $_->{size}) {
            # Put a warning here if the struct size does not match what we're expecting.
            my $expected_size = as_decimal $_->{size};
            if ($current_offset != $expected_size) {
                print "    // WARNING: Expected $expected_size, but got $current_offset!\n";
            }
        }

        # Close brackets.
        print "};";
        # If a size was given we want to add a compile time assert to see
        # if the struct is correct when the C++ processor gets to it.
        if (exists $_->{size}) {
            print " static_assert(sizeof($_->{name}) == $_->{size});";

        }
        print "\n\n";
    }
    print "\n#pragma pack(pop)\n\n";
}



if (exists $file->{signatures}) {
    print OUTPUT_SRC "#include <hooker/hooker.hpp>\n\n";
    print OUTPUT_SRC "#include <cstdio>\n\n";
    print OUTPUT_SRC "#include <cstdlib>\n\n";

    my @sigs = map { preprocess_signature } @{$file->{signatures}};

    # Signature definitions.
    print OUTPUT_SRC (map { yaml_sig_to_c_sig } @sigs), "\n";
    # Address holding variables.
    print OUTPUT_SRC (map { yaml_sig_to_c_address_var } @sigs), "\n";
    # Getters that apply offsets if needed.
    print OUTPUT_SRC (map { yaml_sig_to_c_getter } @sigs), "\n";

    # Initialization function.
    my $init_name = $name;
    $init_name =~ s/\.\w+$//;
    print OUTPUT_SRC "void init_$init_name\_signatures() {\n";
    print OUTPUT_SRC map { yaml_sig_to_c_initializer } @sigs;
    print OUTPUT_SRC qq{
    std::vector<const char*> crucial_missing;
    std::vector<const char*> non_crucial_missing;

};
    print OUTPUT_SRC map { yaml_sig_to_c_validator } @sigs;
    print OUTPUT_SRC qq{
    if (crucial_missing.size()) {
        printf("Vulpes connot find the following crucial signatures:\\n");
        for (size_t i=0;i<crucial_missing.size();i++) {
            printf("\%s\\n", crucial_missing[i]);
        };
        if (non_crucial_missing.size()) printf("And less importantly, ");
    }
    if (non_crucial_missing.size()) {
        printf("Vulpes connot find the following non-crucial signatures:\\n");
        for (size_t i=0;i<non_crucial_missing.size();i++) {
            printf("\%s\\n", non_crucial_missing[i]);
        };
    }
    if (crucial_missing.size()) {
        printf("\\nSince there is crucial signatures that we cannot find Vulpes cannot continue loading.\\n");
        printf("Please submit your error log to the Github issues page.\\n");
        exit(0);
    }
};

    print OUTPUT_SRC "}\n";

    print OUTPUT_HEAD map { yaml_sig_to_c_getter_header } @sigs;
    print OUTPUT_HEAD "\n";
    print OUTPUT_HEAD "void init_$init_name\_signatures();\n";
}
