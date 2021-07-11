#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include "I2C_CO2Sensor.h"

#include <signal.h>
#include <stdlib.h>

/*** updating variables manually ***/

int CO2Reading, fd;

static void
addSensorVariable(UA_Server *server, int nodeId, char* nodeName, char* displayName) {
    UA_Int32 now = 0;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "CO2 Sensor");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&attr.value, &now, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 15);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "current-time-value-callback");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
                              parentReferenceNodeId, currentName,
                              variableTypeNodeId, attr, NULL, NULL);

    updateSensorData(server);
}


static void
updateSensorData(UA_Server *server) {
    int readContinuous = CO2SensorReadContinous(&fd);
    if(readContinuous != -1)
        CO2Reading = readContinuous;
    UA_Int32 sensorReading = CO2Reading;
    UA_Variant value;
    UA_Variant_setScalar(&value, &sensorReading, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 15);
    UA_Server_writeValue(server, currentNodeId, value);
}


static void
beforeRead(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateSensorData(server);
}

static void
afterWrite(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "The variable was updated");
}

static void
addValueCallbackToSensorVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 15);
    UA_ValueCallback callback ;
    callback.onRead = beforeRead;
    callback.onWrite = afterWrite;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

/******** Data source ********/



// read callback

static UA_StatusCode 
readValue(UA_Server *server, const UA_NodeId *sessionId, 
         void *sessionContext, const UA_NodeId *nodeId, 
         void *nodeContext, UA_Boolean sourceTimeStamp, 
         const UA_NumericRange *range, UA_DataValue *dataValue) {
    int readContinuous = CO2SensorReadContinous(&fd);
    if(readContinuous != -1)
        CO2Reading = readContinuous;
    UA_Int32 value = CO2Reading;
    
    UA_Variant_setScalarCopy(&dataValue->value, &value,
                             &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

//write callback

static UA_StatusCode
writeValue(UA_Server *server,
                 const UA_NodeId *sessionId, void *sessionContext,
                 const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Sensor values can't be externally written to");
    return UA_STATUSCODE_BADINTERNALERROR;
}

//Source Node creation function

static void
addDataSourceVariable(UA_Server *server, int nodeNum, char* nodeName, char* displayName) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayName);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_HISTORYREAD;

    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, nodeNum);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, nodeName);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource sensorDataSource;
    sensorDataSource.read = readValue;
    sensorDataSource.write = writeValue;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        sensorDataSource, NULL, NULL);
}

static volatile UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}


int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    UA_ServerConfig* config = UA_Server_getConfig(server);
    config->verifyRequestTimestamp = UA_RULEHANDLING_ACCEPT;
    
    CO2SensorInit(&fd);
    CO2Reading = CO2SensorSingleShotMeasurement(&fd);
    CO2SensorSetContiniousMeasurement(&fd);
    
    
    addSensorVariable(server);
    addValueCallbackToSensorVariable(server);
    //addCurrentTimeDataSourceVariable(server);

    //addCurrentTimeExternalDataSource(server);

    //addDataSourceVariable(server, 15, "Sensor data", "CO2 Sensor");
    
    printf("Starting server ...\n");

    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
