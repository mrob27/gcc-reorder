$hd = $ENV{"HOME"};

# &help_arg simply returns true iff its argument is one of the aliases for
# "-help"
sub help_arg
{
  my ($a) = @_;
  return ($a &&
    ( ($a eq '-h') || ($a eq '--h') || ($a eq '-help') || ($a eq '--help') )
  );
}

# Quote a string in the manner needed to pass it to a shell command without
# quotes. Example:
# $pq = &quote1($path); system("ls -l $pq");
sub quote1
{
  my($s) = @_;

  $s =~ s/([,?'"&\\(){}\[\]|>< `~!#\$\%^*+:;])/\\$1/g; # "

  return $s;
}

sub find_cksum
{
  my($fatal) = @_;
  my($p, $hd);

  if ($g_cksum_path ne '') {
    return $g_cksum_path;
  }

  $hd = $ENV{"HOME"};
  if ($p = "/usr/bin/cksum", -x $p) {
    $g_cksum_path = $p;
    return $p;
  } elsif ($p = "$hd/shared/proj/bcrc/bcrc", -x $p) {
    $g_cksum_path = $p;
    return $p;
  }

  if ($fatal) {
    die "find_cksum: no #cksum# found. Perhaps use .../proj/bcrc instead?\n";
  }

  return '';
} # End of find.cksum

# Get the CRC of a file using #bcrc# or #cksum#.
sub path_crc
{
  my ($pn) = @_;
  my ($bcrc, $cmd, $rv);

  if (!(-f $pn)) {
    # Not a file
    return -1;
  }

  $bcrc = &find_cksum();
  if ($bcrc eq '') {
    # Cannot run bcrc
    return -3;
  }

  $cmd = $bcrc . " " . &quote1($pn);
  $rv = `$cmd`; chomp $rv;
  if ($rv =~ m/^([0-9]+) /) {
    return $1;
  }
  # Bad result string
  return -2;
} # End of path.crc

# Self-contained CRC32
#   Source: billauer.co.il/blog/2011/05/perl-crc32-crc-xs-module/
#   Notes by the author:
#
#   In the following snippet:
#
#     #!/usr/bin/perl
#     use warnings;
#     use strict;
#     require String::CRC32;
#     require Digest::CRC;
#     my $str = "This is just any string.";
#     my $refcrc = String::CRC32::crc32($str);
#     my $refcrc2 = Digest::CRC::crc32($str);
#     my $mycrc = mycrc32($str);
#
#   All three results ($refcrc, $refcrc2, and $mycrc) will be the same.
#
#   For any string $str, the following code:
#
#     my $append = mycrc32($str) ^ 0xffffffff;
#     my $res = mycrc32($str.pack("V", $append));
#
#   yields a CRC in $res which equals Oxffffffff.
#
#   Little-endian is assumed.
#
# To convert the output to a hexadecimal string, use sprintf.
$g_crc32_inited = 0;
$g_crc32_poly = 0xedb88320;
sub crc32_string
{
  my ($input, $init_value) = @_;

  $init_value = 0 unless (defined $init_value);

  if (!($g_crc32_inited)) {
    @crc32_lookup_table = ();
    for (my $i=0; $i<256; $i++) {
      my $x = $i;
      for (my $j=0; $j<8; $j++) {
        if ($x & 1) {
          $x = ($x >> 1) ^ $g_crc32_poly;
        } else {
          $x = $x >> 1;
        }
      }
      push @crc32_lookup_table, $x;
    }
    $g_crc32_inited = 1;
  }
  my $crc = $init_value ^ 0xffffffff;

  foreach my $x (unpack ('C*', $input)) {
    $crc = (($crc>>8) & 0xffffff) ^ $crc32_lookup_table[ ($crc ^ $x) & 0xff ];
  }

  $crc = $crc ^ 0xffffffff;

  return $crc;
} # End of crc32.string

# Given a value (a number, interpreted as a Unicode "codepoint"), return
# a string containing the UTF-8 encoding of that value.
#
# The opposite conversion is performed by &utf8_decode in ~/bin/bcv8to7
sub utf8_encode
{
  my($x, $hex) = @_;
  my($b1, $b2, $b3, $b4, $b5, $b6);
  my($u, $hs, $c, $uhx);

  $u = ''; $hs = '';
  if ($x <= 0x7f) {
    $hs = $x;
    $u = chr($x);
  } elsif ($x <= 0x7ff) {
    $b2 = $x & 0x3f; $x >>= 6;
    $b1 = $x;
    $hs = (0xc0 + $b1) . "," . (0x80 + $b2);
    $u = chr(0xc0 + $b1) . chr(0x80 + $b2);
  } elsif ($x <= 0xffff) {
    $b3 = $x & 0x3f; $x >>= 6;
    $b2 = $x & 0x3f; $x >>= 6;
    $b1 = $x;
    $hs = (0 + 0xe0 + $b1) . "," . (0 + 0x80 + $b2) . "," . (0 + 0x80 + $b3);
    $u = chr(0xe0 + $b1) . chr(0x80 + $b2) . chr(0x80 + $b3);
  } elsif ($x <= 0x1fffff) {
    $b4 = $x & 0x3f; $x >>= 6;
    $b3 = $x & 0x3f; $x >>= 6;
    $b2 = $x & 0x3f; $x >>= 6;
    $b1 = $x;
    $hs = (0 + 0xf0 + $b1) . "," . (0 + 0x80 + $b2) . "," . (0 + 0x80 + $b3)
                           . "," . (0 + 0x80 + $b4);
    $u = chr(0xf0 + $b1) . chr(0x80 + $b2) . chr(0x80 + $b3) . chr(0x80 + $b4);
  }
  $uhx = '';
  foreach $c (split(/,/, $hs)) {
    if ($hex == 8) {
      # octal
      $uhx .= sprintf(" %03o", $c);
    } else {
      $uhx .= sprintf(" %02X", $c);
    }
  }
# print "encode $hex $x: hs [$hs] uhx $uhx\n";
  if ($hex) {
    return $uhx;
  }
  return $u;
} # End of utf8.encode

# Given an 8-bit integer value, return the UTF8-encoded   0 4     0 3
# Braille character with the dots numbered in the         1 5 --\ 1 4
# 'logical' way depicted here (left figure).              2 6 --/ 2 5
# (This is NOT the dot ordering as defined in Unicode)    3 7     6 7
# Source: en.wikipedia.org/wiki/Braille_Patterns
sub braille_encode
{
  my($x) = @_;
  my($cp, $enc);

      # 012..7->012..7        456->345              3->6
  $cp = ($x & 0x87)   + (($x & 0x70) >> 1) + (($x & 0x08) << 3);
  $enc = &utf8_encode($cp + 0x2800);
  return $enc;
} # End of braille.encode

sub a2b_condense
{
  my (@in) = @_;
  my ($i, $gg, $base, $inrun, $len, $ls, $thr);
  my($dbg1, $OUT, $l);
  $dbg1 = $ENV{'HOME'} . "/tmp/a2b-debug.txt";
  $l = $#in;
  open($OUT, ">> $dbg1"); print $OUT "initial (len $l)\n";
  foreach $l (@in) { print $OUT "$l\n"; }
  print $OUT (("=" x 78) . "\n"); close $OUT;

  $gg = 1; $thr = 3;
  while ($gg) {
    $base = -1; $inrun = $len = $ls = 0;
    for ($i=0; $i<($#in); $i++) {
      if ($in[$i] =~ m/^ *$/) {
        $ls++;
      } else {
        $ls = 0;
      }
      if ($ls >= $thr+3) {
        # We are in a shrinkable run
        if ($base < 0) {
          # Beginning of the first shrinkable run
          $base = $i + 1 - $ls;
          $len = $i + 1 - $base;
          $inrun = 1;
        }
      }
      if (($base >= 0) && $inrun) {
        if ($ls > 0) {
          # We are still within the first shrinkable run
          $len = $i + 1 - $base;
        } else {
          $inrun = 0;
        }
      }
    }
    # Done scanning array, see if we found a shrinkable run
    $len -= $thr;
    if (($base > 0) && ($len > 0)) {
      splice @in, $base, $len;
  $l = $#in;
  open($OUT, ">> $dbg1"); print $OUT "splice in $base $len : len now $l :\n";
  foreach $l (@in) { print $OUT "$l\n"; }
  print $OUT (("=" x 78) . "\n"); close $OUT;
    } else {
      $gg = 0;
  $l = $#in;
  open($OUT, ">> $dbg1"); print $OUT "finished, len is $l\n";
  print $OUT (("=" x 78) . "\n"); close $OUT;
    }
  }  
  return ((@in));
} # End of a2b.condense

# Convert ASCII graphics to Braille-dot graphics. The first argument
# is a set of options separated by whitespace, the rest are an array of
# ASCII strings. Each non-blank character will become a single dot in
# Unicode Braille
#   Options marked %%% are not implemented (but planned):
#    cond  - condense multiple blank rows (of input) into one row;
#            this is done before rotation and pixel-conversion
# (%%rot0  - No rotation, horizontal lines in input remain horizontal)
#    rot1  - First line of ASCII text becomes leftmost column of Braille dots
#            (great for custom-generated graphs and histograms)
# (%%2px   - Input is "2-pixel encoded": characters [~'"] and [,._]
#            represent single "on" pixels, all other nonblank characters
#            represent two "on" pixels)
#    trim  - Remove trailing blanks from output lines
sub a2b_graphix
{
  my(@in) = @_;
  my($opts, $o);
  my($l, $in_rows, $in_col);
  my($i, $j, $is, $js, $bi, $out_rows, $out_col);
  my(@out);
  my(@bits);
  my($bbl, $o_trim, $o_cond);

  $opts = shift @in;
  $opts =~ s/\t/ /g; $opts =~ s/^ +//; $opts =~ s/ +$//; $opts =~ s/ +/ /g;
  $o_cond = $o_trim = 0;
  foreach $o (split(" ", $opts)) {
    if ($o eq 'cond') {
      $o_cond = 1;
    } elsif ($o eq 'rot1') {
      # ok
    } elsif ($o eq 'trim') {
      $o_trim = 1;
    } else {
      die "a2b.graphix: Unrecognised option '$o'\n";
    }
  }
  # Condense rows vertically
  if ($o_cond) {
    @in = &a2b_condense(@in);
  }
  # Count rows and columns
  $in_rows = $#in + 1;
  $in_col = 0;
  for ($i=0; $i<$in_rows; $i++) {
    $in[$i] =~ s/[^ -~]/ /g;
    if (length($in[$i]) > $in_col) {
      $in_col = length($in[$i]);
    }
  }
  # print "$in_rows rows, $in_col columns";
  $in_rows = ($in_rows + 1) & 0xfffe;
  $in_col = ($in_col + 3) & 0xfffc;
  # print "  -> $in_rows, $in_col\n";
  $out_rows = ($in_col / 4);
  $out_col = ($in_rows / 2);
  for($i=0; $i<$in_rows; $i++) {
    for($j=0; $j<$in_col; $j++) {
      $bi = $out_rows - ($j>>2) - 1;
      $is = $i & 1; $js = $j & 3;
      if (($is+$js) == 0) { $bits[$bi] = 0; }
      $t = $in[$i]; "$t " =~ m/^(.)(.*)$/; $in[$i]=$2; $t=$1;
      # print "$i $j $t\n";
      if ($t ne ' ') {
        # $t=1<<((3-$js)+4*$is); print "is=$is js=$js t=$t\n";
        $bits[$bi] = $bits[$bi] | (1<<((3-$js)+4*$is));
      }
      # print "'$t' bits[$bi]:$bits[$bi]\n";
      if ($is+$js == 4) { $out[$bi] .= &braille_encode($bits[$bi]); }
    }
  }
  if ($o_trim) {
    # Trim trailing blanks on all lines
    $bbl = &braille_encode(0);
    for($i=0; $i<=$#out; $i++) {
      $t = $out[$i];
      while($t =~ m/^(.+)$bbl$/) {
        $t = $1;
      }
      $out[$i] = $t;
    }
  }
  $g_a2b_out_col = $out_col;
  return(@out);
} # End of a2b.graphix

sub need_dir
{
  my($dir) = @_;
  my($d1, $d2, $t1, $t2);

  if (("/$dir/" =~ m|/\./|) || ("/$dir/" =~ m|/\.\./|)) {
    die "ERR11: need.dir request cannot contain ./ or ../\n";
  }

  if ($dir eq '') {
    die "need.dir illegal request to create '$dir'\n";
  } elsif (-d $dir) {
    # all set
  } elsif (-f $dir) {
    die "need.dir '$dir' is already a file\n";
  } elsif (-e $dir) {
    die "need.dir Can't create directory $dir, something (not a directory or file) is already there\n";
  } else {
    if ($dir =~ m|^(.+)/[^/]+$|) {
       $d1 = $1;
       &need_dir($d1);
    }
    print "mkdir '$dir'\n";
    system("mkdir $dir");
  }
} # End of need.dir

sub date3
{
  my($tm, $numeric) = @_;
  my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
  my($l, $fmt);

  ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($tm);
  if (($tm > 1.0e8) && ($year < 71)) {
    # This shouldn't happen: 19701231 is in the 31 million range
    # In early Perls, localtime has a bug: calling localtime(5000000000)
    # gives 19691231 because it treats the input as -1.
    ($sec,$min,$hour,$mday,$mon,$year) = locltime($tm, 1);
  }
  # YYYYMMDD.HH:MM:SS
  $fmt = "%04d%02d%02d.%02d:%02d:%02d";
  $l = sprintf($fmt, 1900 + $year, $mon+1, $mday, $hour, $min, $sec);

  return $l;
} # End of date3

# Print a prompt and wait for a Y or N answer.
sub yorn
{
  my($prompt) = @_;
  my($ans);

  $ans = "";
  while ($ans eq "") {
    print $prompt;
    $ans = <>; chomp $ans;
    $ans =~ tr/A-Z/a-z/;
    if ($ans =~ m/^y/) {
      return 1;
    } elsif ($ans =~ m/^n/) {
      return 0;
    }
    print "Please answer Y or N.\n";
    $ans = "";
  }
} # End of y.orn
