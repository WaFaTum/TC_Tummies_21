#include "open62541_server.h"

open62541_server::open62541_server(int ns, sighandler_t __handler){
    this->nodesNamespace = ns;
    signal(SIGINT, __handler);
    signal(SIGTERM, __handler);
    this->server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(this->server));
    UA_ServerConfig* config = UA_Server_getConfig(this->server);
    config->verifyRequestTimestamp = UA_RULEHANDLING_ACCEPT;
}

void open62541_server::startServer(volatile UA_Boolean* run){
    UA_Server_run(this->server, run);
}
void open62541_server::addVariableNode(char* nodeStringID, char* nodeName, char* displayName, int initVal){
    UA_Int32 initValue = initVal;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayName);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&attr.value, &initValue, &UA_TYPES[UA_TYPES_INT32]);
    int* newVar = new int;
    *newVar = initVal;
    nodesVector.push_back(newVar);
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(this->nodesNamespace, this->nodesVector.size());
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(this->nodesNamespace, nodeName);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(this->server, currentNodeId, parentNodeId,
                              parentReferenceNodeId, currentName,
                              variableTypeNodeId, attr, NULL, NULL);
}
void open62541_server::updateNodeData(UA_NodeId nodeId){
    int readValue;

    UA_Int32 sensorReading = *(this->nodesVector[nodeId.identifier.numeric-1]);
    UA_Variant value;
    UA_Variant_setScalar(&value, &sensorReading, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(this->nodesNamespace, nodeId.identifier.numeric);
    UA_Server_writeValue(this->server, currentNodeId, value);
}

void open62541_server::addValueCallbackToSensorVariable(int nodeId) {
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(this->nodesNamespace, nodeId);
    UA_ValueCallback callback ;
    //callback.onRead = beforeRead;
    //callback.onWrite = afterWrite;
    UA_Server_setVariableNode_valueCallback(this->server, currentNodeId, callback);
}

static void beforeRead(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    //updateNodeData(server, *nodeid);
}

static void
afterWrite(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                //"The variable was updated");
}