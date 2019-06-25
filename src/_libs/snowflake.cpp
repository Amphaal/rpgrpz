#include "snowflake.h"

uint64_t SnowFlake::getNextMill() {
    uint64_t mill = getNewstmp();
    while (mill <= lastStmp) {
        mill = getNewstmp();
    }
    return mill;
}

uint64_t SnowFlake::getNewstmp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint64_t time = tv.tv_usec;
    time /= 1000;
    time += (tv.tv_sec * 1000);
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

snowflake_uid SnowFlake::nextId() {
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