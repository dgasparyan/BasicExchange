#include "Exchange.h"

Exchange::Exchange(EventQueue& eventQueue) : eventQueue_(eventQueue){
}

Exchange::~Exchange() {
}

void Exchange::processEvent(const std::string& event) {
    // eventParser_.parse(event);
}