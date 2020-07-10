#include "pi_heap.h"
#include "pi_lib.h"
#include "game_service/PiScriptingEngine/v8-profiler/heap_snapshot.h"
#include "game_service/PiScriptingEngine/v8-profiler/heap_output_stream.h"

using v8::ActivityControl;
using v8::Array;
using v8::Function;
using v8::Handle;
using v8::HeapSnapshot;
using v8::Integer;
using v8::Local;
using v8::Object;
using v8::SnapshotObjectId;
using v8::String;
using v8::TryCatch;
using v8::Value;
using v8::HeapSpaceStatistics;
using v8::HeapStatistics;

static int kMbSize = 1024 * 1024;


class ActivityControlAdapter : public ActivityControl
{
public:
	ActivityControlAdapter(Local<Value> progress)
		: reportProgress(Local<Function>::Cast(progress)),
		abort(Nan::False())
	{
	}

	ControlOption ReportProgressValue(int done, int total)
	{
		Local<Value> argv[2] = {
			Nan::New<Integer>(done),
			Nan::New<Integer>(total)
		};

		TryCatch try_catch;
		abort = reportProgress->Call(Nan::GetCurrentContext()->Global(), 2, argv);

		if (try_catch.HasCaught())
		{
			Nan::ThrowError(try_catch.Exception());
			return kAbort;
		}

		return abort->IsFalse() ? kAbort : kContinue;
	}

private:
	Local<Function> reportProgress;
	Local<Value> abort;
};


NAN_METHOD(HeapBinding::TakeSnapshot)
{
	ActivityControlAdapter* control = new ActivityControlAdapter(info[1]);
#if (NODE_MODULE_VERSION < 0x000F)
	Local<String> title = info[0]->ToString();
#endif

#if (NODE_MODULE_VERSION > 0x002C)
	const HeapSnapshot* snapshot = v8::Isolate::GetCurrent()->GetHeapProfiler()->TakeHeapSnapshot(control);
#elif (NODE_MODULE_VERSION > 0x000B)
	const HeapSnapshot* snapshot = v8::Isolate::GetCurrent()->GetHeapProfiler()->TakeHeapSnapshot(title, control);
#else
	const HeapSnapshot* snapshot = v8::HeapProfiler::TakeSnapshot(title, HeapSnapshot::kFull, control);
#endif

	info.GetReturnValue().Set(nodex::Snapshot::New(snapshot));
}

NAN_METHOD(HeapBinding::StartTrackingHeapObjects)
{
#if (NODE_MODULE_VERSION > 0x000B)
	v8::Isolate::GetCurrent()->GetHeapProfiler()->StartTrackingHeapObjects();
#else
	v8::HeapProfiler::StartHeapObjectsTracking();
#endif

	return;
}

NAN_METHOD(HeapBinding::StopTrackingHeapObjects)
{
#if (NODE_MODULE_VERSION > 0x000B)
	v8::Isolate::GetCurrent()->GetHeapProfiler()->StopTrackingHeapObjects();
#else
	v8::HeapProfiler::StopHeapObjectsTracking();
#endif
}

NAN_METHOD(HeapBinding::GetHeapStats)
{
	Local<Function> iterator = Local<Function>::Cast(info[0]);
	Local<Function> callback = Local<Function>::Cast(info[1]);

	nodex::OutputStreamAdapter* stream = new nodex::OutputStreamAdapter(iterator, callback);
#if (NODE_MODULE_VERSION > 0x000B)
	SnapshotObjectId ID = v8::Isolate::GetCurrent()->GetHeapProfiler()->GetHeapStats(stream);
#else
	SnapshotObjectId ID = v8::HeapProfiler::PushHeapObjectsStats(stream);
#endif
	info.GetReturnValue().Set(Nan::New<Integer>(ID));
}

NAN_METHOD(HeapBinding::GetObjectByHeapObjectId)
{
	SnapshotObjectId id = info[0]->Uint32Value();
	Local<Value> object;
#if (NODE_MODULE_VERSION > 0x000B)
	object = v8::Isolate::GetCurrent()->GetHeapProfiler()->FindObjectById(id);
#else
	Local<Object> snapshots = Local<Object>::Cast(info.This()->Get(Nan::New<String>("snapshots").ToLocalChecked()));
	Local<Object> snapshot;

	Local<Array> names = Nan::GetOwnPropertyNames(snapshots).ToLocalChecked();
	uint32_t length = names->Length();
	if (length == 0) return;

	for (uint32_t i = 0; i < length; ++i)
	{
		Local<Value> name = Nan::Get(names, i).ToLocalChecked();
		uint32_t uid = Nan::To<Integer>(name).ToLocalChecked()->Value();
		if (uid >= id)
		{
			snapshot = Nan::To<Object>(Nan::Get(snapshots, uid).ToLocalChecked()).ToLocalChecked();

			Local<Value> argv[] = { info[0] };
			Local<Object> graph_node = Function::Cast(*snapshot->Get(Nan::New<String>("getNodeById").ToLocalChecked()))
				->Call(snapshot, 1, argv)->ToObject();
			object = Function::Cast(*graph_node->Get(Nan::New<String>("getHeapValue").ToLocalChecked()))
				->Call(graph_node, 0, NULL);
			break;
		}
	}
#endif

	if (object.IsEmpty())
	{
		return;
	}
	else if (object->IsObject()
			 || object->IsNumber()
			 || object->IsString()
#if (NODE_MODULE_VERSION > 0x000B)
			 || object->IsSymbol()
#endif
			 || object->IsBoolean())
	{
		info.GetReturnValue().Set(object);
	}
	else
	{
		info.GetReturnValue().Set(Nan::New<String>("Preview is not available").ToLocalChecked());
	}
}

NAN_METHOD(HeapBinding::GetHeapObjectId)
{
	if (info[0].IsEmpty()) return;

	SnapshotObjectId id;
#if (NODE_MODULE_VERSION > 0x000B)
	id = v8::Isolate::GetCurrent()->GetHeapProfiler()->GetObjectId(info[0]);
#else
	id = v8::HeapProfiler::GetSnapshotObjectId(info[0]);
#endif

	info.GetReturnValue().Set(Nan::New<Integer>(id));
}

NAN_METHOD(HeapBinding::GetHeapSpaceStatistics)
{
	size_t number_of_heap_spaces = info.GetIsolate()->NumberOfHeapSpaces();

	pi_log_print(LOG_INFO, "Heap Space Statistics");
	for (size_t i = 0; i < number_of_heap_spaces; i++)
	{
		HeapSpaceStatistics s;
		info.GetIsolate()->GetHeapSpaceStatistics(&s, i);
		pi_log_print(LOG_INFO, "index: %d", i);
		pi_log_print(LOG_INFO, "space_name: %s", s.space_name());
		pi_log_print(LOG_INFO, "space_size: %dK %.2fM", s.space_size(), static_cast<double>(s.space_size()) / kMbSize);
		pi_log_print(LOG_INFO, "space_used_size: %dK %.2fM", s.space_used_size(), static_cast<double>(s.space_used_size()) / kMbSize);
		pi_log_print(LOG_INFO, "space_available_size: %dK %.2fM", s.space_available_size(), static_cast<double>(s.space_available_size()) / kMbSize);
		pi_log_print(LOG_INFO, "physical_space_size: %dK %.2fM", s.physical_space_size(), static_cast<double>(s.physical_space_size()) / kMbSize);
	}
	info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
}

NAN_METHOD(HeapBinding::GetHeapStatistics)
{
	HeapStatistics s;
	info.GetIsolate()->GetHeapStatistics(&s);

	pi_log_print(LOG_INFO, "Heap Statistics");
	pi_log_print(LOG_INFO, "total_heap_size: %dK %.2fM", s.total_available_size(), static_cast<double>(s.total_available_size()) / kMbSize);
	pi_log_print(LOG_INFO, "total_heap_size_executable: %dK %.2fM", s.total_heap_size_executable(), static_cast<double>(s.total_heap_size_executable()) / kMbSize);
	pi_log_print(LOG_INFO, "total_physical_size: %dK %.2fM", s.total_physical_size(), static_cast<double>(s.total_physical_size()) / kMbSize);
	pi_log_print(LOG_INFO, "total_available_size: %dK %.2fM", s.total_available_size(), static_cast<double>(s.total_available_size()) / kMbSize);
	pi_log_print(LOG_INFO, "used_heap_size: %dK %.2fM", s.used_heap_size(), static_cast<double>(s.used_heap_size()) / kMbSize);
	pi_log_print(LOG_INFO, "heap_size_limit: %dK %.2fM", s.heap_size_limit(), static_cast<double>(s.heap_size_limit()) / kMbSize);
	pi_log_print(LOG_INFO, "malloced_memory: %dK %.2fM", s.malloced_memory(), static_cast<double>(s.malloced_memory()) / kMbSize);
	pi_log_print(LOG_INFO, "peak_malloced_memory: %dK %.2fM", s.peak_malloced_memory(), static_cast<double>(s.peak_malloced_memory()) / kMbSize);
	pi_log_print(LOG_INFO, "does_zap_garbage: %d", s.does_zap_garbage());
	info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
}