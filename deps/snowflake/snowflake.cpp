#include "snowflake.h"

#ifdef _WIN32
int SnowFlake::gtod(struct timeval * tp, struct timezone * tzp) {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime );
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#else
int SnowFlake::gtod(struct timeval * tp, struct timezone * tzp) {
    return gettimeofday(tp, tzp);
}
#endif

uint64_t SnowFlake::getNextMill() {
    uint64_t mill = getNewstmp();
    while (mill <= lastStmp) {
        mill = getNewstmp();
    }
    return mill;
}

uint64_t SnowFlake::getNewstmp() {
    struct timeval tv;
    SnowFlake::gtod(&tv, NULL);

    uint64_t time = tv.tv_usec;
    time /= 1000;
    time += (((long long)tv.tv_sec) * 1000);
    return time;
}

SnowFlake::SnowFlake(int datacenter_Id, int machine_Id) {
    if ((uint64_t)datacenter_Id > max_datacenter_num_ || datacenter_Id < 0) {
        qDebug() << "datacenterId can't be greater than max_datacenter_num_ or less than 0";
        exit(0);
    }
    if ((uint64_t)machine_Id > max_machine_num_ || machine_Id < 0) {
        qDebug() << "machineId can't be greater than max_machine_num_or less than 0";
        exit(0);
    }
    datacenterId = datacenter_Id;
    machineId = machine_Id;
    sequence = 0L;
    lastStmp = 0L;
}

SnowFlake* SnowFlake::get() {
    if(!_self) _self = new SnowFlake(1, 1);
    return _self;
}

SnowFlake::Id SnowFlake::nextId() {
    std::unique_lock<std::mutex> lock(mutex_);
    uint64_t currStmp = getNewstmp();
    if (currStmp < lastStmp) {
        qDebug() << "Clock moved backwards.  Refusing to generate id";
        exit(0);
    }

    if (currStmp == lastStmp) {
        sequence = (sequence + 1) & max_sequence_num_;
        if (sequence == 0) {
            currStmp = getNextMill();
        }
    } else {
        sequence = 0;
    }
    lastStmp = currStmp;
    return (currStmp - start_stmp_) << timestmp_left
            | datacenterId << datacenter_left
            | machineId << machine_left
            | sequence;
} 