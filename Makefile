INCDIRS:=$(shell echo | gcc -Wp,-v -x c++ - -fsyntax-only 2>&1 | awk '/^ / {printf "-I%s ", $$1}')
CC=g++
CFLAGS=-static -std=c++11 -Wall -Werror -O2
LFLAGS=-lsgutils2 -lz

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
% : %.o
	$(CC) $< -o $@ $(LFLAGS)

C_FILES = siod.c siod_progress.c
TARGETS = $(patsubst %.c,%,$(C_FILES))
OBJECTS = $(patsubst %.c,%.o,$(C_FILES))

default: $(TARGETS)

clean:
	\rm -f $(TARGETS) $(OBJECTS)
depend:
	@makedepend $(INCDIRS) siod.c siod_progress.c

# DO NOT DELETE

siod.o: /usr/include/stdio.h /usr/include/features.h
siod.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
siod.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stddef.h
siod.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
siod.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stdarg.h
siod.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stdint.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stdint-gcc.h
siod.o: /usr/include/stdlib.h /usr/include/bits/waitflags.h
siod.o: /usr/include/bits/waitstatus.h /usr/include/endian.h
siod.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
siod.o: /usr/include/bits/byteswap-16.h /usr/include/sys/types.h
siod.o: /usr/include/time.h /usr/include/sys/select.h
siod.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
siod.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
siod.o: /usr/include/bits/pthreadtypes.h /usr/include/alloca.h
siod.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
siod.o: /usr/include/xlocale.h /usr/include/errno.h /usr/include/bits/errno.h
siod.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
siod.o: /usr/include/asm-generic/errno.h
siod.o: /usr/include/asm-generic/errno-base.h /usr/include/scsi/sg_cmds.h
siod.o: /usr/include/scsi/sg_cmds_basic.h /usr/include/scsi/sg_cmds_extra.h
siod.o: /usr/include/scsi/sg_cmds_mmc.h /usr/include/scsi/sg_lib.h
siod.o: /usr/include/scsi/sg_io_linux.h /usr/include/scsi/sg_lib.h
siod.o: /usr/include/scsi/sg_linux_inc.h /usr/include/scsi/sg.h
siod.o: /usr/include/scsi/scsi.h /usr/include/signal.h
siod.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
siod.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
siod.o: /usr/include/bits/sigstack.h /usr/include/sys/ucontext.h
siod.o: /usr/include/bits/sigthread.h /usr/include/unistd.h
siod.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
siod.o: /usr/include/bits/confname.h /usr/include/getopt.h
siod.o: /usr/include/sys/time.h /usr/include/sys/wait.h /usr/include/zlib.h
siod.o: /usr/include/zconf.h /usr/include/sys/stat.h /usr/include/bits/stat.h
siod.o: /usr/include/fcntl.h /usr/include/bits/fcntl.h
siod.o: /usr/include/bits/fcntl-linux.h /usr/include/sys/ipc.h
siod.o: /usr/include/bits/ipctypes.h /usr/include/bits/ipc.h
siod.o: /usr/include/sys/shm.h /usr/include/bits/shm.h
siod.o: /usr/lib/../include/c++/4.8.5/string
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/c++config.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/os_defines.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/cpu_defines.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stringfwd.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/memoryfwd.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/char_traits.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_algobase.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/functexcept.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/exception_defines.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/cpp_type_traits.h
siod.o: /usr/lib/../include/c++/4.8.5/ext/type_traits.h
siod.o: /usr/lib/../include/c++/4.8.5/ext/numeric_traits.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_pair.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/move.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/concept_check.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_iterator_base_types.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_iterator_base_funcs.h
siod.o: /usr/lib/../include/c++/4.8.5/debug/debug.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_iterator.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/postypes.h
siod.o: /usr/lib/../include/c++/4.8.5/cwchar
siod.o: /usr/lib/../include/c++/4.8.5/bits/allocator.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/c++allocator.h
siod.o: /usr/lib/../include/c++/4.8.5/ext/new_allocator.h
siod.o: /usr/lib/../include/c++/4.8.5/new
siod.o: /usr/lib/../include/c++/4.8.5/exception
siod.o: /usr/lib/../include/c++/4.8.5/bits/atomic_lockfree_defines.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/localefwd.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/c++locale.h
siod.o: /usr/lib/../include/c++/4.8.5/clocale /usr/include/locale.h
siod.o: /usr/include/bits/locale.h /usr/lib/../include/c++/4.8.5/iosfwd
siod.o: /usr/lib/../include/c++/4.8.5/cctype /usr/include/ctype.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/ostream_insert.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/cxxabi_forced.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_function.h
siod.o: /usr/lib/../include/c++/4.8.5/backward/binders.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/range_access.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/basic_string.h
siod.o: /usr/lib/../include/c++/4.8.5/ext/atomicity.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/gthr.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/gthr-default.h
siod.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
siod.o: /usr/include/bits/setjmp.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/atomic_word.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/basic_string.tcc
siod.o: /usr/lib/../include/c++/4.8.5/iostream
siod.o: /usr/lib/../include/c++/4.8.5/ostream
siod.o: /usr/lib/../include/c++/4.8.5/ios
siod.o: /usr/lib/../include/c++/4.8.5/bits/ios_base.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/locale_classes.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/locale_classes.tcc
siod.o: /usr/lib/../include/c++/4.8.5/streambuf
siod.o: /usr/lib/../include/c++/4.8.5/bits/streambuf.tcc
siod.o: /usr/lib/../include/c++/4.8.5/bits/basic_ios.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/locale_facets.h
siod.o: /usr/lib/../include/c++/4.8.5/cwctype /usr/include/wctype.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/ctype_base.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/streambuf_iterator.h
siod.o: /usr/lib/../include/c++/4.8.5/x86_64-redhat-linux/bits/ctype_inline.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/locale_facets.tcc
siod.o: /usr/lib/../include/c++/4.8.5/bits/basic_ios.tcc
siod.o: /usr/lib/../include/c++/4.8.5/bits/ostream.tcc
siod.o: /usr/lib/../include/c++/4.8.5/istream
siod.o: /usr/lib/../include/c++/4.8.5/bits/istream.tcc
siod.o: /usr/lib/../include/c++/4.8.5/vector
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_construct.h
siod.o: /usr/lib/../include/c++/4.8.5/ext/alloc_traits.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_uninitialized.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_vector.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_bvector.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/vector.tcc
siod.o: /usr/lib/../include/c++/4.8.5/set
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_tree.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_set.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_multiset.h
siod.o: /usr/lib/../include/c++/4.8.5/iomanip
siod.o: /usr/lib/../include/c++/4.8.5/sstream
siod.o: /usr/lib/../include/c++/4.8.5/bits/sstream.tcc lfsr.h
siod.o: /usr/lib/../include/c++/4.8.5/map
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_map.h
siod.o: /usr/lib/../include/c++/4.8.5/bits/stl_multimap.h siod.h
siod_progress.o: /usr/include/stdio.h /usr/include/features.h
siod_progress.o: /usr/include/stdc-predef.h /usr/include/sys/cdefs.h
siod_progress.o: /usr/include/bits/wordsize.h /usr/include/gnu/stubs.h
siod_progress.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stddef.h
siod_progress.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
siod_progress.o: /usr/include/libio.h /usr/include/_G_config.h
siod_progress.o: /usr/include/wchar.h
siod_progress.o: /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/stdarg.h
siod_progress.o: /usr/include/bits/stdio_lim.h
siod_progress.o: /usr/include/bits/sys_errlist.h /usr/include/errno.h
siod_progress.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
siod_progress.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
siod_progress.o: /usr/include/asm-generic/errno-base.h /usr/include/string.h
siod_progress.o: /usr/include/xlocale.h /usr/include/sys/types.h
siod_progress.o: /usr/include/time.h /usr/include/endian.h
siod_progress.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
siod_progress.o: /usr/include/bits/byteswap-16.h /usr/include/sys/select.h
siod_progress.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
siod_progress.o: /usr/include/bits/time.h /usr/include/sys/sysmacros.h
siod_progress.o: /usr/include/bits/pthreadtypes.h /usr/include/sys/ipc.h
siod_progress.o: /usr/include/bits/ipctypes.h /usr/include/bits/ipc.h
siod_progress.o: /usr/include/sys/shm.h /usr/include/bits/shm.h siod.h
