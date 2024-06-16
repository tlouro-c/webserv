#!/usr/bin/env perl

my $html_content = "<html>\n<head><title>Sample Page</title></head>\n<body>\n<h1>Hello, from Perl!</h1>\n<p>This is a test using a CGI script made with Perl.</p>\n</body>\n</html>\n";
my $content_length = length $html_content;

print "HTTP/1.1 200 OK\n";
print "Content-length: $content_length\n";
print "Content-type: text/html\n\n";

print "$html_content";
