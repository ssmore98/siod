INCDIRS:=$(shell echo | gcc -Wp,-v -x c++ - -fsyntax-only 2>&1 | awk '/^ / {printf "-I%s ", $$1}')
CC=g++
CFLAGS=-static -std=c++11 -Wall -Werror -O2
LFLAGS=-lsgutils2 -lz

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
% : %.o siod-common.o
	$(CC) $^ -o $@ $(LFLAGS)

C_FILES = siod.c siod_progress.c splot.c
TARGETS = $(patsubst %.c,%,$(C_FILES))
OBJECTS = $(patsubst %.c,%.o,$(C_FILES))

default: $(TARGETS)

clean:
	\rm -f $(TARGETS) $(OBJECTS)
depend:
	@makedepend $(INCDIRS) siod.c splot.c siod_progress.c siod-common.c

# DO NOT DELETE

siod.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
siod.o: /usr/include/features.h /usr/include/stdc-predef.h
siod.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
siod.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stddef.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdarg.h
siod.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
siod.o: /usr/include/bits/types/__fpos_t.h
siod.o: /usr/include/bits/types/__mbstate_t.h
siod.o: /usr/include/bits/types/__fpos64_t.h /usr/include/bits/types/__FILE.h
siod.o: /usr/include/bits/types/FILE.h /usr/include/bits/types/struct_FILE.h
siod.o: /usr/include/bits/stdio_lim.h /usr/include/bits/sys_errlist.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdint.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdint-gcc.h
siod.o: /usr/lib/../include/c++/8/stdlib.h /usr/include/stdlib.h
siod.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
siod.o: /usr/include/sys/types.h /usr/include/bits/types/clockid_t.h
siod.o: /usr/include/bits/types/time_t.h /usr/include/bits/types/timer_t.h
siod.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
siod.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
siod.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
siod.o: /usr/include/bits/select.h /usr/include/bits/types/sigset_t.h
siod.o: /usr/include/bits/types/__sigset_t.h
siod.o: /usr/include/bits/types/struct_timeval.h /usr/include/alloca.h
siod.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
siod.o: /usr/include/strings.h /usr/include/errno.h /usr/include/bits/errno.h
siod.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
siod.o: /usr/include/asm-generic/errno.h
siod.o: /usr/include/asm-generic/errno-base.h /usr/include/time.h
siod.o: /usr/include/bits/time.h /usr/include/bits/types/clock_t.h
siod.o: /usr/include/bits/types/struct_tm.h /usr/include/scsi/sg_cmds.h
siod.o: /usr/include/scsi/sg_cmds_basic.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdbool.h
siod.o: /usr/include/scsi/sg_cmds_extra.h /usr/include/scsi/sg_cmds_mmc.h
siod.o: /usr/include/scsi/sg_lib.h /usr/include/scsi/sg_io_linux.h
siod.o: /usr/include/scsi/sg_lib.h /usr/include/scsi/sg_linux_inc.h
siod.o: /usr/include/scsi/sg.h /usr/include/scsi/scsi.h /usr/include/signal.h
siod.o: /usr/include/bits/signum.h /usr/include/bits/signum-generic.h
siod.o: /usr/include/bits/types/sig_atomic_t.h
siod.o: /usr/include/bits/types/sigval_t.h
siod.o: /usr/include/bits/types/__sigval_t.h /usr/include/bits/sigaction.h
siod.o: /usr/include/bits/sigcontext.h /usr/include/bits/sigstack.h
siod.o: /usr/include/bits/ss_flags.h
siod.o: /usr/include/bits/types/struct_sigstack.h /usr/include/unistd.h
siod.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
siod.o: /usr/include/sys/time.h /usr/include/sys/wait.h
siod.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
siod.o: /usr/include/zlib.h /usr/include/zconf.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/limits.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/syslimits.h
siod.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
siod.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
siod.o: /usr/include/sys/stat.h /usr/include/bits/stat.h /usr/include/fcntl.h
siod.o: /usr/include/bits/fcntl.h /usr/include/bits/fcntl-linux.h
siod.o: /usr/include/sys/ipc.h /usr/include/bits/ipctypes.h
siod.o: /usr/include/bits/ipc.h /usr/include/sys/shm.h
siod.o: /usr/include/bits/shm.h /usr/lib/../include/c++/8/string
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++config.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/os_defines.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/cpu_defines.h
siod.o: /usr/lib/../include/c++/8/bits/stringfwd.h
siod.o: /usr/lib/../include/c++/8/bits/memoryfwd.h
siod.o: /usr/lib/../include/c++/8/bits/char_traits.h
siod.o: /usr/lib/../include/c++/8/bits/stl_algobase.h
siod.o: /usr/lib/../include/c++/8/bits/functexcept.h
siod.o: /usr/lib/../include/c++/8/bits/exception_defines.h
siod.o: /usr/lib/../include/c++/8/bits/cpp_type_traits.h
siod.o: /usr/lib/../include/c++/8/ext/type_traits.h
siod.o: /usr/lib/../include/c++/8/ext/numeric_traits.h
siod.o: /usr/lib/../include/c++/8/bits/stl_pair.h
siod.o: /usr/lib/../include/c++/8/bits/move.h
siod.o: /usr/lib/../include/c++/8/bits/concept_check.h
siod.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_types.h
siod.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_funcs.h
siod.o: /usr/lib/../include/c++/8/debug/assertions.h
siod.o: /usr/lib/../include/c++/8/bits/stl_iterator.h
siod.o: /usr/lib/../include/c++/8/bits/ptr_traits.h
siod.o: /usr/lib/../include/c++/8/debug/debug.h
siod.o: /usr/lib/../include/c++/8/bits/predefined_ops.h
siod.o: /usr/lib/../include/c++/8/bits/postypes.h
siod.o: /usr/lib/../include/c++/8/cwchar /usr/include/wchar.h
siod.o: /usr/include/bits/wchar.h /usr/include/bits/types/wint_t.h
siod.o: /usr/include/bits/types/mbstate_t.h
siod.o: /usr/lib/../include/c++/8/bits/allocator.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++allocator.h
siod.o: /usr/lib/../include/c++/8/ext/new_allocator.h
siod.o: /usr/lib/../include/c++/8/new /usr/lib/../include/c++/8/exception
siod.o: /usr/lib/../include/c++/8/bits/exception.h
siod.o: /usr/lib/../include/c++/8/bits/localefwd.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++locale.h
siod.o: /usr/lib/../include/c++/8/clocale /usr/include/locale.h
siod.o: /usr/include/bits/locale.h /usr/lib/../include/c++/8/iosfwd
siod.o: /usr/lib/../include/c++/8/cctype /usr/include/ctype.h
siod.o: /usr/lib/../include/c++/8/bits/ostream_insert.h
siod.o: /usr/lib/../include/c++/8/bits/cxxabi_forced.h
siod.o: /usr/lib/../include/c++/8/bits/stl_function.h
siod.o: /usr/lib/../include/c++/8/backward/binders.h
siod.o: /usr/lib/../include/c++/8/bits/range_access.h
siod.o: /usr/lib/../include/c++/8/bits/basic_string.h
siod.o: /usr/lib/../include/c++/8/ext/atomicity.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr-default.h
siod.o: /usr/include/pthread.h /usr/include/sched.h
siod.o: /usr/include/bits/types/struct_timespec.h /usr/include/bits/sched.h
siod.o: /usr/include/bits/types/struct_sched_param.h
siod.o: /usr/include/bits/cpu-set.h /usr/include/bits/pthreadtypes.h
siod.o: /usr/include/bits/thread-shared-types.h
siod.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/bits/setjmp.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/atomic_word.h
siod.o: /usr/lib/../include/c++/8/ext/alloc_traits.h
siod.o: /usr/lib/../include/c++/8/bits/basic_string.tcc
siod.o: /usr/lib/../include/c++/8/iostream /usr/lib/../include/c++/8/ostream
siod.o: /usr/lib/../include/c++/8/ios
siod.o: /usr/lib/../include/c++/8/bits/ios_base.h
siod.o: /usr/lib/../include/c++/8/bits/locale_classes.h
siod.o: /usr/lib/../include/c++/8/bits/locale_classes.tcc
siod.o: /usr/lib/../include/c++/8/stdexcept
siod.o: /usr/lib/../include/c++/8/streambuf
siod.o: /usr/lib/../include/c++/8/bits/streambuf.tcc
siod.o: /usr/lib/../include/c++/8/bits/basic_ios.h
siod.o: /usr/lib/../include/c++/8/bits/locale_facets.h
siod.o: /usr/lib/../include/c++/8/cwctype
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/ctype_base.h
siod.o: /usr/lib/../include/c++/8/bits/streambuf_iterator.h
siod.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/ctype_inline.h
siod.o: /usr/lib/../include/c++/8/bits/locale_facets.tcc
siod.o: /usr/lib/../include/c++/8/bits/basic_ios.tcc
siod.o: /usr/lib/../include/c++/8/bits/ostream.tcc
siod.o: /usr/lib/../include/c++/8/istream
siod.o: /usr/lib/../include/c++/8/bits/istream.tcc
siod.o: /usr/lib/../include/c++/8/vector
siod.o: /usr/lib/../include/c++/8/bits/stl_construct.h
siod.o: /usr/lib/../include/c++/8/bits/stl_uninitialized.h
siod.o: /usr/lib/../include/c++/8/bits/stl_vector.h
siod.o: /usr/lib/../include/c++/8/bits/stl_bvector.h
siod.o: /usr/lib/../include/c++/8/bits/vector.tcc
siod.o: /usr/lib/../include/c++/8/set
siod.o: /usr/lib/../include/c++/8/bits/stl_tree.h
siod.o: /usr/lib/../include/c++/8/bits/stl_set.h
siod.o: /usr/lib/../include/c++/8/bits/stl_multiset.h
siod.o: /usr/lib/../include/c++/8/iomanip /usr/lib/../include/c++/8/sstream
siod.o: /usr/lib/../include/c++/8/bits/sstream.tcc lfsr.h
siod.o: /usr/lib/../include/c++/8/map
siod.o: /usr/lib/../include/c++/8/bits/stl_map.h
siod.o: /usr/lib/../include/c++/8/bits/stl_multimap.h siod.h
splot.o: /usr/include/zlib.h /usr/include/zconf.h
splot.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stddef.h
splot.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/limits.h
splot.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/syslimits.h
splot.o: /usr/include/limits.h /usr/include/bits/libc-header-start.h
splot.o: /usr/include/features.h /usr/include/stdc-predef.h
splot.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
splot.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
splot.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
splot.o: /usr/include/linux/limits.h /usr/include/sys/types.h
splot.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
splot.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
splot.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
splot.o: /usr/include/endian.h /usr/include/bits/endian.h
splot.o: /usr/include/bits/byteswap.h /usr/include/bits/uintn-identity.h
splot.o: /usr/include/sys/select.h /usr/include/bits/select.h
splot.o: /usr/include/bits/types/sigset_t.h
splot.o: /usr/include/bits/types/__sigset_t.h
splot.o: /usr/include/bits/types/struct_timeval.h
splot.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdarg.h
splot.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
splot.o: /usr/include/bits/confname.h /usr/include/errno.h
splot.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
splot.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
splot.o: /usr/include/asm-generic/errno-base.h
splot.o: /usr/lib/../include/c++/8/string
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++config.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/os_defines.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/cpu_defines.h
splot.o: /usr/lib/../include/c++/8/bits/stringfwd.h
splot.o: /usr/lib/../include/c++/8/bits/memoryfwd.h
splot.o: /usr/lib/../include/c++/8/bits/char_traits.h
splot.o: /usr/lib/../include/c++/8/bits/stl_algobase.h
splot.o: /usr/lib/../include/c++/8/bits/functexcept.h
splot.o: /usr/lib/../include/c++/8/bits/exception_defines.h
splot.o: /usr/lib/../include/c++/8/bits/cpp_type_traits.h
splot.o: /usr/lib/../include/c++/8/ext/type_traits.h
splot.o: /usr/lib/../include/c++/8/ext/numeric_traits.h
splot.o: /usr/lib/../include/c++/8/bits/stl_pair.h
splot.o: /usr/lib/../include/c++/8/bits/move.h
splot.o: /usr/lib/../include/c++/8/bits/concept_check.h
splot.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_types.h
splot.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_funcs.h
splot.o: /usr/lib/../include/c++/8/debug/assertions.h
splot.o: /usr/lib/../include/c++/8/bits/stl_iterator.h
splot.o: /usr/lib/../include/c++/8/bits/ptr_traits.h
splot.o: /usr/lib/../include/c++/8/debug/debug.h
splot.o: /usr/lib/../include/c++/8/bits/predefined_ops.h
splot.o: /usr/lib/../include/c++/8/bits/postypes.h
splot.o: /usr/lib/../include/c++/8/cwchar /usr/include/wchar.h
splot.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
splot.o: /usr/include/bits/wchar.h /usr/include/bits/types/wint_t.h
splot.o: /usr/include/bits/types/mbstate_t.h
splot.o: /usr/include/bits/types/__mbstate_t.h
splot.o: /usr/include/bits/types/__FILE.h
splot.o: /usr/lib/../include/c++/8/bits/allocator.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++allocator.h
splot.o: /usr/lib/../include/c++/8/ext/new_allocator.h
splot.o: /usr/lib/../include/c++/8/new /usr/lib/../include/c++/8/exception
splot.o: /usr/lib/../include/c++/8/bits/exception.h
splot.o: /usr/lib/../include/c++/8/bits/localefwd.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++locale.h
splot.o: /usr/lib/../include/c++/8/clocale /usr/include/locale.h
splot.o: /usr/include/bits/locale.h /usr/lib/../include/c++/8/iosfwd
splot.o: /usr/lib/../include/c++/8/cctype /usr/include/ctype.h
splot.o: /usr/lib/../include/c++/8/bits/ostream_insert.h
splot.o: /usr/lib/../include/c++/8/bits/cxxabi_forced.h
splot.o: /usr/lib/../include/c++/8/bits/stl_function.h
splot.o: /usr/lib/../include/c++/8/backward/binders.h
splot.o: /usr/lib/../include/c++/8/bits/range_access.h
splot.o: /usr/lib/../include/c++/8/bits/basic_string.h
splot.o: /usr/lib/../include/c++/8/ext/atomicity.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr-default.h
splot.o: /usr/include/pthread.h /usr/include/sched.h
splot.o: /usr/include/bits/types/struct_timespec.h /usr/include/time.h
splot.o: /usr/include/bits/time.h /usr/include/bits/types/clock_t.h
splot.o: /usr/include/bits/types/struct_tm.h /usr/include/bits/sched.h
splot.o: /usr/include/bits/types/struct_sched_param.h
splot.o: /usr/include/bits/cpu-set.h /usr/include/bits/pthreadtypes.h
splot.o: /usr/include/bits/thread-shared-types.h
splot.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/bits/setjmp.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/atomic_word.h
splot.o: /usr/lib/../include/c++/8/ext/alloc_traits.h
splot.o: /usr/lib/../include/c++/8/bits/basic_string.tcc
splot.o: /usr/lib/../include/c++/8/vector
splot.o: /usr/lib/../include/c++/8/bits/stl_construct.h
splot.o: /usr/lib/../include/c++/8/bits/stl_uninitialized.h
splot.o: /usr/lib/../include/c++/8/bits/stl_vector.h
splot.o: /usr/lib/../include/c++/8/bits/stl_bvector.h
splot.o: /usr/lib/../include/c++/8/bits/vector.tcc
splot.o: /usr/lib/../include/c++/8/sstream /usr/lib/../include/c++/8/istream
splot.o: /usr/lib/../include/c++/8/ios
splot.o: /usr/lib/../include/c++/8/bits/ios_base.h
splot.o: /usr/lib/../include/c++/8/bits/locale_classes.h
splot.o: /usr/lib/../include/c++/8/bits/locale_classes.tcc
splot.o: /usr/lib/../include/c++/8/stdexcept
splot.o: /usr/lib/../include/c++/8/streambuf
splot.o: /usr/lib/../include/c++/8/bits/streambuf.tcc
splot.o: /usr/lib/../include/c++/8/bits/basic_ios.h
splot.o: /usr/lib/../include/c++/8/bits/locale_facets.h
splot.o: /usr/lib/../include/c++/8/cwctype
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/ctype_base.h
splot.o: /usr/lib/../include/c++/8/bits/streambuf_iterator.h
splot.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/ctype_inline.h
splot.o: /usr/lib/../include/c++/8/bits/locale_facets.tcc
splot.o: /usr/lib/../include/c++/8/bits/basic_ios.tcc
splot.o: /usr/lib/../include/c++/8/ostream
splot.o: /usr/lib/../include/c++/8/bits/ostream.tcc
splot.o: /usr/lib/../include/c++/8/bits/istream.tcc
splot.o: /usr/lib/../include/c++/8/bits/sstream.tcc siod.h
siod_progress.o: /usr/include/stdio.h /usr/include/bits/libc-header-start.h
siod_progress.o: /usr/include/features.h /usr/include/stdc-predef.h
siod_progress.o: /usr/include/sys/cdefs.h /usr/include/bits/wordsize.h
siod_progress.o: /usr/include/bits/long-double.h /usr/include/gnu/stubs.h
siod_progress.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stddef.h
siod_progress.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdarg.h
siod_progress.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
siod_progress.o: /usr/include/bits/types/__fpos_t.h
siod_progress.o: /usr/include/bits/types/__mbstate_t.h
siod_progress.o: /usr/include/bits/types/__fpos64_t.h
siod_progress.o: /usr/include/bits/types/__FILE.h
siod_progress.o: /usr/include/bits/types/FILE.h
siod_progress.o: /usr/include/bits/types/struct_FILE.h
siod_progress.o: /usr/include/bits/stdio_lim.h
siod_progress.o: /usr/include/bits/sys_errlist.h /usr/include/errno.h
siod_progress.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
siod_progress.o: /usr/include/asm/errno.h /usr/include/asm-generic/errno.h
siod_progress.o: /usr/include/asm-generic/errno-base.h /usr/include/string.h
siod_progress.o: /usr/include/strings.h /usr/include/sys/types.h
siod_progress.o: /usr/include/bits/types/clockid_t.h
siod_progress.o: /usr/include/bits/types/time_t.h
siod_progress.o: /usr/include/bits/types/timer_t.h
siod_progress.o: /usr/include/bits/stdint-intn.h /usr/include/endian.h
siod_progress.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
siod_progress.o: /usr/include/bits/uintn-identity.h /usr/include/sys/select.h
siod_progress.o: /usr/include/bits/select.h
siod_progress.o: /usr/include/bits/types/sigset_t.h
siod_progress.o: /usr/include/bits/types/__sigset_t.h
siod_progress.o: /usr/include/bits/types/struct_timeval.h
siod_progress.o: /usr/include/sys/ipc.h /usr/include/bits/ipctypes.h
siod_progress.o: /usr/include/bits/ipc.h /usr/include/sys/shm.h
siod_progress.o: /usr/include/bits/shm.h siod.h
siod_progress.o: /usr/lib/../include/c++/8/string
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++config.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/os_defines.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/cpu_defines.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stringfwd.h
siod_progress.o: /usr/lib/../include/c++/8/bits/memoryfwd.h
siod_progress.o: /usr/lib/../include/c++/8/bits/char_traits.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_algobase.h
siod_progress.o: /usr/lib/../include/c++/8/bits/functexcept.h
siod_progress.o: /usr/lib/../include/c++/8/bits/exception_defines.h
siod_progress.o: /usr/lib/../include/c++/8/bits/cpp_type_traits.h
siod_progress.o: /usr/lib/../include/c++/8/ext/type_traits.h
siod_progress.o: /usr/lib/../include/c++/8/ext/numeric_traits.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_pair.h
siod_progress.o: /usr/lib/../include/c++/8/bits/move.h
siod_progress.o: /usr/lib/../include/c++/8/bits/concept_check.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_types.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_funcs.h
siod_progress.o: /usr/lib/../include/c++/8/debug/assertions.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_iterator.h
siod_progress.o: /usr/lib/../include/c++/8/bits/ptr_traits.h
siod_progress.o: /usr/lib/../include/c++/8/debug/debug.h
siod_progress.o: /usr/lib/../include/c++/8/bits/predefined_ops.h
siod_progress.o: /usr/lib/../include/c++/8/bits/postypes.h
siod_progress.o: /usr/lib/../include/c++/8/cwchar /usr/include/wchar.h
siod_progress.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
siod_progress.o: /usr/include/bits/wchar.h /usr/include/bits/types/wint_t.h
siod_progress.o: /usr/include/bits/types/mbstate_t.h
siod_progress.o: /usr/lib/../include/c++/8/bits/allocator.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++allocator.h
siod_progress.o: /usr/lib/../include/c++/8/ext/new_allocator.h
siod_progress.o: /usr/lib/../include/c++/8/new
siod_progress.o: /usr/lib/../include/c++/8/exception
siod_progress.o: /usr/lib/../include/c++/8/bits/exception.h
siod_progress.o: /usr/lib/../include/c++/8/bits/localefwd.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++locale.h
siod_progress.o: /usr/lib/../include/c++/8/clocale /usr/include/locale.h
siod_progress.o: /usr/include/bits/locale.h /usr/lib/../include/c++/8/iosfwd
siod_progress.o: /usr/lib/../include/c++/8/cctype /usr/include/ctype.h
siod_progress.o: /usr/lib/../include/c++/8/bits/ostream_insert.h
siod_progress.o: /usr/lib/../include/c++/8/bits/cxxabi_forced.h
siod_progress.o: /usr/lib/../include/c++/8/bits/stl_function.h
siod_progress.o: /usr/lib/../include/c++/8/backward/binders.h
siod_progress.o: /usr/lib/../include/c++/8/bits/range_access.h
siod_progress.o: /usr/lib/../include/c++/8/bits/basic_string.h
siod_progress.o: /usr/lib/../include/c++/8/ext/atomicity.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr-default.h
siod_progress.o: /usr/include/pthread.h /usr/include/sched.h
siod_progress.o: /usr/include/bits/types/struct_timespec.h
siod_progress.o: /usr/include/time.h /usr/include/bits/time.h
siod_progress.o: /usr/include/bits/types/clock_t.h
siod_progress.o: /usr/include/bits/types/struct_tm.h
siod_progress.o: /usr/include/bits/sched.h
siod_progress.o: /usr/include/bits/types/struct_sched_param.h
siod_progress.o: /usr/include/bits/cpu-set.h /usr/include/bits/pthreadtypes.h
siod_progress.o: /usr/include/bits/thread-shared-types.h
siod_progress.o: /usr/include/bits/pthreadtypes-arch.h
siod_progress.o: /usr/include/bits/setjmp.h
siod_progress.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/atomic_word.h
siod_progress.o: /usr/lib/../include/c++/8/ext/alloc_traits.h
siod_progress.o: /usr/lib/../include/c++/8/bits/basic_string.tcc
siod-common.o: siod.h /usr/lib/../include/c++/8/string
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++config.h
siod-common.o: /usr/include/bits/wordsize.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/os_defines.h
siod-common.o: /usr/include/features.h /usr/include/stdc-predef.h
siod-common.o: /usr/include/sys/cdefs.h /usr/include/bits/long-double.h
siod-common.o: /usr/include/gnu/stubs.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/cpu_defines.h
siod-common.o: /usr/lib/../include/c++/8/bits/stringfwd.h
siod-common.o: /usr/lib/../include/c++/8/bits/memoryfwd.h
siod-common.o: /usr/lib/../include/c++/8/bits/char_traits.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_algobase.h
siod-common.o: /usr/lib/../include/c++/8/bits/functexcept.h
siod-common.o: /usr/lib/../include/c++/8/bits/exception_defines.h
siod-common.o: /usr/lib/../include/c++/8/bits/cpp_type_traits.h
siod-common.o: /usr/lib/../include/c++/8/ext/type_traits.h
siod-common.o: /usr/lib/../include/c++/8/ext/numeric_traits.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_pair.h
siod-common.o: /usr/lib/../include/c++/8/bits/move.h
siod-common.o: /usr/lib/../include/c++/8/bits/concept_check.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_types.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_iterator_base_funcs.h
siod-common.o: /usr/lib/../include/c++/8/debug/assertions.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_iterator.h
siod-common.o: /usr/lib/../include/c++/8/bits/ptr_traits.h
siod-common.o: /usr/lib/../include/c++/8/debug/debug.h
siod-common.o: /usr/lib/../include/c++/8/bits/predefined_ops.h
siod-common.o: /usr/lib/../include/c++/8/bits/postypes.h
siod-common.o: /usr/lib/../include/c++/8/cwchar /usr/include/wchar.h
siod-common.o: /usr/include/bits/libc-header-start.h
siod-common.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
siod-common.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stddef.h
siod-common.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdarg.h
siod-common.o: /usr/include/bits/wchar.h /usr/include/bits/types/wint_t.h
siod-common.o: /usr/include/bits/types/mbstate_t.h
siod-common.o: /usr/include/bits/types/__mbstate_t.h
siod-common.o: /usr/include/bits/types/__FILE.h
siod-common.o: /usr/lib/../include/c++/8/bits/allocator.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++allocator.h
siod-common.o: /usr/lib/../include/c++/8/ext/new_allocator.h
siod-common.o: /usr/lib/../include/c++/8/new
siod-common.o: /usr/lib/../include/c++/8/exception
siod-common.o: /usr/lib/../include/c++/8/bits/exception.h
siod-common.o: /usr/lib/../include/c++/8/bits/localefwd.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/c++locale.h
siod-common.o: /usr/lib/../include/c++/8/clocale /usr/include/locale.h
siod-common.o: /usr/include/bits/locale.h /usr/lib/../include/c++/8/iosfwd
siod-common.o: /usr/lib/../include/c++/8/cctype /usr/include/ctype.h
siod-common.o: /usr/include/bits/types.h /usr/include/bits/typesizes.h
siod-common.o: /usr/include/endian.h /usr/include/bits/endian.h
siod-common.o: /usr/include/bits/byteswap.h
siod-common.o: /usr/include/bits/uintn-identity.h
siod-common.o: /usr/lib/../include/c++/8/bits/ostream_insert.h
siod-common.o: /usr/lib/../include/c++/8/bits/cxxabi_forced.h
siod-common.o: /usr/lib/../include/c++/8/bits/stl_function.h
siod-common.o: /usr/lib/../include/c++/8/backward/binders.h
siod-common.o: /usr/lib/../include/c++/8/bits/range_access.h
siod-common.o: /usr/lib/../include/c++/8/bits/basic_string.h
siod-common.o: /usr/lib/../include/c++/8/ext/atomicity.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/gthr-default.h
siod-common.o: /usr/include/pthread.h /usr/include/sched.h
siod-common.o: /usr/include/bits/types/time_t.h
siod-common.o: /usr/include/bits/types/struct_timespec.h /usr/include/time.h
siod-common.o: /usr/include/bits/time.h /usr/include/bits/types/clock_t.h
siod-common.o: /usr/include/bits/types/struct_tm.h /usr/include/bits/sched.h
siod-common.o: /usr/include/bits/types/struct_sched_param.h
siod-common.o: /usr/include/bits/cpu-set.h /usr/include/bits/pthreadtypes.h
siod-common.o: /usr/include/bits/thread-shared-types.h
siod-common.o: /usr/include/bits/pthreadtypes-arch.h
siod-common.o: /usr/include/bits/setjmp.h
siod-common.o: /usr/lib/../include/c++/8/x86_64-redhat-linux/bits/atomic_word.h
siod-common.o: /usr/lib/../include/c++/8/ext/alloc_traits.h
siod-common.o: /usr/lib/../include/c++/8/bits/basic_string.tcc
