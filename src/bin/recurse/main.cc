// Copyright (C) 2009  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

// $Id$

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>

#include <string>
#include <iostream>

#include <boost/foreach.hpp>

#include <asiolink/asiolink.h>

#include <exceptions/exceptions.h>

#include <dns/buffer.h>
#include <dns/message.h>
#include <dns/messagerenderer.h>

#include <cc/session.h>
#include <cc/data.h>
#include <config/ccsession.h>

#include <xfr/xfrout_client.h>

#include <auth/change_user.h>
#include <auth/common.h>

#include <recurse/spec_config.h>
#include <recurse/recursor.h>

#include <log/dummylog.h>

using namespace std;
using namespace isc::cc;
using namespace isc::config;
using namespace isc::data;
using isc::log::dlog;
using namespace asiolink;

namespace {

// Default port current 5300 for testing purposes
static const string PROGRAM = "Recurse";

IOService io_service;
static Recursor *recursor;

ConstElementPtr
my_config_handler(ConstElementPtr new_config) {
    return (recursor->updateConfig(new_config));
}

ConstElementPtr
my_command_handler(const string& command, ConstElementPtr args) {
    ConstElementPtr answer = createAnswer();

    if (command == "print_message") {
        cout << args << endl;
        /* let's add that message to our answer as well */
        answer = createAnswer(0, args);
    } else if (command == "shutdown") {
        io_service.stop();
    }

    return (answer);
}

void
usage() {
    cerr << "Usage:  b10-recurse [-u user] [-v]" << endl;
    cerr << "\t-u: change process UID to the specified user" << endl;
    cerr << "\t-v: verbose output" << endl;
    exit(1);
}
} // end of anonymous namespace

int
main(int argc, char* argv[]) {
    isc::log::dprefix = "b10-recurse";
    int ch;
    const char* uid = NULL;

    while ((ch = getopt(argc, argv, "u:v")) != -1) {
        switch (ch) {
        case 'u':
            uid = optarg;
            break;
        case 'v':
            isc::log::denabled = true;
            break;
        case '?':
        default:
            usage();
        }
    }

    if (argc - optind > 0) {
        usage();
    }

    if (isc::log::denabled) { // Show the command line
        string cmdline("Command line:");
        for (int i = 0; i < argc; ++ i) {
            cmdline = cmdline + " " + argv[i];
        }
        dlog(cmdline);
    }

    int ret = 0;

    Session* cc_session = NULL;
    ModuleCCSession* config_session = NULL;
    try {
        string specfile;
        if (getenv("B10_FROM_BUILD")) {
            specfile = string(getenv("B10_FROM_BUILD")) +
                "/src/bin/recurse/recurse.spec";
        } else {
            specfile = string(RECURSE_SPECFILE_LOCATION);
        }

        recursor = new Recursor();
        dlog("Server created.");

        SimpleCallback* checkin = recursor->getCheckinProvider();
        DNSLookup* lookup = recursor->getDNSLookupProvider();
        DNSAnswer* answer = recursor->getDNSAnswerProvider();

        DNSService dns_service(io_service, checkin, lookup, answer);

        recursor->setDNSService(dns_service);
        dlog("IOService created.");

        cc_session = new Session(io_service.get_io_service());
        dlog("Configuration session channel created.");

        config_session = new ModuleCCSession(specfile, *cc_session,
                                             my_config_handler,
                                             my_command_handler);
        dlog("Configuration channel established.");

        // FIXME: This does not belong here, but inside Boss
        if (uid != NULL) {
            changeUser(uid);
        }

        recursor->setConfigSession(config_session);
        dlog("Config loaded");

        dlog("Server started.");
        io_service.run();
    } catch (const std::exception& ex) {
        dlog(string("Server failed: ") + ex.what());
        ret = 1;
    }

    delete config_session;
    delete cc_session;
    delete recursor;

    return (ret);
}
