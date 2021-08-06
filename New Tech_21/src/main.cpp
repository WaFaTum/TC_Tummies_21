#include <iostream>
#include "open62541_server.h"

#define nodes_namespace 1

static volatile UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}
int main(){

    open62541_server myServer(nodes_namespace, stopHandler);
    myServer.addVariableNode("test ndoe id", "test node name", "test node display name", 42);
    myServer.startServer(&running);
    
    return 0;
}