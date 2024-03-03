#!/bin/perl
use File::Copy;

if ($^O =~ m/Win/)
{
	#There's some sort of bug (4.10.2015) in MinGW Makefile and we can't call LaTeX -> PDF in the Makefile.
	#Workaround is to call .bat script with the make command.
	system("doxygen_to_pdf.bat");
}
else
{
	system("cd documentation/doxygen/latex && make pdf");
}

copy("documentation/doxygen/latex/refman.pdf", "documentation/api.pdf");
