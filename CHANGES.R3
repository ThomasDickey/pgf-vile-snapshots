
Changes for vile 3.40:
(pgf)

	+ the makefile no longer supplies any -I options.  this will cause some
	  machines to not find "ioctl.h" where it's in the sys subdirectory, but
	  I just spent a day tracking a bug caused by picking up the wrong system
	  headers (i needed the gcc "fixincludes", and they had moved).

	+ now honor LINES and COLUMNS in getscreensize (display.c)

	+ now handle searching multiple tags files specified in "set tags"

	+ bugfix to fences.c, which fixes indentation when inserting '}'
(tom)
	+ corrected 'ffwopen()' in fileio.c so that write of current buffer on
	  VMS works properly (forgot to strip version number).

	+ made 'flook()' work properly for vms (file bind.c)

	+ moved 'dname[]' table into file exec.c and 'cname[]' table into file
	  modes.c; did this to get rid of 'comma' macro, which does not work as
	  intended on Turbo-C (the comma got evaluated too soon).

	+ to prevent redefinition error in Turbo-C, removed redundant includes
	  for <stdio.h>, <string.h> and <signal.h> (already in file estruct.h)
	  from the ".c" files: ansi.c at386.c basic.c bind.c buffer.c crypt.c
	  dg10.c display.c eval.c exec.c file.c globals.c hp110.c hp150.c
	  ibmpc.c input.c isearch.c line.c main.c modes.c npopen.c oneliner.c
	  regexp.c region.c search.c spawn.c st520.c tcap.c termio.c tipc.c
	  vmsvt.c vt52.c window.c word.c wordmov.c x11.c z309.c z_ibmpc.c

	+ moved macro 'P' from file proto.h to estruct.h, adjusted definitions
	  in file 'estruct.h' to allow use of 'P' macro in CMDFUNC definition.
	  Added definition of type 'CmdFunc' for the actual pointer-type.

	+ use P-macro for TERM struct definitions (and added TTscroll),
	  modified files estruct.h and display.c

	+ corrected logic of 'dot_replays_macro()' in file input.c (for the
	  special case of macro "@w" followed by "dd" command, I noticed that
	  '.' replayed both commands).

	+ compiled with Turbo-C, turned on warning if function was called w/o a
	  prototype.  Added prototypes (including those for static procedures)
	  to get rid of these and other compiler warnings.  Doing this found an
	  error in eval.c and one in history.c (files modified: bind.c buffer.c
	  display.c eval.c exec.c fences.c file.c fileio.c history.c ibmpc.c
	  input.c insert.c main.c mktbls.c modes.c npopen.c opers.c path.c
	  random.c regexp.c region.c spawn.c tbuff.c window.c).

	+ corrected MSDOS-specific pathname translation in files path.c and
	  random.c

	+ corrected d_namlen value supplied in readdir function for VMS (file
	  vms2unix.c)

	+ implemented first version of filename completion for VMS.  (Directory
	  names are not completed; there are still some ambiguities between
	  scratch buffer names and VMS directory paths).

	+ added option KBD_UPPERC to input.c; use this to force prompts for VMS
	  filenames to uppercase.

	+ removed redundant declarations for 'getenv()'; it is already in file
	  estruct.h

	+ added new file 'vmspipe.c' which provides a piped-read facility
	  'vms_rpipe()'.

	+ modified file finderr.c to recognize lint output on SUNOS; also
	  corrected it to ensure that sscanf doesn't scan past the string
	  limits (no EOS was formerly on each line).

	+ added/used new functions 'pathleaf()' and 'pathcat()' to file path.c
	  to hide vms/unix pathname differences.

Changes for vile 3.39 (from Tom Dickey):

	+ corrected error in 'get_recorded_char()' (file input.c) that caused
	  command "ix^[j.j.j.j." to not repeat properly.

	+ corrected prompt-string in 'ShellPrompt()' (file spawn.c)

	+ corrected behavior of filename completion (file filec.c) so that
	  internal names and shell names can have blanks in them.

	+ provided a last-replay value for 'execfile()' in file exec.c (it was
	  defaulting to the current buffer, which is wrong).

	+ corrected pointer test in 'tb_sappend()', which was null in the
	  special case of a ":w" command before any ":e" or other read.

	+ simplified MSDOS-drive translation (untested) in file random.c to
	  accommodate changes made for VMS in other files.

	+ modified 'cd()' procedure in file random.c to chdir to home directory
	  on systems that have one.

	The following are changes to support VMS:

	+ added new files (shown in makefile and descrip.mms): dirstuff.h,
	  vms2unix.c, vile.com

	+ use new file 'dirstuff.h' for definitions in filec.c and path.c

	+ corrected some dependencies in descrip.mms

	+ corrected logic in 'imply_alt()' (file buffer.c) that lost the
	  original filename argument (replaced it with the lengthen-path).

	+ corrected logic in 'getfile()2' (file file.c) to account for the
	  distinction between buffer-names and file-names.  Simplified
	  'getfile()' and 'getfile2()' because of modifications to
	  'shorten_path()' and 'lengthen_path()'.

	+ corrected logic of 'ffread()' (file fileio.c); on VMS a single read
	  does not necessarily read an entire file.

	+ corrected definition of macro 'isready_c()' for VMS.

	+ modified 'shorten_path()' (file path.c) to provide the "./" prefix
	  that was added in files buffer.c and display.c; also absorbed the
	  test for MSDOS device prefix (e.g., "c:") there.  This was needed to
	  allow VMS port to show native pathnames.  The equivalent of "./" on
	  VMS is "[]".

	+ use VMS function 'fgetname()' and 'getname()' to determine the
	  fully-resolved path in 3 cases: after writing a file, after reading a
	  file, and in 'lengthen_path()' (files file.c and path.c).  This makes
	  it know the current version number of each file.

	+ modified 'makename()' (file file.c) to account for VMS pathnames. 
	  Note that DecShell names (which the C runtime library accepts as
	  arguments to fopen, etc.) look much like unix filenames.

	+ added new function 'is_internalname()' to file path.c; this serves
	  the same purpose as the (now obsolete) macro isScratchName.  This
	  makes a better check (to keep VMS paths distinct from scratch buffer
	  names) than the macro.

	+ deleted macro isScratchName from file estruct.h; changed macro
	  isInternalName to use function 'is_internalname()'.  Adjusted other
	  macros in estruct.h and display.c to make VMS port work properly.

	+ added new function 'non_filename()' to file path.c; use this in files
	  bind.c and random.c to encapsulate the program+version string.  Use
	  'non_filename()' in 'is_internalname()'.

	+ corrected logic of 'screen_string()' (file input.c) to handle the
	  distintion between "[" and "]" used to delimit pathnames on VMS, and
	  to indicate scratch-buffer names.

	+ corrected wildcard, path and scratch-buffer characters for VMS (file
	  main.c).

	+ corrected/modified file path.c so that the functions therein support
	  VMS filenames.  Added several new functions to do this.

	+ corrected code for 'catnap()' (file random.c) for VMS.

	+ corrected several 'exit()' arguments (file termio.c)

	+ corrected VMS's ttgetc function (its interaction with 'typahead()'
	  was wrong; also 'typahead()' was not really implemented (file
	  termio.c).

	+ ifdef'd out a redundant 'ttclean()' call (which caused the output on
	  VMS to be closed more than once, which is really an error).

	+ guard 'vtinit()' (file vmsvt.c) against t_mcol or t_mrow too small

Changes for vile 3.38:
(pgf)
	+ cleaned up internal naming and help for alt-tabpos mode. 
	  alt-tabpos on implies emacs-style, and it's off by default

(tom)
	+ corrected status-check in 'ffclose()' (in fileio.c) that generated
	  spurious 'Error on close' message, visible when nothing was read from
	  a pipe.  (Also eliminated redundant defs for 'npopen.c' in fileio.c
	  and proto.h).

	+ corrected place in 'globals.c' where (if the buffer was readonly),
	  the '/' delimiter after an s-command was not eaten.

	+ modified loop-limit in "show-registers" command to display the
	  numbered registers.

	+ corrected file fences.c, allowing C preprocessor lines to have
	  leading whitespace before '#'.  The cursor may be on any character
	  from the first column through the '#'.

	+ rewrote procedure 'cpp_fence()' in file fences.c, allowing it to
	  recognize "elif" keyword.  Added 'nextchar()' to file basic.c to
	  support this.

	+ modified procedure 'ShellPrompt()' in file spawn.c to eliminate the
	  ": " before prompts for ^X-!.

	+ corrected 'kbd_reply()' in file input.c to invoke 'tgetc()' directly
	  when ^V is used to escape a character.

	+ corrected 'kbd_kill_response()' in file input.c; ^W is not supposed
	  to trim blanks before a word.

	+ corrected 'update()' in file display.c; the VISMAC define does not
	  apply to "." command.  This fix allows showmatch to work with "."
	  command.  Also, in 'fmatch()' in file fences.c; test return-value of
	  'update()' to avoid catnap when update fails.

	+ corrected 'insert()' and 'insertbol()' in file insert.c; the cursor
	  was not being advanced after repeated insertions like vi.

	+ corrected file input.c so that it can handle a "." command within a
	  keyboard macro.  The macro was "iA^[l2." (for testing).

	+ modified file input.c (new procedure 'dot_replays_macro()' so that
	  the "." command after defining or executing a keyboard macro causes
	  the macro to be (re)invoked.

	+ split 'kcod2prc()' in file bind.c into 2 parts; the new one (i.e.,
	  'kcod2str()') is used to support 'dot_replays_macro()'.  Also use new
	  function 'fnc2kcod()'.

	+ added new function 'kbm_started()' to file input.c to get rid of then
	  non-recursive variable 'kbdplayreg'.

	+ save/restore "." repeat counts in 'start_kbm()' and 'finish_kbm()' so
	  that repeat works properly for keyboard macros.

Changes for vile 3.37:
(pgf)
	+ works on osf/1 alpha now, with hacks applied to termio.c and
	  display.c
(tom)
	+ added files 'descrip.mms' and 'vms_link.opt'; compiled on VAX/VMS
	  (but did not make it run) (tom dickey)

	+ pathname tracking fixes (calling shorten_path() correctly everywhere)

Changes for vile 3.36 (from Tom Dickey):

	+ supplied initial value for dirc-mode

	+ guarded 'mlreply_file()' against missing curbp->b_fname (for use in
	  .vilerc startup).

	+ corrected logic in 'kbd_reply()' (file input.c) that caused backspace
	  to be treated as a command-delimiter.

	+ corrected logic in 'kbd_reply()' (file input.c) that caused '/'
	  character in complex substitutions to be eaten, e.g.,
		:g/pattern/s//newpattern/g
	  broke after the 's'.

	+ corrected calls (broke in 3.35) on 'mlreply_file()' from 'insfile()'
	  and 'operwrite()' (mods to files estruct.h, filec.c, file.c and
	  opers.c).  The ^R and ^W commands always prompt for name, rather than
	  taking it from the screen.

	+ corrected end-of-line logic in 'do_a_mode()' (file modes.c).  This
	  had allowed history scrolling to display keywords that were separated
	  by blanks; fixed this by testing for both space and "=" characters.

	+ added calls to 'hst_glue()' in 'unqname()' and 'getfile2()' to add
	  blank in command history before the responses these functions solicit
	  from 'mlreply()'

	+ added macro 'char2int()' to file estruct.h, use this in fileio.c and
	  tbuff.c to hide byte-masking.

	+ changed data in TBUFF to 'char' (unsigned char does not buy that much
	  conciseness); absorbed macros 'tb_values()' and 'tb_length()' into
	  file tbuff.c as functions.

	+ corrected logic of 'tb_leaks()' (file tbuff.c); I had only exercised
	  this at the end of execution.

	+ removed unused define for TMPSTOP

	+ recoded handling of keyboard macros in file input.c to use TBUFF
	  structs (so there is no limit on length of keyboard macros).

	+ modified 'execkreg()' to remove length-limit on macros there as well
	  (file line.c)

	+ modified 'execkreg()' to allow invoking registers 0-9 (file line.c)

	+ modified files input.c and line.c so that "." and @-commands can be
	  invoked from within a keyboard-macro (i.e., ^X-&).

	+ modified files insert.c and input.c so that repeated "." command
	  works properly for i,I,R commands (the cursor must appear at the end
	  of the insertion).

	+ modified so that ^Z (job suspension) works properly on apollo sr10.3
	  (files termio.c and spawn.c).

	+ modified 'ttgetc()' in file termio.c so that (for apollo) it reads in
	  raw-mode.

	+ added flag to 'tgetc()', allowing it to suppress the conversion of
	  intrc to abortc (files basic.c csrch.c eval.c exec.c input.c
	  insert.c).  This change also lets users quote ESC characters.

	+ corrected unused args in 'globber()' (file globals.c)

	+ corrected special case in use of 'canonpath()' on apollo (user could
	  type in path beginning with "/", which was not getting "//" portion
	  of pathname prepended.

	+ renamed "vitabpos" mode to "alt-tabpos" mode, avoiding conflict with
	  "view" mode.

	+ added "implybuffer" (ib) mode to use in splitting out vi-style
	  buffer-implication from 'autobuffer' mode (files modetbl and main.c).

	+ added "samebangs" (sb) mode to allow the ^X-!!  and :!!  commands to
	  be optionally the same.

	+ Corrected logic of 'imply_alt()' so that it works with files read, as
	  well as written (files buffer.c and file.c).

	+ split out the pathname functions into file path.c

	+ added function 'is_appendname()' to path.c to better encapsulate the
	  isAppendToName macro's knowlege of the ">>" length (files path.c,
	  filec.c, fileio.c, random.c).  Used this to make functions
	  'canonpath()', 'lengthen_path()', 'shorten_path()' and
	  'is_pathname()' work properly with ">>" prefix.

	+ added code to 'path.c' to resolve "~" as user's home-directory.

	+ corrected code in 'makebufflist()' that caused b_bfname member to
	  be modified (file buffer.c).

	+ moved MSDOS drive-logic from 'ch_fname()' into 'lengthen_path()'
	  (files random.c and path.c)

	+ added procedure 'copy_val()' to modes.c to encapsulate the make-local
	  and make-global macros.  This fixes a case in which memory could be
	  freed more than once: if the user had done 'setl' commands for two
	  different buffers.

	+ made variables 'doslines' and 'unixlines' local to the procedures
	  'slowreadf()' 'readin()' in file.c; initialized these so that
	  successive reads of a set of files will treat the dos/unix line
	  counts separately.

	+ added command "set-dos-mode" to force the current buffer to be
	  in DOS-mode (trimming CR's).

Changes for vile 3.35 (from Tom Dickey):

	+ addressed compiler warnings from gcc 1.36 on apollo sr10.3.  (I know
	  it is old, but it gives useful warnings).  This required some minor
	  adjustments to defines/includes in files estruct.h, proto.h, main.c,
	  fileio.c, npopen.c, random.c to accommodate order-of-definition
	  problems in stdio.h and signal.h

	+ tested with gcc 1.40 on sunos 4.1.1

	+ tested compiling with SMALLER=1; fixes to files eval.c, input.c,
	  mktbls.c, npopen.c, proto.h, random.c (On my machine, this only cuts
	  about 10% from the binary).

	+ added EOS at mismatch-point in 'fill_partial()' in file bind.c, fixes
	  bug that caused incorrect display after repeated name completion and
	  query operations.

	+ quarded 'kbd_complete()' against embedded nulls in its buf-argument,
	  by testing for nulls before completion logic in file input.c

	+ added a flag 'kbd_expand' to make calls on 'kbd_putc()' via
	  'mlforce()' and related functions show tabs as a space.  This
	  modifies files bind.c, edef.h, display.c to fix a problem I
	  introduced in 3.32

	+ use 'kbd_expand' flag in file input.c to fix non-displaying ^M; other
	  fixes include quoting and backslash fixes, as well as correction to
	  wkillc handling.

	+ modified finderr.c to make the embedded tabs as "\t" digraphs, so
	  they are visible.  Added pattern to use with apollo C compiler.

	+ corrected logic in 'updpos()' (file display.c) so that vile displays
	  cursor location just-like-vi for tabs and control characters.

	+ made 'wordcount()' code (file word.c) work properly.

	+ corrected 'join()' (file word.c) so that blanks are not inserted when
	  joining after a space, or joining a blank line.  Also (like 'vi'), if
	  there is trailing whitespace on the first line, don't modify it.

	+ corrected flags for join-lines in file cmdtbl so that the region-
	  oriented code can work.

	+ made join-lines work with regions (files opers.c, word.c).

	+ restored GLOBOK flag for join-lines-til (regions work now).

	+ interchanged order of calls on 'toss_to_undo()' and 'lremove()' in
	  'ldelnewline()', since this was causing incorrect cursor placement
	  after an undo.  (The particular case arose in the rewritten
	  join-lines, when joining a blank line to a non-blank line).

	+ modified 'edithistory()' in file history.c to ifdef out the ESC code
	  (with new symbol KSH_HISTORY), merge the loops to use 'escaped' flag
	  to avoid redundant logic, and pick up control chars (e.g., ^N and ^P)
	  for scrolling.  This also modified files input.c and exec.c

	+ corrected logic in history.c (from 3.34) that uses the function
	  'WillExtend()' to modify/macroize !-commands.  The code did not
	  provide for the case in which the user would modify the text after
	  scrolling to it.

	+ corrected call on 'hst_flush()' in file pipecmd.c (it was not called
	  if the pipe-command failed).

	+ revised 'tgetc()' in file input.c to make it use a single return
	  point (simplifies testing and debugging).

	+ corrected file modes.c so that user can abort from a set command.

	+ deleted code for unused global 'lineinput' (files bind.c, input.c,
	  edef.h).

	+ renamed ifdef for LAZINESS to (global mode) TAGSLOOK, added an entry
	  to modetbl for "looktags" to test/update this code.  Note that this
	  is turned off by commenting out the "LookTags" line in modetbl.  This
	  affects also files main.c, file.c modes.c and tags.c

	+ added file 'filec.c' to perform filename completion.  Moved code that
	  was ifdef'd LAZINESS there, and recoded so that it stores paths
	  normally (not reversed!).

	+ created function 'mlreply_file()' in filec.c, used this to
	  consolidate prompting for filenames in file.c (Note that now all
	  filename prompts can get data from the screen).

	+ made most of the calls on 'mlreply_file()' default to the current
	  filename; this is simpler than the last name given to the command.

	+ moved check for writing view-only buffer from 'filewrite()' as new
	  local function 'writable()', before calls on 'writereg()' This
	  affects 'filewrite()', 'filesave()', 'imdying()' and 'writeregion()'
	  implicitly.

	+ use 'mlreply_file()' in 'execfile()' (file exec.c)

	+ use 'mlreply_file()' in 'operwrite()' (file opers.c)

	+ added 'no_such_file()' to file.c; use this in 'execfile()' so that
	  ":source" will report an error if the file is not found.

	+ created function 'mkreply_dir()' modeled after 'mlreply_file()' in
	  filec.c, used this to allow pathname completion in 'cd()' in random.c

	+ added mode "dirc" to file modetbl, to support directory name
	  completion.

	+ added l_number to LINE struct, b_bytecount to BUFFER struct (file
	  estruct.h) to use in caches for # of bytes and lines displays.

	+ added macro 'for_each_line' to file estruct.h

	+ added procedure 'bsizes()' to buffer.c to compute values for l_number
	  and b_numchars, b_numlines values.  Will later get this to cache
	  results to speed up 'line_no()' and related code.

	+ corrected column-splitting in file modes.c (error in 3.34 changes)
	  which caused dos-mode to not be shown.

	+ corrected flag in 'modetbl' for dos-mode: this makes dos-mode show in
	  the status-line.

	+ added new entrypoints to tbuff.c: tb_copy, tb_scopy, tb_bappend

	+ merged code in 'spawn1()' and 'pipecmd()' (file spawn.c) so that
	  !-commands and !!-commands work consistently.

Changes for vile 3.34 (from Tom Dickey):

	+ modified mktbls.c to use 3rd column of modetbl (window-flags).  Moved
	  window-flag defines in estruct.h to allow their use in nemode.h

	+ in modes.c, moved the code that updates window hints-flags from the
	  'adjustmode()' to 'adjvalueset()' procedure so that it can use the
	  new window-flags entry in nemode.h

	+ modified mktbls.c (and estruct.h, edef.h) to support global modes.

	+ added to modes.c: 'new_regexval()', 'free_regexval()', and
	  'free_val()'.  Use these in main.c and modes.c to simplify
	  leak-testing.

	+ added to estruct.h defines to use in testing memory leaks using both
	  dbmalloc as well as apollo-specific traces.

	+ added to main.c ifdef for NO_LEAKS; used this in testing for
	  permanent memory leaks (freeing data also finds specific types of
	  allocator corruption).  Added test-code for this purpose to:
	  buffer.c, display.c, line.c, regexp.c, tbuff.c, window.c

	+ plugged memory-leak in 'getfile()' in file.c by splitting it after
	  the temporary-buffer allocation, making tbp a static value, and
	  deleting the line that set the tbp.b_fname pointer to null.

	+ revised the value-checking in 'adjvalset()' (file modes.c) so that
	  unnecessary changes are ignored.

	+ further modified file modes.c so that typing ' ' (except after string
	  or pattern values) will continue the prompting for further set-modes,
	  as in noninteractive use.

	+ put the TTputc for '\r' back into 'kbd_putc()'; omitted this when
	  taking code for 'mlmsg()' from display.c

	+ added new function 'GetEnv()' to eval.c to guard against
	  null-return from 'getenv()'.

	+ added new command (to file line.c) "show-registers" to show the
	  contents of kill-buffers and registers.

	+ added new module 'history.c'.  This implements up/down scrolling in
	  an invisible buffer "[History]", which logs all :-commands.

	+ coded-around apollo sr10.3 optimizer-bug in 'execkreg()':
		kbdend = &kp->d_chunk[i];
	  resulted in kbdend == kp->d_chunk.

	+ moved autobuffer and c-suffixes modes to global-table.  Deleted 'cwd'
	  mode.

	+ modified mktbls.c so that each table will be sorted by name (not by
	  type&name).  Modified modes.c to take advantage of this and display
	  the modes sorted in columns (more like 'vi'), except for long strings
	  (which are deferred to the bottom of the display, still).

	+ make modes.c show the actual color name in addition to the index.

	+ rewrote 'kbd_string()' to interface it with 'edithistory()'.  Split
	  out entrypoint 'kbd_reply()' to support integration of history with
	  'namedcmd()'.

	+ redefined 'kcod2key()' in terms of N_chars; added macro 'isspecial()'
	  to estruct.h

	+ revised 'kbd_engl_stat()' and 'namedcmd()' to use the 'kbd_reply()'
	  entrypoint so that user can scroll through history of any portion of
	  commands.

	+ modified 'search.c' to log history for '/' and '?' commands.

	+ modified x11.c to use memcpy rather than bcopy (for consistency).

	+ made x11.c compile properly on apollo sr10.3 (there is a conflict
	  between prototypes in <string.h> and <strings.h>).

Changes for vile 3.33:

    	* many: cleanup for gcc-2.3's -Wall warnings

    	* bind.c: added new function, insertion_cmd(), which returns char
	that gives us simple insert mode.  used in x11.c and input.c, for
	pasting and for insert-mode arrow keys

    	* estruct.h: don't redefine "const" on linux

    	* makefile: added aix warnings, changed name of ENV to ENVIR, due
	to name conflict with linux make

    	* random.c: add include of sys/select.h for AIX

    	* x11.c: added code to support arrow keys and function keys from
	Phil Rubini

Changes for vile 3.32 (from Tom Dickey):

	* modified estruct.h, proto.h and display.c to compile vile on HP/Apollo
	  SR10.2 (CC 6.7).  Added new definitions ANSI_PROTOS and ANSI_VARARGS
	  to simplify this.

	* modified 'mktbls.c' to produce nemode.h; use these definitions
	  to replace buffer/window mode definitions in estruct.h and edef.h
	  (note that this obsoletes symbol MAX_STRING_W_VALUE, reordered
	  window int-modes).  Further modified 'mktbls.c' to only create
	  ".h" files for the cases where it has data (so that I can split
	  modetbl off from cmdtbl, avoiding unnecessary recompiles).

	* supplied an ifdef (at least for APOLLO!) in termio.c so that ^C
	  and ^U work properly.

	* added buffer-mode 'wrapmargin' to file insert.c (which overrides
	  wrapwords iff it is set to nonzero).

	* added new module 'tbuff.c' to manage dynamic buffers in places
	  where local static buffers exist now.

	* modified 'ins()' in insert.c so that it reallocates 'insbuff[]',
	  allowing arbitrarily-large re-inserts.

	* modified 'input.c' to dynamically allocate 'tmpcmd' and 'dotcmd'
	  buffers.  This allows arbitrarily-large commands to be repeated
	  with '.'.

	* guard 'lengthen_path()' in file.c against shell/pipe/internal names.

	* added variable "$shell" (files cmdtbl and eval.c) to override
	  environment-variable "SHELL" in pipe-commands.

	* made 'show-vars' (file eval.c) show leading '$' or '%' to be
	  consistent with the way the user must type them.

	* suppress 'show-vars' (file eval.c) ERROR values (these correspond to
	  unimplemented variables)

	* corrected error in nextbuffer/swbuffer (file buffer.c) that allowed
	  vile to crash if "set noautobuffer" was specified in .vilerc

	* corrected display in 'show-vars' (file eval.c) of "$kill" (reference
	  to volatile data).

	* made new function 'kbd_complete()' (file bind.c) to allow keyboard
	  completion in tables other than NTAB.

	* modified logic in new function 'kbd_complete()' to show permissible
	  completions in response to '?'.

	* consolidated logic in exec.c, display.c, bind.c, input.c with new
	  procedures 'kbd_putc()', 'kbd_puts()', 'kbd_erase()', 'kbd_alarm()'
	  These hide random use of 'ttcol'.

	* make 'kbd_engl_stat()' (file bind.c) recognize word-delete character.

	* eliminated redundant unget/get (files bind.c, exec.c) in the interface
	  of 'kbd_engl_stat()'; this caused an infinite loop in the call from
	  'namedcmd()'.

	* modified bug-test in 'updpos()' (file display.c); I had a process hang
	  in that loop for some reason.

	* collapsed expand/dobackslashes args of 'kbd_string' (file input.c)
	  into one mask.

	* modified 'mktbls.c' further to produce table 'all_modes[]', to use
	  this for name-completion of mode.

	* modified 'mktbls.c' to recognize color-types so I could get rid of
	  upper/lower case kludge in modes.c

	* modified 'modes.c' to use color-types.  The interactive behavior now
	  uses 'kbd_string()' to split the input at the '=' mark (if given).

	* added option-flag KBD_LOWERC to 'kbd_string()' (file input.c) so that
	  things that are forced to lowercase will be shown in that way on
	  input.

	* modified 'token()' (file exec.c) to handle the 'eolchar' for
	  non-interactive calls on 'kbd_string()'.

	- modified 'eval.c' to recognize '=' as a separator between
	  variable-name and value, as in 'modes.c'.

	- modified 'eval.c' to perform name-completion for environment
	  variables (i.e., those beginning with '$').

	- modified 'modes.c' to accept vi-style multiple assignments in a ":set"
	  command (this works non-interactively).

	- corrected 'imply_alt()' in buffer.c; ensure that the fname-argument is
	  non-null.  If called via 'filterregion()', it will be null.

Changes in 3.31:  (Tom Dickey)

	* corrected test in 'find_alt()', which caused the %/# toggle to work
	  incorrectly in noautobuffer mode.

	* updatelistbuffers after 'cd' command

	* added 'chg_buff()' & 'unchg_buff()' to encapsulate logic to update
	  buffer-list after buffers are modified.  The function 'lchange()'
	  is replaced by 'chg_buff()'.

	* modified 'histbuffer' to toggle according to 'find_alt()'

	* make the filename/buffer/number change for 'killbuffer' apply to
	  'usebuffer()'

	* corrected handling of repeat-count in 'C' command.

	* updated vile.hlp

	* revised 'mktbls.c':

		* filter out redundant ifdef's
		* tab-align data items for readability
		* modify 'nefunc.h' to use prototypes rather than K&R style
		  definition, also split the extern-definitions from the
		  data items for readability.
		* make procedures static, linted
		* modified so that 'evar.h' definitions are automatically
		  generated, so that "show-variables" can show a sorted
		  list of variables.  The new filename is "nevars.h".

	* corrected window refresh flag for 'U' (lineundo) command.

	* added an apollo-specific code in 'imdying()' to generate a walkback
	  for debugging.

	* corrected an error in 'zotwp()': reference to data that had been
	  deleted.

	* corrected an error in 'onlywind()': it did not delink window-pointer
	  after freeing it.

	* noticed that references to 'dfoutfn' weren't reentrant:  I did a
	  ":show-var" from the buffer-list and got only part of the heading.
	  Reorganized display.c to ensure that it is reentrant by using a
	  local copy of 'dfoutfn'.  Note that I didn't add 'dfoutfn' as an
	  argument to 'dofmt()' to avoid complications with varargs.

	* corrected 'dfputli()': it didn't do hexadecimal.

	* corrected 'updateline()': it sometimes failed to highlight the ends
	  of the mode-line.

	* corrected (my last change) to 'vtset()' so that "^J" is shown in
	  list-mode.

	* moved 'writeall()' from main.c to buffer.c; it belonged there, and
	  I used local macros for simplifying it.

	* remove SIGHUP handler before normal call on 'exit()' to avoid
	  vile-died messages when running it in an xterm.

	* corrected VMS definition of exit codes; added def of BAD-macro.
	  modified main.c, display.c, window.c accordingly.

Changes in 3.30, mostly from Tom Dickey:

This is lots of changes, mostly in buffer.c:

	* lint-clean: buffer.c

	* linted also: file.c, modes.c (though I left size_t alone for now).

	* added "list-vars" & "show-vars" to show the current variables

	* modified 'killbuffer' so I could add binding ^X-k to point to
	  a buffer-name in the buffer-list and kill it.

	* also modified 'killbuffer' to recognize filename if buffer-name is
	  not found, also buffer-number.

	* modify buffer.c so that the number shown by '_' and '*' is consistent

	* fixed ^X-o and ^X-O so they update buffer-list, as well as some
	  other places I missed before.

	* make some functions static (where they are really private), because
	  my per-module lint library got too large.  (I believe there are
	  too many externals).  In modes.c, this involved reordering, since
	  my practice is not to have forward-refs on static functions, to
	  make this work with some awful compilers.  This meant that I
	  deleted lines from 'proto.h'.

	* introduced new macro 'isreturn()', used this in correction in
	  'histbuff()'.  I used it in one bug fix & also where I could find
	  explicit compare for \r, \n.

	* added mode 'autobuffer':

		* the default is 'autobuffer' (the vile lru buffering)
		* curbp no longer is necessarily equal to bheadp
		* added members b_created, b_last_used, b_relink to support
		  the autobuffer/noautobuffer mode.
		* if "noautobuffer" is set, ":rewind" and ":n" apply only
		  to the original command-line files.

	* added macros SCRTCH_LEFT, SCRTCH_RIGHT, ScratchName, IsInternalName
	  to encapsulate use of '[' in names.

	* added macros SHPIPE_LEFT, isShellOrPipe to encapsulate use of '!' in
	  names.

	* added function is_pathname to encapsulate tests for pathnames
	  beginning with '.'.  This fixes the display of ".vilerc" from the current
	  directory as "./.vilerc".

	* added new char-types _scrtch and _shpipe to support killbuffer from
	  screen.  Use these types in new macro 'screen_to_bname()' which uses
	  modification of 'screen_string()'.  Used 'screen_to_bname()' to
	  make ^X-e and ^X-k "correctly" parse the bname, fname columns from
	  buffer-list.

	* fixed the error you mentioned in modeline (the position of col-80 mark
	  taking into account sideways and number modes).

	* supplied a missing TTflush in input.c

	* modified 'makebufflist()' to show only the flags actually used in the
	  footnote.

	* modified 'makebufflist()' to show '%' and '#' by the corresponding
	  buffer/filename.

	* added macros 'for_each_buffer()' and 'for_each_window()' to simplify
	  code as well as to keep visible references to bheadp & wheadp
	  confined to buffer.c and window.c respectively.

	* added/used macro SIZEOF

-------------------------------------------------------------------------------

Changes in 3.29:
-----------------

(fixes supplied by tom dickey, eric krohn, and willem kasdorp)

buffer.c: changes for keeping the buffer list more up to date when
	on screen
fences.c: allow match of fence that is first char. in buffer
file.c: avoid inf. loop when choosing unique name for buffers that have names
	containing "x" in the NBUFN-1 position and allow ":e!" with no file
	to default to current file
globals.c: implemented 'v' command
isearch.c, line.c, display.c: ifdef of some unused code
main.c: added .C, .i to cmode suffixes, for C++ and cpp output files
makefile, install target: test for presence of vile in destdir before
	trying to move it to ovile
oneliner.c: don't allow pregion to be used if the plines buffer is current,
	and set the WINDOW list mode, not the BUFFER list mode, so :l works
	again, and fix for s/.//g bug
regexp.c: fixes for interaction of BOL and \s etc.
spawn.c: lint cleanup  mostly casting strcXX() to void.
tcap.c: hack to supply CS or SR for scrolling region control, missing
	from some xterms
termio.c: added missing fflush in two of the ttclean() routines
word.c: fix for counts on 'J' command

------------------------------------------

The rest of the changes lister here are grouped only roughly, and represent
the delta of version 3.28 from vile version three.  -pgf Dec 30 1992

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

- replacement sub-expressions work (\( and \), \1, etc.), as do \U \L \u \l

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

- there is a separate c-shiftwidth, used when in cmode, much as c-tabstop
	replaces tabstop when in cmode.

- ": args" is now similar in spirit to the vi command of that name


MISCELLANEOUS
===============

- reading files in general is _much_ faster, unless the file contains lines
	longer than 255 characters, in which case things slow down again.

- regular expressions are more powerful and much, much faster than they
	were.  vile now uses Henry Spencer's regexp code.  This has the
	side effect of making '\|' a new metacharacter, for alternation
	of expressions.  '\+' and '\?' are also new, and stand for "one
	or more" and "zero or one" occurrences.  they can be used from
	the I-search commands as well.

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

- there is now some support for horizontal scrolling, using ^X-^R and ^X-^L

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
	NOTE THAT THE DOS CODE IS KNOWN TO BE MUCH BUGGIER THAN THE UNIX CODE!

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

