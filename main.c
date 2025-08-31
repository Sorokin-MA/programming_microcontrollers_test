#include <stdlib.h>
#include <stdio.h>

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/client.h"

/* Choose here which server would you like to use: local or remote */

/* ===== For remote server ===== */
//#define EXPERIMENT_TOKEN "put_your_token_here"
//#define SERVER_URI "http://5.35.70.189:8999/RPC2"


/* ===== For local server ===== */
#define EXPERIMENT_TOKEN "local_server_token"
#define SERVER_URI "http://127.0.0.1:8999/RPC2"


#define NAME "Xmlrpc-c Test Client"
#define VERSION "1.0"

#if _MSC_VER && !__INTEL_COMPILER
/* Some hacks for Visual Studio compiler */
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE* __cdecl __iob_func(void)
{
    return _iob;
}
#endif


/* XML-RPC Error handling */
static void dieIfFaultOccurred(xmlrpc_env* const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
            envP->fault_string, envP->fault_code);
        exit(1);
    }
}

void init_xmlrpc(xmlrpc_env* env) {
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(env);

    /* Create the global XML-RPC client object. */
    xmlrpc_client_init2(env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    dieIfFaultOccurred(env);
}


void deinit_xmlrpc(xmlrpc_env* env) {
    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();
}


void run_experiment(xmlrpc_env* env) {
    /*
     * Run this function to start clean simulation
     * This function requires initialized XML-RPC environment
     */
    xmlrpc_value* resultP;

    resultP = xmlrpc_client_call(env, SERVER_URI, "run_experiment", "(s)", EXPERIMENT_TOKEN);
    dieIfFaultOccurred(env);
    xmlrpc_DECREF(resultP);
}


void set_current(xmlrpc_env* env, double current) {
    /*
     * Set Peltier thermoelectric heat pump current (in A)
     * TODO: you should write implementation for this function
     * XML-RPC documentation:
     * https://xmlrpc-c.sourceforge.io/doc/libxmlrpc.html
     * https://xmlrpc-c.sourceforge.io/doc/libxmlrpc_client.html
     */
}


double get_thermocouple_signal_mV(xmlrpc_env* env) {
    /*
     * Read  thermocouple signal (in mV)
     * 
     */
    xmlrpc_value* resultP;
    double signal;

    resultP = xmlrpc_client_call(env, SERVER_URI, "get_thermocouple_signal_mV", "(s)", EXPERIMENT_TOKEN);
    dieIfFaultOccurred(env);

    xmlrpc_read_double(env, resultP, &signal);
    dieIfFaultOccurred(env);

    xmlrpc_DECREF(resultP);
    return signal;
}


int main(int const argc, const char** const argv) {
    xmlrpc_env env;
    double peltier_current = 1; // in A
    double thermocouple_voltage_mV;

    /* Initialize XML-RPC environment in the very beginning */
    init_xmlrpc(&env);

    /* (Re)Start new simulation */
    run_experiment(&env);

    /* Update heating / cooling effect impact*/
    // set_current(&env, peltier_current); // Not implemented
    
    /* Read temperature sensor */
    thermocouple_voltage_mV= get_thermocouple_signal_mV(&env);
    printf("Thermocouple signal: %lf mV\n", thermocouple_voltage_mV);

    /* Clear XML-RPC environment before exiting */
    deinit_xmlrpc(&env);
    return 0;
}
