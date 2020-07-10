#ifndef SCRIPT_ENGINE_PI_HEAP_H_
#define SCRIPT_ENGINE_PI_HEAP_H_

#include "v8.h"
#include "nan/nan.h"


class HeapBinding
{
public:
	static NAN_METHOD(TakeSnapshot);
	static NAN_METHOD(StartTrackingHeapObjects);
	static NAN_METHOD(StopTrackingHeapObjects);
	static NAN_METHOD(GetHeapStats);
	static NAN_METHOD(GetObjectByHeapObjectId);
	static NAN_METHOD(GetHeapObjectId);
	static NAN_METHOD(GetHeapSpaceStatistics);
	static NAN_METHOD(GetHeapStatistics);
};


#endif