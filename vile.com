$! Runs VILE from the current directory (used for testing)
$ vile :== $"sys$disk:''f$directory()'vile.exe"
$ define/user_mode sys$input  sys$command
$ define/user_mode sys$output sys$command
$ vile 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8
$!+
$! $Log: vile.com,v $
$! Revision 1.0  1993/03/25 19:47:58  pgf
$! Initial revision
$!
$!-
