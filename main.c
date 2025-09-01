#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#if _MSC_VER && !__INTEL_COMPILER
/* Some hacks for Visual Studio compiler */
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE* __cdecl __iob_func(void)
{
    return _iob;
}
#endif


/**
 * Logs an error message to standard error and exits if a fault has occurred in the XMLRPC environment.
 *
 * @param envP A pointer to the XMLRPC environment structure.
 */
static void dieIfFaultOccurred(xmlrpc_env* const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
            envP->fault_string, envP->fault_code);
        exit(1);
    }
}

/**
 * Initialize the XML-RPC environment and client.
 *
 * @param env Pointer to the XML-RPC error-handling environment.
 */
void init_xmlrpc(xmlrpc_env* env) {
    /* Initialize our error-handling environment. */
    xmlrpc_env_init(env);

    /* Create the global XML-RPC client object. */
    xmlrpc_client_init2(env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    dieIfFaultOccurred(env);
}


/**
 * @brief Cleans up the XML-RPC environment and shuts down the XML-RPC client library.
 *
 * This function is responsible for cleaning up the error-handling environment
 * created by xmlrpc_env_init() and shutting down the XML-RPC client library.
 * It ensures that all resources associated with these operations are properly released.
 *
 * @param env Pointer to the XML-RPC environment to be cleaned up.
 */
void deinit_xmlrpc(xmlrpc_env* env) {
    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();
}


/**
 * @brief Runs an experiment using the XML-RPC client.
 *
 * This function initializes a clean simulation by calling the 'run_experiment'
 * method on the server specified in SERVER_URI. It requires an initialized XML-RPC
 * environment and a valid EXPERIMENT_TOKEN to authenticate the request.
 *
 * @param env Pointer to the initialized XML-RPC environment.
 */
void run_experiment(xmlrpc_env* env) {
    xmlrpc_value* resultP;

    resultP = xmlrpc_client_call(env, SERVER_URI, "run_experiment", "(s)", EXPERIMENT_TOKEN);
    dieIfFaultOccurred(env);
    xmlrpc_DECREF(resultP);
}


/**
 * @brief Set the Peltier thermoelectric heat pump current.
 *
 * @param env XML-RPC environment object.
 * @param current Current value in amperes (A).
 */
void set_current(xmlrpc_env* env, double current) {
    double signal;
    xmlrpc_client_call(env,
        SERVER_URI,
        "set_current",
        "(sd)", EXPERIMENT_TOKEN, current);
    dieIfFaultOccurred(env);
}


/**
 * @brief Retrieves the thermocouple signal in millivolts.
 *
 * This function makes an XML-RPC call to a server URI to fetch the thermocouple
 * signal value and returns it as a double precision floating-point number
 * representing the voltage in millivolts.
 *
 * @param env Pointer to the XML-RPC environment structure for error handling.
 * @return The thermocouple signal value in mV.
 */
double get_thermocouple_signal_mV(xmlrpc_env* env) {
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

    //NOTE value from GOST Р 8.585-2001 p. 29
    double thermocouple_voltage_mV_20degree = 0.798;
    double accuracy = 0;

    /* Initialize XML-RPC environment in the very beginning */
    init_xmlrpc(&env);

    /* (Re)Start new simulation */
    run_experiment(&env);

    while(1){
        /* Read temperature sensor */
        sleep(1);
        thermocouple_voltage_mV= get_thermocouple_signal_mV(&env);
        accuracy = thermocouple_voltage_mV/thermocouple_voltage_mV_20degree;
        printf("Thermocouple signal: %lf mV\n", thermocouple_voltage_mV);
        printf("Should be signal: %lf mV\n", thermocouple_voltage_mV_20degree);
        printf("Accuracy: %lf \%\n", accuracy*100);

        //abs(I) cant be more then 6.4 according to documentation
        peltier_current = MAX(MIN(peltier_current * accuracy, 6.4), -6.4);

        /* Update heating / cooling effect impact*/
        set_current(&env, peltier_current);
    }
    /* Clear XML-RPC environment before exiting */
    deinit_xmlrpc(&env);
    return 0;
}
