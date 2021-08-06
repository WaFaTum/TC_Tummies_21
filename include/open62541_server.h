#include <vector>
#include <string>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

class open62541_server{
    private:
    UA_Server *server;
    std::vector<int*> nodesVector;
    int nodesNamespace;
    public:
    open62541_server(int ns, sighandler_t __handler);
    void startServer(volatile UA_Boolean* run);
    void addVariableNode(char* nodeStringID, char* nodeName, char* displayName, int initVal);
    void addValueCallbackToSensorVariable(int nodeId);
    void updateNodeData(UA_NodeId nodeId);
};