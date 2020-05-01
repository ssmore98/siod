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
siod.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
siod.o: /usr/include/bits/floatn.h /usr/include/bits/floatn-common.h
siod.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
siod.o: /usr/include/bits/types/clockid_t.h /usr/include/bits/types/time_t.h
siod.o: /usr/include/bits/types/timer_t.h /usr/include/bits/stdint-intn.h
siod.o: /usr/include/endian.h /usr/include/bits/endian.h
siod.o: /usr/include/bits/byteswap.h /usr/include/bits/uintn-identity.h
siod.o: /usr/include/sys/select.h /usr/include/bits/select.h
siod.o: /usr/include/bits/types/sigset_t.h
siod.o: /usr/include/bits/types/__sigset_t.h
siod.o: /usr/include/bits/types/struct_timeval.h
siod.o: /usr/include/bits/types/struct_timespec.h
siod.o: /usr/include/bits/pthreadtypes.h
siod.o: /usr/include/bits/thread-shared-types.h
siod.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/alloca.h
siod.o: /usr/include/bits/stdlib-float.h /usr/include/string.h
siod.o: /usr/include/bits/types/locale_t.h
siod.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
siod.o: /usr/include/errno.h /usr/include/bits/errno.h
siod.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
siod.o: /usr/include/asm-generic/errno.h
siod.o: /usr/include/asm-generic/errno-base.h /usr/include/time.h
siod.o: /usr/include/bits/time.h /usr/include/bits/types/struct_tm.h
siod.o: /usr/include/bits/types/struct_itimerspec.h
siod.o: /usr/include/scsi/sg_cmds.h /usr/include/scsi/sg_cmds_basic.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/stdbool.h
siod.o: /usr/include/scsi/sg_cmds_extra.h /usr/include/scsi/sg_cmds_mmc.h
siod.o: /usr/include/scsi/sg_lib.h /usr/include/scsi/sg_io_linux.h
siod.o: /usr/include/scsi/sg_lib.h /usr/include/scsi/sg_linux_inc.h
siod.o: /usr/include/scsi/sg.h /usr/include/scsi/scsi.h /usr/include/signal.h
siod.o: /usr/include/bits/signum.h /usr/include/bits/signum-generic.h
siod.o: /usr/include/bits/types/sig_atomic_t.h
siod.o: /usr/include/bits/types/siginfo_t.h
siod.o: /usr/include/bits/types/__sigval_t.h /usr/include/bits/siginfo-arch.h
siod.o: /usr/include/bits/siginfo-consts.h /usr/include/bits/types/sigval_t.h
siod.o: /usr/include/bits/types/sigevent_t.h
siod.o: /usr/include/bits/sigevent-consts.h /usr/include/bits/sigaction.h
siod.o: /usr/include/bits/sigcontext.h /usr/include/bits/types/stack_t.h
siod.o: /usr/include/sys/ucontext.h /usr/include/bits/sigstack.h
siod.o: /usr/include/bits/ss_flags.h
siod.o: /usr/include/bits/types/struct_sigstack.h
siod.o: /usr/include/bits/sigthread.h /usr/include/unistd.h
siod.o: /usr/include/bits/posix_opt.h /usr/include/bits/environments.h
siod.o: /usr/include/bits/confname.h /usr/include/bits/getopt_posix.h
siod.o: /usr/include/bits/getopt_core.h /usr/include/sys/time.h
siod.o: /usr/include/sys/wait.h /usr/include/zlib.h /usr/include/zconf.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/limits.h
siod.o: /usr/lib/gcc/x86_64-redhat-linux/8/include/syslimits.h
siod.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
siod.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
siod.o: /usr/include/bits/posix2_lim.h /usr/include/sys/stat.h
siod.o: /usr/include/bits/stat.h /usr/include/fcntl.h
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
siod.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
siod.o: /usr/include/bits/types/struct_sched_param.h
siod.o: /usr/include/bits/cpu-set.h /usr/include/bits/setjmp.h
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
siod_progress.o: /usr/include/bits/types/locale_t.h
siod_progress.o: /usr/include/bits/types/__locale_t.h /usr/include/strings.h
siod_progress.o: /usr/include/sys/types.h /usr/include/bits/types/clock_t.h
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
siod_progress.o: /usr/include/bits/types/struct_timespec.h
siod_progress.o: /usr/include/bits/pthreadtypes.h
siod_progress.o: /usr/include/bits/thread-shared-types.h
siod_progress.o: /usr/include/bits/pthreadtypes-arch.h /usr/include/sys/ipc.h
siod_progress.o: /usr/include/bits/ipctypes.h /usr/include/bits/ipc.h
siod_progress.o: /usr/include/sys/shm.h /usr/include/bits/shm.h siod.h
