/*-
 * Copyright 2018 Aniket Pandey
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <atf-c.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

static pid_t pid;
static int status;
static struct pollfd fds[1];
static char pcregex[60];
static char bin[] = "/usr/bin/true";
static char argument[] = "sample-argument";
static char *arg[] = {bin, argument, NULL};


ATF_TC_WITH_CLEANUP(chdir_success);
ATF_TC_HEAD(chdir_success, tc)
{
	atf_tc_set_md_var(tc, "descr", "Tests the audit of a successful "
					"chdir(2) call");
}

ATF_TC_BODY(chdir_success, tc)
{
	/* Build an absolute path to the test-case directory */
	char dirpath[50];
	ATF_REQUIRE(getcwd(dirpath, sizeof(dirpath)) != NULL);
	snprintf(pcregex, 60, "chdir.*%s.*return,success", dirpath);

	FILE *pipefd = setup(fds, "pc");
	ATF_REQUIRE_EQ(0, chdir(dirpath));
	check_audit(fds, pcregex, pipefd);
}

ATF_TC_CLEANUP(chdir_success, tc)
{
	cleanup();
}


ATF_TC_WITH_CLEANUP(execve_success);
ATF_TC_HEAD(execve_success, tc)
{
	atf_tc_set_md_var(tc, "descr", "Tests the audit of a successful "
					"execve(2) call");
}

ATF_TC_BODY(execve_success, tc)
{
	const char *regex = "execve.*sample-argument.*Unknown error: 201";
	FILE *pipefd = setup(fds, "pc");

	ATF_REQUIRE((pid = fork()) != -1);
	if (pid) {
		ATF_REQUIRE(wait(&status) != -1);
		check_audit(fds, regex, pipefd);
	}
	else
		ATF_REQUIRE(execve(bin, arg, NULL) != -1);
}

ATF_TC_CLEANUP(execve_success, tc)
{
	cleanup();
}


ATF_TC_WITH_CLEANUP(execve_failure);
ATF_TC_HEAD(execve_failure, tc)
{
	atf_tc_set_md_var(tc, "descr", "Tests the audit of an unsuccessful "
					"execve(2) call");
}

ATF_TC_BODY(execve_failure, tc)
{
	const char *regex = "execve.*return,failure : Bad address";
	FILE *pipefd = setup(fds, "pc");

	ATF_REQUIRE((pid = fork()) != -1);
	if (pid) {
		ATF_REQUIRE(wait(&status) != -1);
		check_audit(fds, regex, pipefd);
	}
	else
		ATF_REQUIRE_EQ(-1, execve(bin, arg, (char *const *)(-1)));
}

ATF_TC_CLEANUP(execve_failure, tc)
{
	cleanup();
}


ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, chdir_success);
	ATF_TP_ADD_TC(tp, execve_success);
	ATF_TP_ADD_TC(tp, execve_failure);

	return (atf_no_error());
}