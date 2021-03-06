#include <util/stream/output.h>
#include <util/stream/null.h>
#include <util/system/compat.h>
#include <util/system/yassert.h>
#include <util/system/defaults.h>
#include <util/generic/singleton.h>

#if defined(_unix_)
#include <syslog.h>
#endif

#include "system.h"
#include "record.h"
#include "stream.h"

TSysLogBackend::TSysLogBackend(const char* ident, EFacility facility, int flags)
    : Ident(ident)
    , Facility(facility)
    , Flags(flags)
{
#if defined(_unix_)
    Y_ASSERT(TSYSLOG_LOCAL0 <= facility && facility <= TSYSLOG_LOCAL7);

    static const int f2sf[] = {
        LOG_LOCAL0,
        LOG_LOCAL1,
        LOG_LOCAL2,
        LOG_LOCAL3,
        LOG_LOCAL4,
        LOG_LOCAL5,
        LOG_LOCAL6,
        LOG_LOCAL7};

    int sysflags = LOG_NDELAY | LOG_PID;

    if (flags & LogPerror) {
        sysflags |= LOG_PERROR;
    }

    if (flags & LogCons) {
        sysflags |= LOG_CONS;
    }

    openlog(~Ident, sysflags, f2sf[(size_t)facility]);
#endif
}

TSysLogBackend::~TSysLogBackend() {
#if defined(_unix_)
    closelog();
#endif
}

void TSysLogBackend::WriteData(const TLogRecord& rec) {
#if defined(_unix_)
    syslog(TLogPriority2SyslogPriority(rec.Priority), "%.*s", (int)rec.Len, rec.Data);
#else
    Y_UNUSED(rec);
#endif
}

void TSysLogBackend::ReopenLog() {
}

int TSysLogBackend::TLogPriority2SyslogPriority(TLogPriority priority) {
    // trivial conversion
    return int(priority);
}

namespace {
    class TSysLogInstance: public TLog {
    public:
        inline TSysLogInstance()
            : TLog(new TStreamLogBackend(&Cnull))
        {
        }
    };
}

TLog& SysLogInstance() {
    return *Singleton<TSysLogInstance>();
}
