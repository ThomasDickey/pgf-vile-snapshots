The following changes are grouped only roughly, and represent the delta
of the current release from vile version three.  -pgf   10/22/91
------------------------------------------------------

VI COMPLIANCE:

- made ^N, ^P, and ^T do exactly what they do in vi.  (next line,
	previous-line, and tag-pop) Use ^X-o to switch windows, and ^X-2 to
	split them.

- the H, M, and L commands are now absolute motions

- you can no longer backspace from the beginning of a line to the end of
	the previous.

- a motion caused by an undo is considered absolute, and the "last dot"
	mark is reset.

- some modes/values have changed names: aindent is now autoindent, swrap is
	now wrapscan, exact is now noignorecase, and they all have the vi
	shorthands (e.g.  tabstop is the same as ts, ic is the same as
	ignorecase).

- added 'vi +/searchstring file.c' invocation syntax

- "showmatch" mode now works

- you can now specify a count to the ~ command.

- "paragraphs" terminate at nroff sections, as well as nroff paragraphs.

- backslashes now have a protective effect on the : line -- they guard
	against expansion of #, %, :, and \.

- a lone ":f" without filename now gives same info as ^G

- ^W ad ^U are now both read from the tty settings, and they both work in
	insert mode and on the : line.  There is a "backspacelimit" setting
	that controls whether these (and ^H or DEL) will backspace past the
	insert point in insert mode.

- two new functions, to make normal motions more vi compliant --
	forwchar_to_eol and backchar_to_bol, which won't move past the
	boundaries of the current line.

- 'x', 's', 'r' will no longer delete a line if used on an empty line.

- after a ":e!" command, the buffer name changes to match the filename, as
	if this were a new file read.  It used to just suck a new file into
	an old buffer, and the names didn't match.

- there is now a "taglength" setting, and the name of the tags file is in
	the "tags" setting.

- ANSI style arrow keys now work from either command or insert mode.

MISCELLANEOUS

- tabstops can now be set to any value, and there is a separate value for
	tabstops in C mode.  (c-tabstop)

- there is now minimal support for horizontal scrolling -- the code for
	keeping the cursor on lines that are too short to be visible at all
	is still broken.

- the !  filter command for running text through an external filter now
	works

- the code should now be more portable, to various compilers and systems

- there's less use of variant arguments to internal routines, to help
	portability

- quite a bit of code cleanup, at least I think so

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

- reading files in general is _much_ faster, unless the file contains lines
	longer than 255 characters, in which case things slow down again.

- it is no longer legal to write out the contents of a buffer that has view
	mode set.

- the format region command (^A-f) now restarts with each fresh paragraph,
	so you can format an entire file at once, without collapsing it all
	into a single paragraph

- the position of the cursor within a window (the framing) is now preserved
	while the window if "off-screen" -- it no longer reframes when made
	visible again.

- there is now a mode indicator on the modeline: I for insert, O for
	overwrite, and R for replace-char.  No, it's not suppressable.  It
	could be.

- :s/s1/s2/5 now works, to change the 5th occurence on a line

- there are new commands to write all buffers: :ww is a synonym for
	"write-changed-buffers", and :wwq is now a synonym for
	"write-changed-buffers-and-quit"

- it is now legal to "kill-buffer" a displayed buffer

IMPLEMENTORS NOTES:

- internally, most pointers into the buffer, consisting of a LINE * and an
	int offset are now represented with a MARK structure -- in the
	nineties, it's time to start using structure assignment.

- set of values that are frequently handed about between buffers and
	windows, or windows and their "children" are now grouped into
	B_TRAITS and W_TRAITS structures, for easier bookkeeping.  There
	are lots of macros to hide the added data structure depth.

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

- do_fl_region() is a convenient wrapper for writing operator commands that
	work on full lines -- there isn't yet a similar wrapper for partial
	line operators.

