The following changes are grouped only roughly, and represent the delta
of the current release from vile version three.  -pgf   10/22/91

------------------------------------------------------

VI COMPLIANCE:
===============

- made ^N, ^P, and ^T do exactly what they do in vi.  (next line,
	previous-line, and tag-pop) Use ^X-o to switch windows, and ^X-2 to
	split them.

- ANSI style arrow keys now work from either command or insert mode.  Any
	ANSI function key of the form ESC [ c can be used as a command
	key -- the binding is FN-c

- the '#' key may now be used to force the following character to be treated
	as a function key.  Thus #1 is now equivalent to FN-1.

- some modes/values have changed names: aindent is now autoindent, swrap is
	now wrapscan, exact is now noignorecase, and they all have the vi
	shorthands (e.g.  tabstop is the same as ts, ic is the same as
	ignorecase).

- the "paragraphs" and "sections" values are now implemented, but as
	regular expressions -- they look pretty ugly, but I think they
	do what vi does.

- the sentence motions ')' and '(' now work.  they may not do exactly what
	vi does, but they're close.  they're controlled by regular
	expressions that find the ends of sentences -- then they move
	forward to the start of the next one.  The regular expressions are
	set in the new "sentences" value setting.

- "showmatch" mode now works, and momentarily shows the previous matching '('
	or '{'.

- "magic" mode, or rather "nomagic" mode, now works more like vi:  special
	characters are still available by escaping them in nomagic mode.

- there is a new mode, settable as "set terse" and "set noterse", which will
	suppress some of the extra messages that vile prints, reducing
	traffic for slow terminals.

- the '%' command now scans forward for a fence character if you're not on
	one to begin with (thanks to Dave Lemke).  I also added '^X-%', which
	scans backward instead.

- the ":&" command now works, and repeats the last ":s" or '^X-s'
	substitution.  it re-uses trailing p and l options, which vi does
	not.

- the '&' now works, and repeats the last ":s" or '^X-s' substitution.

- there is a new operator, '^A-&', which is similar to '&' but acts on a
	region.  I expect that if you want it, you'll probably rebind it to
	something else (like '&') since typing it is awkward, to say the least.

- ^W and ^U are now both read from the tty settings, and they work in both
	insert mode and on the : line.  There is a "backspacelimit" setting
	that controls whether these (and regular backspacing (^H or DEL)
	will backspace past the insert point in insert mode.

- The '@' command can now be used to execute the contents of a named register,
	as if it were entered at the keyboard.  Thus @k executes register k.

- The "keyboard macro" obtained with ^X-( and ^X-) can be saved into a named
	register with the ^X-^ command.  This lets you record a keyboard
	macro and save it for use with the new '@' command.

- And, along to further complement the '@' command, it is possible to pre-load
	the named registers with the "load-register" command.  In a .vilerc
	you can say:
		use-register a load-register ihello^[
	(where ^[ is really an ESC entered with ^V-ESC), and then from
	within vile you can execute '@a', and the word "hello" will be
	inserted.

- the H, M, and L commands are now absolute motions

- insert mode now takes a repeat count, so '80i=ESC' inserts 80 '=' chars

- two new functions, to make normal motions more vi compliant --
	forward-character-to-eol and backward-character-to-bol, which won't
	move past the boundaries of the current line.

- you can no longer backspace from the beginning of a line to the end of
	the previous.  (rebind 'h' and '^h' from "backward-character-to-bol"
	to "backward-character"

- a motion caused by an undo is considered absolute, and the "last dot"
	mark is reset.

- added 'vi +/searchstring file.c' invocation syntax

- you can now specify a count to the ~ command.

- backslashes now have a protective effect on the : line -- they guard
	against expansion of #, %, :, and \.

- a lone ":f" without filename now gives same info as ^G

- 'x', 's', 'r' will no longer delete a line if used on an empty line.

- after a ":e!" command, the buffer name changes to match the filename, as
	if this were a new file read.  It used to just suck a new file into
	an old buffer, and the names didn't match.

- there is now a "taglength" setting, and the name of the tags file is in
	the "tags" setting.

- the "cd" and "pwd" commands now work

- shiftwidth has been implemented, so ^T, ^D, '<', and '>' all do pretty
	much what they do in vi


MISCELLANEOUS
===============

- reading files in general is _much_ faster, unless the file contains lines
	longer than 255 characters, in which case things slow down again.

- regular expressions are more powerful and much, much faster than they
	were.  vile now uses Henry Spencer's regexp code.  This has the
	side effect of making '\|' a new metacharacter, for alternation
	of expressions.  '\+' and '\?' are also new, and stand for "one
	or more" and "zero or one" occurrences.

- in addition, \< and \> find beginnings and ends of words, and \w, \s,
	and \d find "word" characters, "space" characters, and digits.
	\W, \S, and \D find the converse

- related to the above change, replacement metacharacters now work, so you
	can use '&' and '\1' through '\9' in the replacement string, and
	you'll get the right thing.

- Dave Lemke contributed code to support vile under X windows -- including
	support for cutting/pasting, etc.  Thanks Dave!

- the '%' command will now find matching #if/#else/#endif sets.

- the '%' command will now find matching C comment sets.  Yes, it does
	the (non-)nesting correctly.

- tabstops can now be set to any value, and there is a separate value for
	tabstops in C mode.  (c-tabstop)

- there is now minimal support for horizontal scrolling -- the code for
	keeping the cursor on lines that are too short to be visible at all
	is still broken.

- the ! filter command for running text through an external filter now
	works

- quite a bit of code cleanup, at least I think so -- it has ported pretty
	easily to a R6000, linux, 386bsd, Sony NeWS, NeXT, and the UNIXPC. 
	it's close to lint-free, and Saber has very few problems with the
	code.  it'll now use prototypes if __STDC__ is defined, and it
	passes gcc -Wall -Wshadow with no warnings, if you ignore implicit
	declarations of system calls and lib routines.  the code should now
	be more portable, to various compilers and systems.  there are new
	targets in the makefile to make building on various platforms
	easier.  the makefile even supports microsoft C.

- the code now builds and runs under DOS, using (at least) Microsoft C 6.0. 
	Peter Ruczynski has done a great job porting vile to DOS -- he
	calls that version "pcvile".  He has used the Zortech compiler in
	the past -- I didn't try to break that, but you never know.

- quite a few bug fixes, including all (I think) of those reported by users

- the "last dot" mark, accessed with the names ' and ` doesn't get reset
	needlessly as often -- that is, if an absolute motion results in no
	move (e.g.  a failed search or tags operation, or an undo) then the
	"last dot" mark is unchanged.

- the "source" ("execfile") command now does globbing on the file being run
	-- be sure to put it in quotes, as in :source "$HOME/.vilestuff"

- entab, detab, and line-trim are now all operators (^A-tab, ^A-space, and
	^A-t)

- "make install" installs to /usr/local/bin or $HOME/bin, whichever is
	writable

- the english names of functions, particularly new ones, have been
	"rationalized", hopefully making it easier to remember their names.

- if you invoke a substitute command with ^X-s, it will always act globally
	across lines.  (i.e.  it behaves like "s/s1/s2/g", not like
	"s/s1/s2/")

- a QUIT signal will no longer cause core dump unless built with DEBUG on

- the display optimizes scrolling if possible when the window scroll
	commands ^E and ^Y are used.

- values/modes have been reworked.  Some are now attached to buffers, some
	to windows.  By default, all values use the global settings of
	those values, and will track changes to the global settings.  If a
	local setting is specified, it breaks the link between the local
	and global value, and the local value will no longer track the
	global value.  ":set all" is now more informative.

- the "set list" value is now attached to a window, not a buffer, so a
	single buffer can be displayed in two windows, "list"ed in one but
	not the other.

- reading from other processes into a buffer is now _much_ faster

- it is no longer legal to write out the contents of a buffer that has view
	mode set.

- the format region command (^A-f) now restarts with each fresh paragraph,
	so you can format an entire file at once, without collapsing it all
	into a single paragraph.  It also knows a little bit about C and
	shell comments, so you can now reformat blocks of commentary text.
	(There can't be any non-commentary stuff in front of the comments.)

- the position of the cursor within a window (the framing) is now preserved
	while the window if "off-screen" -- it no longer reframes when made
	visible again.

- there is now a mode indicator on the modeline: I for insert, O for
	overwrite, and R for replace-char.

- a line starting with '#' won't shift-right if the buffer is in C mode.

- :s/s1/s2/5 now works, to change the 5th occurrence on a line

- there are new commands to write all buffers: :ww is a synonym for
	"write-changed-buffers", and :wwq is now a synonym for
	"write-changed-buffers-and-quit"

- it is now legal to "kill-buffer" a displayed buffer

- there are new variables, $word, $pathname, and $identifier, which return
	the appropriate type of string from the cursor's location

- all of the variables, and the functions and directives of the extension
	language have been documented, and most have been tried, if not
	tested

- two new functions: &rd and &wr, return whether a file is readable or writable

- a new mode, "tabinsert", has been created to fill the needs of the 
	"I hate tabs" camp.

- filenames are now put into canonical form -- this keeps "junk", "./junk",
	"./somedir/../junk" from appearing as different files to the editor.

- there is a new mode, "tagsrelative", which, when set, causes filenames
	looked up via tags to have the directory name of the tags file
	prepended to them before being accessed.  This allows one to cd into
	a build tree from within the editor, to facilitate a grep or a local
	build, and have tags lookups continue to work.

- the find-next-error code now honors the "Entering/Leaving directory"
	messages that GNU make (and others?) puts out.

- job control now works right

IMPLEMENTORS NOTES:
===================

- internally, most pointers into the buffer, consisting of a LINE * and an
	int offset are now represented with a MARK structure -- in the
	nineties, it's time to start using structure assignment.

- the set of values that are frequently handed about between buffers and
	windows, or windows and their "children" are now grouped into
	B_TRAITS and W_TRAITS structures, for easier bookkeeping.  There
	are lots of macros to hide the added data structure depth.

- if you add a new setting that should be inherited by and selectable
	on a per-buffer or per-window basis, add it to the W_VALUES or
	B_VALUES structs.  Otherwise, if its just a global value, like the
	X font, make it a variable.  I intend to merge the setting of
	variables in with the other values one of these days.

- there is now a generic printf facility built in, which is layered under
	mlwrite() for the message line, bprintf() to print into a buffer,
	and lsprintf() to print into a string

- LINE structures can now have NULL l_text pointers -- empty lines don't
	have memory associated with them (except for the LINE struct
	itself)

- line text is now "block malloc"ed -- now for most files there is are two
	mallocs and a single read, no copies.  previously there were two
	mallocs per line, and two copies (stdio's and ours).  This change
	implies that LINE structures and their text should not be moved
	between buffers -- the space they occupy may have come from the big
	chunks attached to the buffer structure, and not from the malloc
	pool directly.  So lfree() and lalloc() now take a buffer pointer
	argument.

- there is "line poisoning" code, in line.c, which trashes the contents of
	freed lines and LINE structs, to make it easier to find bad usage.

- do_fl_region() is a convenient wrapper for writing operator commands that
	work on full lines -- there isn't yet a similar wrapper for partial
	line operators.

- use mlwrite() for informative messages that terse mode will suppress,
	mlforce() for messages that _must_ be seen, and mlprompt() for
	messages that need a response from the user unless they're in a
	command file.
