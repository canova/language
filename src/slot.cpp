#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern mObject *objalloc();

mObject* getSlot(mObject *self, char *slot, int followChain)
{
	printf ("Getting slot for %s\n", slot);
	mObject *obj = self;
	while (obj) {
		printf("obj %d\n", obj);
		slotmap::iterator it = obj->slots->find(slot);
		if (it == obj->slots->end()) {
			if (followChain) { obj = obj->prototype; continue; }
			else return NULL;
		}
		printf("Found slot %d\n", (*it).second);
		return (*it).second;
	}
	return NULL;
}

void putSlot(mObject *self, char *slot, mObject *value)
{
	printf ("Putting slot for %s\n", slot);
	(*self->slots)[slot] = value;
}

mObject* newobj(mObject *prototype)
{
	mObject *obj = new mObject;
	mObject *init = getSlot(prototype, (char *)"init", 1);
	obj->prototype = prototype;
	obj->slots = new slotmap();
	return obj;
}

#ifdef __cplusplus
}
#endif
