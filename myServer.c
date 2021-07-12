#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <mosquitto.h>

#include "I2C_CO2Sensor.h"
#include "jsmn/jsmn.h"

#include <signal.h>
#include <stdlib.h>

#define SENSOR_NAMESPACE 1
#define CO2_SENSOR_STRING_ID "CO2"
#define HUMIDITY_SENSOR_STRING_ID "Humidity"
#define TEMP_SENSOR_STRING_ID "Temp"
#define LIGHT_SENSOR_STRING_ID "Light"
#define MQTT_HOST "192.168.1.122"
#define MQTT_PORT 1883
#define MQTT_TOPIC "Sensors/Data"

/*** updating variables manually ***/

int CO2Reading, fd;
int varCounter = 0;
int* varArray ;


static void
updateSensorData(UA_Server *server, UA_NodeId nodeId) {
    int readValue;
    if(nodeId.identifier.numeric == 1){
        readValue = CO2SensorReadContinous(&fd);
        if(readValue != -1)
            varArray[nodeId.identifier.numeric-1] = readValue;
        }
    UA_Int32 sensorReading = varArray[nodeId.identifier.numeric-1];
    UA_Variant value;
    UA_Variant_setScalar(&value, &sensorReading, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(SENSOR_NAMESPACE, nodeId.identifier.numeric);
    UA_Server_writeValue(server, currentNodeId, value);
}

static void
addSensorVariable(UA_Server *server, int namespace, char* nodeStringID, char* nodeName, char* displayName) {
    varCounter++;
    UA_Int32 initValue = 0;
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", displayName);
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Variant_setScalar(&attr.value, &initValue, &UA_TYPES[UA_TYPES_INT32]);

    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(namespace, varCounter);
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(namespace, nodeName);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_Server_addVariableNode(server, currentNodeId, parentNodeId,
                              parentReferenceNodeId, currentName,
                              variableTypeNodeId, attr, NULL, NULL);

    //updateSensorData(server, currentNodeId);
}


static void
beforeRead(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateSensorData(server, *nodeid);
}

static void
afterWrite(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                //"The variable was updated");
}

static void
addValueCallbackToSensorVariable(UA_Server *server, int namespace, int nodeId) {
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(namespace, nodeId);
    UA_ValueCallback callback ;
    callback.onRead = beforeRead;
    callback.onWrite = afterWrite;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}
static void
setVariableArray(){
    varArray = malloc(4*varCounter);
    printf("%p", varArray);
    }

/*** MQTT callbacks ***/

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	printf("ID: %d\n", * (int *) obj);
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */

    jsmn_init(&p);
    r = jsmn_parse(&p, (char *) msg->payload, strlen((char *) msg->payload), t,
                 sizeof(t) / sizeof(t[0]));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        exit(1);
    }
    char* token_string = malloc(7);
    int value;
    // Light
    memcpy(token_string, (char *) msg->payload +  t[4].start, 6);
    if(token_string[2] == ','){
        token_string[2] = '.';
        token_string[6] = '\0';
        }
    else{
            token_string[3] = '.';
            token_string[6] = '0';
            token_string[7] = '\0';
            }
    
    sscanf(token_string, "%d", &value);
    varArray[1] = value;
    
    // Temperature
    memcpy(token_string, (char *) msg->payload +  t[18].start, 6);
    token_string[2] = '.';
    token_string[6] = '\0';
    
    sscanf(token_string, "%d", &value);
    varArray[2] = value;
    
    // Humidity
    memcpy(token_string, (char *) msg->payload +  t[10].start, 6);
    if(token_string[2] == ','){
        token_string[2] = '.';
        token_string[6] = '\0';
        }
    else{
            token_string[3] = '.';
            token_string[6] = '0';
            token_string[7] = '\0';
            }
    
    sscanf(token_string, "%d", &value);
    varArray[3] = value;
    
    
    
        
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
    
    addSensorVariable(server, SENSOR_NAMESPACE, CO2_SENSOR_STRING_ID, "CO2 Sensor data", "CO2 concentration in ppm");
    addValueCallbackToSensorVariable(server, SENSOR_NAMESPACE, varCounter);
    addSensorVariable(server, SENSOR_NAMESPACE, CO2_SENSOR_STRING_ID, "Light Sensor data", "Light intensity in Lux");
    addValueCallbackToSensorVariable(server, SENSOR_NAMESPACE, varCounter);
    addSensorVariable(server, SENSOR_NAMESPACE, CO2_SENSOR_STRING_ID, "Temperature Sensor data", "Ambient Temperature in °C");
    addValueCallbackToSensorVariable(server, SENSOR_NAMESPACE, varCounter);
    addSensorVariable(server, SENSOR_NAMESPACE, CO2_SENSOR_STRING_ID, "Humidity Sensor data", "Air humidity in %");
    addValueCallbackToSensorVariable(server, SENSOR_NAMESPACE, varCounter);
    
    printf("Starting server ...\n");
    
    setVariableArray();
    
    CO2SensorInit(&fd);
    varArray[0] = CO2SensorSingleShotMeasurement(&fd);
    CO2SensorSetContiniousMeasurement(&fd);
    
    // MQTT
    int rc, id=12;
	printf("Initialized \n");
	mosquitto_lib_init();
	
	struct mosquitto *mosq;

	mosq = mosquitto_new("Subscribe", true, &id);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, id);
	if(rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		return -1;
	}

	mosquitto_loop_start(mosq);
	

    UA_StatusCode retval = UA_Server_run(server, &running);
    mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
