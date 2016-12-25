#include <map>

#ifdef __cplusplus
extern "C" {
#endif

struct mObject;
typedef std::map<char*, mObject*> slotmap;

struct mObject {
	struct mObject *prototype;
	slotmap *slots;
};

struct cstring {
	struct mObject *prototype;
	slotmap *slots;
	char *ptr;
	size_t len;
};

struct cinteger {
	struct mObject *prototype;
	slotmap *slots;
	uint64_t value;
};

struct cdouble {
	struct mObject *prototype;
	slotmap *slots;
	double value;
};

#ifdef __cplusplus
}
#endif
